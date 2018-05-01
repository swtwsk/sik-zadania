//
// Created by kennedy63 on 01.05.18.
//

#include "TCPSocket.h"

TCPSocket::TCPSocket(uint16_t port) {
    sock = socket(PF_INET, SOCK_STREAM, 0); // creating IPv4 socket
    if (sock < 0) {
        throw TCPSocketException("sock");
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    if (bind(sock, (sockaddr *) &server_address, sizeof(server_address)) < 0) {
        throw TCPSocketException("bind");
    }

    const int QUEUE_LENGTH = 5; // TODO: change it!

    if (listen(sock, QUEUE_LENGTH) < 0) {
        throw TCPSocketException("listen");
    }
}

TCPSocketException::TCPSocketException(const std::string &function_name) {
    error_msg_ = "error while creating TCP socket in function: " + function_name;
}