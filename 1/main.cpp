#include <iostream>
#include <sstream>

#include "Server.h"
#include "TelnetServer.h"

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
        TelnetServer telnetServer(port);
        telnetServer.handleTelnetConnection();
    }
    catch (ServerException &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}