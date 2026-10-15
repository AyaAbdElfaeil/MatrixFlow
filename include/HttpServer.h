#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>

class HttpServer {
public:
    HttpServer(int preferredPort = 8421, const std::string& webDir = "web");
    ~HttpServer();

    bool start();
    void stop();
    int getPort() const { return actualPort; }
    std::string getUrl() const { return "http://127.0.0.1:" + std::to_string(actualPort); }

    // Internal thread runners
    void runServerLoop();
    void handleClient(uintptr_t clientSocket);

private:
    int preferredPort;
    int actualPort;
    std::string webRoot;
    volatile bool isRunning;
    uintptr_t serverSocket; // SOCKET in WinSock

    std::string handleApiSolve(const std::string& body);
    std::string handleApiSamples();
    std::string handleApiTest();
    std::string readFileContent(const std::string& relativePath);
    std::string getMimeType(const std::string& path);
};

#endif // HTTP_SERVER_H
