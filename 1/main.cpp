#include <iostream>
#include <sstream>
#include <memory>

#include "Server.h"
#include "TelnetServer.h"
#include "TelnetTerminal.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " port" << std::endl;
        return 1;
    }

    std::istringstream ss(argv[1]);
    TelnetServer::PortType port;
    if (!(ss >> port)) {
        std::cerr << "Invalid port" << std::endl;
        return 1;
    }

    try {
        TelnetTerminal telnet_terminal(std::make_unique<TelnetServer>(port));
        telnet_terminal.handleClient();
    }
    catch (ServerException &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}