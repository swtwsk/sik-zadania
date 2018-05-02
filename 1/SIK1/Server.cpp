#include <iostream>
#include <cstring>
#include "Server.h"

/** Server **/
Server::Server(PortType port) {
    sock = socket(PF_INET, SOCK_STREAM, 0); // creating IPv4 socket
    if (sock < 0) {
        throw ServerCreateException("sock");
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    if (bind(sock, (sockaddr *) &server_address, sizeof(server_address)) < 0) {
        throw ServerCreateException("bind");
    }

    const int QUEUE_LENGTH = 5; // TODO: change it!

    if (listen(sock, QUEUE_LENGTH) < 0) {
        throw ServerCreateException("listen");
    }
}

void Server::acceptConnection() {
    client_address_len = sizeof(client_address);
    client_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_len); // blocking operation
    if (client_sock < 0) {
        throw ServerClientConnectionException();
    }
}

void Server::readClient() {
    ssize_t len;
    char buffer[2000];
    do {
        len = read(client_sock, buffer, sizeof(buffer));
        if (len < 0) {
            throw ServerClientConnectionException();
        }
        std::cout << "Read from socket: " << buffer;
        memset(buffer, 0, sizeof(buffer));
    } while (len > 0);
}

char Server::readCharacter() {
    char readChar;

    if (read(client_sock, &readChar, sizeof(readChar))< 0) {
        throw ServerClientConnectionException();
    }

    return readChar;
}

void Server::writeClient(int character) {
    if (write(client_sock, &character, 1) != 1) {
        throw ServerClientConnectionException();
    }
}

void Server::writeClient(const std::string &msg) {
    ssize_t len;
    len = write(client_sock, msg.c_str(), msg.size());
    if (len != msg.size()) {
        throw ServerClientConnectionException();
    }
}

void Server::endConnection() {
    if (close(client_sock) < 0) {
        throw ServerClientConnectionException();
    }
}


/** ServerException **/
const char *ServerException::what() const noexcept {
    return error_msg_.c_str();
}
