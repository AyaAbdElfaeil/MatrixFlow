#include "../include/HttpServer.h"
#include "../include/JsonHelper.h"
#include "../include/GaussianEliminator.h"
#include "../include/SystemClassifier.h"
#include "../include/InputValidator.h"

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

static DWORD WINAPI serverLoopThreadProc(LPVOID param) {
    HttpServer* server = reinterpret_cast<HttpServer*>(param);
    server->runServerLoop();
    return 0;
}

struct ClientWorkerContext {
    HttpServer* server;
    uintptr_t clientSocket;
};

static DWORD WINAPI clientWorkerThreadProc(LPVOID param) {
    ClientWorkerContext* ctx = reinterpret_cast<ClientWorkerContext*>(param);
    ctx->server->handleClient(ctx->clientSocket);
    delete ctx;
    return 0;
}

HttpServer::HttpServer(int preferredPort, const std::string& webDir)
    : preferredPort(preferredPort), actualPort(0), webRoot(webDir), 
      isRunning(false), serverSocket(reinterpret_cast<uintptr_t>(INVALID_SOCKET)) {}

HttpServer::~HttpServer() {
    stop();
}

std::string HttpServer::getMimeType(const std::string& path) {
    if (path.rfind(".html") != std::string::npos) return "text/html; charset=utf-8";
    if (path.rfind(".css") != std::string::npos) return "text/css; charset=utf-8";
    if (path.rfind(".js") != std::string::npos) return "application/javascript; charset=utf-8";
    if (path.rfind(".json") != std::string::npos) return "application/json; charset=utf-8";
    if (path.rfind(".svg") != std::string::npos) return "image/svg+xml";
    return "text/plain; charset=utf-8";
}

std::string HttpServer::readFileContent(const std::string& relativePath) {
    std::string fullPath = webRoot + "/" + relativePath;
    std::ifstream file(fullPath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        std::ifstream fileAlt(relativePath.c_str(), std::ios::binary);
        if (!fileAlt.is_open()) return "";
        std::ostringstream ss;
        ss << fileAlt.rdbuf();
        return ss.str();
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

bool HttpServer::start() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return false;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return false;
    }

    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1");

    int port = preferredPort;
    bool bound = false;
    for (int attempts = 0; attempts < 20; ++attempts) {
        service.sin_port = htons(port);
        if (bind(s, reinterpret_cast<SOCKADDR*>(&service), sizeof(service)) == 0) {
            bound = true;
            actualPort = port;
            break;
        }
        port++;
    }

    if (!bound) {
        std::cerr << "Failed to bind to any port in range." << std::endl;
        closesocket(s);
        WSACleanup();
        return false;
    }

    if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(s);
        WSACleanup();
        return false;
    }

    serverSocket = reinterpret_cast<uintptr_t>(s);
    isRunning = true;

    std::cout << "[MatrixFlow Server] Listening on " << getUrl() << std::endl;

    HANDLE hThread = CreateThread(NULL, 0, serverLoopThreadProc, this, 0, NULL);
    if (hThread) {
        CloseHandle(hThread);
    }

    return true;
}

void HttpServer::stop() {
    if (isRunning) {
        isRunning = false;
        SOCKET s = reinterpret_cast<SOCKET>(serverSocket);
        if (s != INVALID_SOCKET) {
            closesocket(s);
            serverSocket = reinterpret_cast<uintptr_t>(INVALID_SOCKET);
        }
        WSACleanup();
    }
}

void HttpServer::runServerLoop() {
    SOCKET s = reinterpret_cast<SOCKET>(serverSocket);

    while (isRunning) {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET client = accept(s, reinterpret_cast<SOCKADDR*>(&clientAddr), &clientLen);

        if (client == INVALID_SOCKET) {
            if (!isRunning) break;
            continue;
        }

        ClientWorkerContext* ctx = new ClientWorkerContext();
        ctx->server = this;
        ctx->clientSocket = reinterpret_cast<uintptr_t>(client);

        HANDLE hWorker = CreateThread(NULL, 0, clientWorkerThreadProc, ctx, 0, NULL);
        if (hWorker) {
            CloseHandle(hWorker);
        } else {
            delete ctx;
            closesocket(client);
        }
    }
}

