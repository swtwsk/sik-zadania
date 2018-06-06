#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <future>
#include <ctime>

#include "Transmitter.h"

Transmitter::Transmitter(TransmitterData *transmitter_data)
    : transmitter_data_(transmitter_data), data_queue_(std::make_shared<DataQueueT>(transmitter_data->getFsize())) {}

void Transmitter::startTransmitter() {
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ < 0) {
        throw ServerCreateException("socket");
    }

    const static int ENABLE_BROADCAST_VALUE = 1;
    int optval = ENABLE_BROADCAST_VALUE;
    if (setsockopt(sock_, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<void *>(&optval), sizeof optval) < 0) {
        close(sock_);
        throw ServerCreateException("setsockopt broadcast");
    }

    const static int TTL_VALUE = 4; // depth of sent packets
    optval = TTL_VALUE;
    if (setsockopt(sock_, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<void *>(&optval), sizeof optval) < 0) {
        close(sock_);
        throw ServerCreateException("setsockopt multicast ttl");
    }

    remote_address_.sin_family = AF_INET;
    remote_address_.sin_port = htons(transmitter_data_->getDataPort());
    if (inet_aton(transmitter_data_->getMcastAddr().c_str(), &remote_address_.sin_addr) == 0) {
        close(sock_);
        throw ServerCreateException("inet_aton");
    }
    if (connect(sock_, reinterpret_cast<struct sockaddr *>(&remote_address_), sizeof remote_address_) < 0) {
        close(sock_);
        throw ServerCreateException("connect");
    }

    // might throw ServerCreateException
    ctrl_port_listener_ = new CtrlPortListener(transmitter_data_, data_queue_);

    std::future<void> future_rexmit_stopper = exit_rexmit_signal_.get_future();
    ctrl_port_rexmit_thread_ = std::thread(
        &CtrlPortListener::handleRetransmissions, ctrl_port_listener_, std::move(future_rexmit_stopper));

    std::future<void> future_listener_stopper = exit_listener_signal_.get_future();
    ctrl_port_listener_thread_ = std::thread(
        &CtrlPortListener::listenOnCtrlPort, ctrl_port_listener_, std::move(future_listener_stopper));
}

void Transmitter::stopTransmitter() {
    close(sock_);
    delete ctrl_port_listener_;
}

void Transmitter::readStdIn() {
    NumType first_byte_num = 0;

    auto *input = new Byte[transmitter_data_->getPsize()];

    while (fread(input, sizeof(Byte), transmitter_data_->getPsize(), stdin)) {
        data_queue_->push(input, transmitter_data_->getPsize());

        Byte *pack_to_send = transmitter_data_->packUp(first_byte_num, input);
        writeToClient(pack_to_send, transmitter_data_->getPsize() + TransmitterData::PACKET_HEADER_SIZE);
        delete[] pack_to_send;

        first_byte_num += transmitter_data_->getPsize();
    }

    exit_rexmit_signal_.set_value();
    exit_listener_signal_.set_value();
    ctrl_port_rexmit_thread_.join();
    ctrl_port_listener_thread_.join();

    delete[] input;
}

void Transmitter::writeToClient(Transmitter::Byte *data, size_t data_size) {
    ssize_t len;
    len = write(sock_, data, data_size);
    if (static_cast<size_t>(len) != data_size) {
        throw ServerRunException("Error in write");
    }
}
