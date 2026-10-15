#include "../include/HttpServer.h"
#include <iostream>
#include <windows.h>
#include <shellapi.h>

void launchBrowser(const std::string& url) {
    // Try launching Microsoft Edge in standalone app mode (clean native-like window without browser address bar)
    std::string edgeCmd = "msedge.exe --app=" + url;
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi;

    if (CreateProcessA(NULL, const_cast<char*>(edgeCmd.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return;
    }

    // Fallback: Default system browser
    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

int main() {
    std::cout << "================================================================" << std::endl;
    std::cout << "   MatrixFlow - Interactive Gaussian Elimination Visualizer    " << std::endl;
    std::cout << "   C++ Mathematical Core & Native Interactive Educational UI   " << std::endl;
    std::cout << "================================================================" << std::endl;
    std::cout << "Initializing C++ Mathematical Backend and Local Engine..." << std::endl;

    HttpServer server(8421, "web");
    if (!server.start()) {
        std::cerr << "Error: Could not start local application server." << std::endl;
        return 1;
    }

    std::string url = server.getUrl();
    std::cout << "\n[SUCCESS] MatrixFlow running at: " << url << std::endl;
    std::cout << "Launching desktop interface window..." << std::endl;

    launchBrowser(url);

    std::cout << "\n----------------------------------------------------------------" << std::endl;
    std::cout << " The application is now running!" << std::endl;
    std::cout << " You can interact with the visualizer directly in the app window." << std::endl;
    std::cout << " Press [Enter] at any time in this console to stop the server." << std::endl;
    std::cout << "----------------------------------------------------------------" << std::endl;

    std::cin.get();

    std::cout << "Stopping MatrixFlow server..." << std::endl;
    server.stop();
    std::cout << "Goodbye!" << std::endl;

    return 0;
}
