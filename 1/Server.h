#ifndef SERVER_H
#define SERVER_H

#include <cstdint>
#include <netinet/in.h>
#include <exception>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class Server {
public:
    using PortType = uint16_t;

    explicit Server(PortType port);

    void acceptConnection();
    void endConnection();

    std::string readClient(size_t len);
    char readCharacter();

    void writeToClient(const std::string &msg);

private:
    const int QUEUE_LENGTH = 5;

    int sock;
    int client_sock;

    sockaddr_in server_address;
    sockaddr_in client_address;
    socklen_t client_address_len;
};

class ServerException : public std::exception {
public:
    ServerException() = delete;

    const char *what() const noexcept override;

protected:
    explicit ServerException(std::string error_msg) : error_msg_(std::move(error_msg)) {}

    std::string error_msg_;
};

class ServerCreateException : public ServerException {
public:
    explicit ServerCreateException(const std::string &function_name)
            : ServerException("error while creating TCP socket in function: " + function_name) {}
};

class ServerClientConnectionException : public ServerException {
public:
    ServerClientConnectionException() : ServerException("client connection error") {}
    explicit ServerClientConnectionException(const std::string &connection_error)
            : ServerException(connection_error) {}

};

class ServerClientDisconnectedException : public ServerClientConnectionException {
public:
    ServerClientDisconnectedException() : ServerClientConnectionException("client disconnected") {}
};

#endif //SERVER_H
