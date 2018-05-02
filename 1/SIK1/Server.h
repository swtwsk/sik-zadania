#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <cstdint>
#include <netinet/in.h>
#include <exception>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// TODO: think about abstracting it
class Server {
public:
    using PortType = uint16_t;

    explicit Server(PortType port);

    void acceptConnection();
    void readClient();
    char readCharacter();
    void writeClient(int character);
    void writeClient(const std::string &msg);
    void endConnection();

private:
    int sock;
    int client_sock;

    sockaddr_in server_address;
    sockaddr_in client_address;
    socklen_t client_address_len;
};

class ServerException : public std::exception {
public:
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
    explicit ServerClientConnectionException()
            : ServerException("client connection error") {}
};

#endif //TCPSOCKET_H