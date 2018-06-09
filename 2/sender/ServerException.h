#ifndef SERVEREXCEPTION_H
#define SERVEREXCEPTION_H

#include <exception>
#include <string>

class TCPServerException : public std::exception {
public:
    TCPServerException() = delete;

    const char *what() const noexcept override {
        return error_msg_.c_str();
    }

protected:
    explicit TCPServerException(std::string error_msg) : error_msg_(std::move(error_msg)) {}

    std::string error_msg_;
};

class ServerCreateException : public TCPServerException {
public:
    explicit ServerCreateException(const std::string &function_name)
        : TCPServerException("error while creating server in function: " + function_name) {}
};

class CtrlServerCreateException : public TCPServerException {
public:
    explicit CtrlServerCreateException(const std::string &function_name)
        : TCPServerException("error while creating server for ctrl_port in function: " + function_name) {}
};

class ServerRunException : public TCPServerException {
public:
    explicit ServerRunException(const std::string &function_name)
        : TCPServerException("error while running server in function: " + function_name) {}
};

#endif //INC_2_SERVEREXCEPTION_H
