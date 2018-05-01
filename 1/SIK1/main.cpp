#include <iostream>
#include <netinet/in.h>
#include <sstream>

#include "TCPSocket.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " port" << std::endl;
        return 1;
    }

    std::istringstream ss(argv[1]);
    TCPSocket::portType port;
    if (!(ss >> port)) {
        std::cerr << "Invalid port" << std::endl;
        return 1;
    }

    try {
        TCPSocket tcpSocket(port);
    }
    catch (TCPSocketException &t) {
        std::cerr << t.what() << std::endl;
        return 1;
    }

    std::cout << "Hello, World!" << std::endl;
    return 0;
}