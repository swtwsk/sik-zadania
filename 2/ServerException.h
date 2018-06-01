#ifndef SERVEREXCEPTION_H
#define SERVEREXCEPTION_H

#include <exception>
#include <string>

class ServerException : public std::exception {
public:
    ServerException() = delete;

    const char *what() const noexcept override {
        return error_msg_.c_str();
    }

protected:
    explicit ServerException(std::string error_msg) : error_msg_(std::move(error_msg)) {}

    std::string error_msg_;
};

class ServerCreateException : public ServerException {
public:
    explicit ServerCreateException(const std::string &function_name)
        : ServerException("error while creating server in function: " + function_name) {}
};

class CtrlServerCreateException : public ServerException {
public:
    explicit CtrlServerCreateException(const std::string &function_name)
        : ServerException("error while creating server for ctrl_port in function: " + function_name) {}
};

#endif //INC_2_SERVEREXCEPTION_H
