#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <iostream>
#include <string>
#include <memory>
#include <netinet/in.h>
#include <queue>

#include "CtrlPortListener.h"

class Transmitter {
public:
    explicit Transmitter(const std::string &mcast_addr);

    void setDataPort(in_port_t data_port);
    void setCtrlPort(in_port_t ctrl_port);
    void setPsize(uint64_t psize);
    void setFsize(uint64_t fsize);
    void setRtime(uint64_t rtime);
    void setNazwa(const std::string &nazwa);

    void startTransmitter();

    // DEBUG
    void printTransmitter();

    using Byte = uint8_t;

private:
    using DataQueueT = std::queue<Byte>;
    using DataQueuePtr = std::shared_ptr<DataQueueT>;
    using CtrlPortListenerPtr = std::unique_ptr<CtrlPortListener>;

    const in_port_t DEFAULT_DATA_PORT = 26085;
    const in_port_t DEFAULT_CTRL_PORT = 36085;
    const uint64_t DEFAULT_PSIZE = 512;
    const uint64_t DEFAULT_FSIZE = 131072;
    const uint64_t DEFAULT_RTIME = 250;
    const std::string DEFAULT_NAZWA = "Nienazwany Nadajnik";

    int sock_;
    struct sockaddr_in remote_address_;
    struct ip_mreq ip_mreq_;

    std::string mcast_addr_;
    in_port_t data_port_;
    in_port_t ctrl_port_;
    uint64_t psize_;
    uint64_t fsize_;
    uint64_t rtime_;
    std::string nazwa_;

    DataQueuePtr data_queue_;
    CtrlPortListenerPtr ctrl_port_listener_;
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
        : ServerException("error while creating server in function: " + function_name) {}
};

#endif //TRANSMITTER_H