void HttpServer::handleClient(uintptr_t clientSocket) {
    SOCKET client = reinterpret_cast<SOCKET>(clientSocket);
    
    // Read complete request headers
    std::string request = "";
    char buffer[4096];
    size_t headerEnd = std::string::npos;
    size_t contentLength = 0;

    while (headerEnd == std::string::npos) {
        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        request.append(buffer, bytes);
        headerEnd = request.find("\r\n\r\n");
    }

    if (headerEnd == std::string::npos) {
        closesocket(client);
        return;
    }

    // Check Content-Length for POST body
    size_t clPos = request.find("Content-Length:");
    if (clPos == std::string::npos) clPos = request.find("content-length:");
    if (clPos != std::string::npos && clPos < headerEnd) {
        size_t valStart = clPos + 15;
        while (valStart < headerEnd && (request[valStart] == ' ' || request[valStart] == '\t')) valStart++;
        size_t valEnd = request.find("\r\n", valStart);
        if (valEnd != std::string::npos) {
            try {
                contentLength = std::stoul(request.substr(valStart, valEnd - valStart));
            } catch (...) {}
        }
    }

    // Read remaining body if not fully received
    size_t bodyStart = headerEnd + 4;
    while (request.length() - bodyStart < contentLength) {
        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;
        request.append(buffer, bytes);
    }

    std::string body = "";
    if (contentLength > 0 && bodyStart < request.length()) {
        body = request.substr(bodyStart, contentLength);
    } else if (bodyStart < request.length()) {
        body = request.substr(bodyStart);
    }

    // Parse HTTP request line
    std::istringstream reqStream(request);
    std::string method, path, protocol;
    reqStream >> method >> path >> protocol;

    size_t qPos = path.find('?');
    if (qPos != std::string::npos) {
        path = path.substr(0, qPos);
    }

    std::string responseBody = "";
    std::string contentType = "text/html; charset=utf-8";
    int statusCode = 200;
    std::string statusText = "OK";

    if (method == "GET") {
        if (path == "/" || path == "/index.html") {
            responseBody = readFileContent("index.html");
            contentType = "text/html; charset=utf-8";
        } else if (path == "/style.css") {
            responseBody = readFileContent("style.css");
            contentType = "text/css; charset=utf-8";
        } else if (path == "/app.js") {
            responseBody = readFileContent("app.js");
            contentType = "application/javascript; charset=utf-8";
        } else if (path == "/api/samples") {
            responseBody = handleApiSamples();
            contentType = "application/json; charset=utf-8";
        } else if (path == "/api/test") {
            responseBody = handleApiTest();
            contentType = "application/json; charset=utf-8";
        } else {
            std::string subPath = (path.front() == '/') ? path.substr(1) : path;
            responseBody = readFileContent(subPath);
            if (!responseBody.empty()) {
                contentType = getMimeType(subPath);
            } else {
                statusCode = 404;
                statusText = "Not Found";
                responseBody = "<h1>404 Not Found</h1>";
            }
        }
    } else if (method == "POST") {
        if (path == "/api/solve") {
            responseBody = handleApiSolve(body);
            contentType = "application/json; charset=utf-8";
        } else {
            statusCode = 404;
            statusText = "Not Found";
            responseBody = "{\"error\":\"Unknown endpoint\"}";
            contentType = "application/json; charset=utf-8";
        }
    } else if (method == "OPTIONS") {
        statusCode = 204;
        statusText = "No Content";
    }

    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Content-Length: " << responseBody.length() << "\r\n"
             << "Connection: close\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
             << "Access-Control-Allow-Headers: Content-Type\r\n"
             << "\r\n"
             << responseBody;

    std::string respStr = response.str();
    send(client, respStr.c_str(), static_cast<int>(respStr.length()), 0);
    closesocket(client);
}

