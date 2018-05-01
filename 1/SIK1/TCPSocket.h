#ifndef TCPSOCKET_H
#define TCPSOCKET_H


#include <cstdint>
#include <netinet/in.h>
#include <exception>
#include <string>

// TODO: think about proper name for class
class TCPSocket {
public:
    explicit TCPSocket(uint16_t port);

    typedef uint16_t portType;

private:
    int sock;
    sockaddr_in server_address;
};

class TCPSocketException : public std::exception {
public:
    explicit TCPSocketException(const std::string &function_name);

    virtual const char *what() const noexcept {
        return error_msg_.c_str();
    }

private:
    std::string error_msg_;
};


#endif //TCPSOCKET_H
