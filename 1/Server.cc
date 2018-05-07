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

    if (bind(sock, reinterpret_cast<struct sockaddr *>(&server_address), sizeof(server_address)) < 0) {
        throw ServerCreateException("bind");
    }

    if (listen(sock, QUEUE_LENGTH) < 0) {
        throw ServerCreateException("listen");
    }
}

Server::~Server() {
    close(sock);
}

void Server::acceptConnection() {
    client_address_len = sizeof(client_address);
    client_sock = accept(sock, reinterpret_cast<struct sockaddr *>(&client_address), &client_address_len); // blocking operation
    if (client_sock < 0) {
        throw ServerClientConnectionException();
    }
}

std::string Server::readClient(size_t len) {
    auto left_to_read = len;
    auto *buffer = new char[len];
    std::string read_message;

    do {
        ssize_t read_len = read(client_sock, buffer, len);

        if (read_len <= 0) {
            if (read_len == 0) {
                throw ServerClientDisconnectedException();
            }

            throw ServerClientConnectionException();
        }

        left_to_read -= read_len;
        read_message.append(buffer, static_cast<unsigned long>(read_len)); // read_len is > 0

        memset(buffer, 0, len);
    } while (left_to_read > 0);

    delete[] buffer;

    return read_message;
}

char Server::readCharacter() {
    char read_char;

    ssize_t read_len = read(client_sock, &read_char, sizeof(read_char));
    if (read_len <= 0) {
        if (read_len == 0) {
            throw ServerClientDisconnectedException();
        }

        throw ServerClientConnectionException();
    }

    return read_char;
}

void Server::writeToClient(const std::string &msg) {
    ssize_t len;
    len = write(client_sock, msg.c_str(), msg.size());
    if (len < 0 || static_cast<size_t>(len) != msg.size()) {
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
