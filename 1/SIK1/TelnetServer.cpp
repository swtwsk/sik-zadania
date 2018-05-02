#include <iostream>
#include "TelnetServer.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void TelnetServer::handleTelnetConnection() {
    while(true) {
        try {
            server_->acceptConnection();
            std::cout << "Connection accepted" << std::endl;

            server_->writeClient(telnetSettings());
            showMainMenu();
            server_->readClient();

           /* char readChar;
            bool clientEndConnection = false;
            do {
                switch (readChar = server_->readCharacter()) {
                    case "\e["
                }
            } while (clientEndConnection);*/
            server_->endConnection();
            std::cout << "Connection closed" << std::endl;
        }
        catch (ServerClientConnectionException &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

void TelnetServer::showMainMenu() {
    std::stringstream ss;
    ss << '\n';
    ss << "======================================================\n";
    ss << "  Welcome to Telnet Server  ";
    ss << "\n\n";
    ss << "======================================================\n";
    server_->writeClient(ss.str());
    sleep(5);
    server_->writeClient("\e[2J");
    server_->writeClient("SEEMS WORKING XD");
}

std::string TelnetServer::telnetSettings() {
    std::string telnetSettings;

    telnetSettings.push_back(IAC);
    telnetSettings.push_back(WILL);
    telnetSettings.push_back(ECHO);
    telnetSettings.push_back(IAC);
    telnetSettings.push_back(WILL);
    telnetSettings.push_back(SUPPRESS_GO_AHEAD);
    telnetSettings.push_back(IAC);
    telnetSettings.push_back(WONT);
    telnetSettings.push_back(LINEMODE);

    return telnetSettings;
}

#pragma clang diagnostic pop