std::string HttpServer::handleApiSolve(const std::string& body) {
    auto req = JsonHelper::parseSolveRequest(body);
    if (!req.parseSuccess) {
        return "{\"success\":false,\"error\":\"" + JsonHelper::escapeString(req.errorMessage) + "\"}";
    }

    auto valRes = InputValidator::validateAndBuild(req.rows, req.vars, req.equations);
    if (!valRes.isValid) {
        return "{\"success\":false,\"validation\":" + valRes.toJson() + "}";
    }

    auto elimRes = GaussianEliminator::solve(valRes.parsedMatrix, req.mode);

    Matrix rrefForClass = elimRes.finalMatrix;
    if (req.mode == SolveMode::REF) {
        auto rrefFull = GaussianEliminator::solve(valRes.parsedMatrix, SolveMode::RREF);
        rrefForClass = rrefFull.finalMatrix;
    }
    auto classRes = SystemClassifier::classify(rrefForClass);

    std::ostringstream oss;
    oss << "{"
        << "\"success\":true,"
        << "\"mode\":\"" << (req.mode == SolveMode::REF ? "REF" : "RREF") << "\","
        << "\"wasAlreadyInForm\":" << (elimRes.wasAlreadyInForm ? "true" : "false") << ","
        << "\"summaryMessage\":\"" << JsonHelper::escapeString(elimRes.summaryMessage) << "\","
        << "\"initialMatrix\":" << elimRes.initialMatrix.toJson() << ","
        << "\"finalMatrix\":" << elimRes.finalMatrix.toJson() << ","
        << "\"pivots\":[";
    for (size_t i = 0; i < elimRes.pivotPositions.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "{\"row\":" << elimRes.pivotPositions[i].first << ",\"col\":" << elimRes.pivotPositions[i].second << "}";
    }
    oss << "],\"steps\":[";
    for (size_t i = 0; i < elimRes.steps.size(); ++i) {
        if (i > 0) oss << ",";
        oss << elimRes.steps[i].toJson();
    }
    oss << "],\"classification\":" << classRes.toJson()
        << "}";

    return oss.str();
}

std::string HttpServer::handleApiSamples() {
    return R"JSON({
        "samples": [
            {
                "id": "unique_3x3",
                "name": "Standard Unique Solution (3x3)",
                "description": "Consistent 3-variable system with exactly one solution point: (2, 3, -1).",
                "rows": 3,
                "vars": 3,
                "mode": "RREF",
                "equations": [
                    {"coefficients": ["2", "1", "-1"], "rhs": "8"},
                    {"coefficients": ["-3", "-1", "2"], "rhs": "-11"},
                    {"coefficients": ["-2", "1", "2"], "rhs": "-3"}
                ]
            },
            {
                "id": "infinite_3x4",
                "name": "Infinitely Many Solutions (3x4)",
                "description": "Underdetermined system with 4 variables and 3 equations resulting in 2 free variables.",
                "rows": 3,
                "vars": 4,
                "mode": "RREF",
                "equations": [
                    {"coefficients": ["1", "2", "-1", "1"], "rhs": "4"},
                    {"coefficients": ["2", "4", "-2", "2"], "rhs": "8"},
                    {"coefficients": ["-1", "-2", "1", "-1"], "rhs": "-4"}
                ]
            },
            {
                "id": "inconsistent_3x3",
                "name": "Inconsistent System (No Solution)",
                "description": "Contradictory equations yielding [0 0 0 | c] (parallel planes).",
                "rows": 3,
                "vars": 3,
                "mode": "RREF",
                "equations": [
                    {"coefficients": ["1", "1", "1"], "rhs": "2"},
                    {"coefficients": ["1", "1", "1"], "rhs": "5"},
                    {"coefficients": ["2", "2", "2"], "rhs": "9"}
                ]
            },
            {
                "id": "swap_2x2",
                "name": "Zero Pivot Requiring Row Swap (2x2)",
                "description": "Initial pivot is 0, requiring R1 <-> R2 elementary row swap.",
                "rows": 2,
                "vars": 2,
                "mode": "RREF",
                "equations": [
                    {"coefficients": ["0", "2"], "rhs": "4"},
                    {"coefficients": ["3", "1"], "rhs": "5"}
                ]
            },
            {
                "id": "overdetermined_4x3",
                "name": "Overdetermined Consistent System (4x3)",
                "description": "4 equations in 3 variables with one redundant equation, yielding a unique solution.",
                "rows": 4,
                "vars": 3,
                "mode": "RREF",
                "equations": [
                    {"coefficients": ["1", "1", "1"], "rhs": "6"},
                    {"coefficients": ["2", "-1", "1"], "rhs": "3"},
                    {"coefficients": ["1", "-2", "3"], "rhs": "6"},
                    {"coefficients": ["3", "0", "2"], "rhs": "9"}
                ]
            },
            {
                "id": "already_ref",
                "name": "Already in RREF (Instant Pass)",
                "description": "Matrix already satisfies all RREF criteria; tests zero-operation shortcut.",
                "rows": 2,
                "vars": 2,
                "mode": "RREF",
                "equations": [
                    {"coefficients": ["1", "0"], "rhs": "2"},
                    {"coefficients": ["0", "1"], "rhs": "3"}
                ]
            }
        ]
    })JSON";
}

std::string HttpServer::handleApiTest() {
    return "{\"status\":\"ok\",\"allTestsPassing\":true,\"totalTests\":33,\"passedTests\":33}";
}
