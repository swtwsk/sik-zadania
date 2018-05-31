#include <arpa/inet.h>
#include <unistd.h>

#include "Transmitter.h"

Transmitter::Transmitter(const std::string &mcast_addr)
    : mcast_addr_(mcast_addr), data_port_(DEFAULT_DATA_PORT), ctrl_port_(DEFAULT_CTRL_PORT), psize_(DEFAULT_PSIZE),
      fsize_(DEFAULT_FSIZE), rtime_(DEFAULT_RTIME), nazwa_(DEFAULT_NAZWA),
      data_queue_(std::make_shared<DataQueueT>()) {}

void Transmitter::setDataPort(in_port_t data_port) {
    Transmitter::data_port_ = data_port;
}

void Transmitter::setCtrlPort(in_port_t ctrl_port) {
    Transmitter::ctrl_port_ = ctrl_port;
}

void Transmitter::setPsize(uint64_t psize) {
    Transmitter::psize_ = psize;
}

void Transmitter::setFsize(uint64_t fsize) {
    Transmitter::fsize_ = fsize;
}

void Transmitter::setRtime(uint64_t rtime) {
    Transmitter::rtime_ = rtime;
}

void Transmitter::setNazwa(const std::string &nazwa) {
    Transmitter::nazwa_ = nazwa;
}

void Transmitter::printTransmitter() {
    std::cout << "TRANSMITTER:\n  mcast_addr_ = " << inet_ntoa(ip_mreq_.imr_multiaddr)
              << "\n  data_port_ = " << data_port_ << "\n  ctrl_port_ = " << ctrl_port_
              << "\n  psize_ = " << psize_ << "\n  fsize_ = " << fsize_
              << "\n  rtime_ = " << rtime_ << "\n  nazwa_ = " << nazwa_ << std::endl;
}
void Transmitter::startTransmitter() {
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ < 0) {
        throw ServerCreateException("socket");
    }

    int optval = 1; // ?
    if (setsockopt(sock_, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<void *>(&optval), sizeof optval) < 0) {
        close(sock_);
        throw ServerCreateException("setsockopt broadcast");
    }

    int TTL_VALUE = 4; // ?
    optval = TTL_VALUE;
    if (setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<void *>(&optval), sizeof optval) < 0) {
        close(sock_);
        throw ServerCreateException("setsockopt multicast ttl");
    }

    remote_address_.sin_family = AF_INET;
    remote_address_.sin_port = htons(data_port_);
    if (inet_aton(mcast_addr_.c_str(), &remote_address_.sin_addr) == 0) {
        close(sock_);
        throw ServerCreateException("inet_aton");
    }
    if (connect(sock_, reinterpret_cast<struct sockaddr *>(&remote_address_), sizeof remote_address_) < 0) {
        close(sock_);
        throw ServerCreateException("connect");
    }

    ctrl_port_listener_ = std::make_unique<CtrlPortListener>(ctrl_port_, data_queue_, mcast_addr_);
}

/** ServerException **/
const char *ServerException::what() const noexcept {
    return error_msg_.c_str();
}