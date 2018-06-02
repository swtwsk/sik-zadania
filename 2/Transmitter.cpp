#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <future>
#include <ctime>

#include "Transmitter.h"

Transmitter::Transmitter(TransmitterData &transmitter_data)
    : transmitter_data_(transmitter_data), data_queue_(std::make_shared<DataQueueT>(transmitter_data.getFsize())) {}

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
    remote_address_.sin_port = htons(transmitter_data_.getDataPort());
    if (inet_aton(transmitter_data_.getMcastAddr().c_str(), &remote_address_.sin_addr) == 0) {
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
    ctrl_port_rexmit_thread_ = std::thread(&CtrlPortListener::rexmitQueue, ctrl_port_listener_, std::move(future_rexmit_stopper));
    std::future<void> future_listener_stopper = exit_listener_signal_.get_future();
    ctrl_port_listener_thread_ = std::thread(&CtrlPortListener::listenOnCtrlPort, ctrl_port_listener_, std::move(future_listener_stopper));
}

void Transmitter::readStdIn() {
    using std::cin;

    std::string s;
    uint64_t first_byte_num = 0;

    while (cin >> s) {
        std::deque<Byte> readed_bytes;
        for (char b : s) {
            data_queue_->push(static_cast<Byte>(b));
            readed_bytes.push_back(static_cast<Byte>(b));
        }

        if (readed_bytes.size() >= transmitter_data_.getDataSize()) {
            unsigned char *pack_to_send = pack_up(first_byte_num, readed_bytes);
            std::cout << pack_to_send;
            writeToClient(pack_to_send, transmitter_data_.getPsize());
            first_byte_num += transmitter_data_.getDataSize();
        }
    }

    //std::cout << "O chuj TU chodzi xD" << std::endl;

    exit_rexmit_signal_.set_value();
    exit_listener_signal_.set_value();
    ctrl_port_rexmit_thread_.join();
    ctrl_port_listener_thread_.join();
}

void Transmitter::writeToClient(Transmitter::Byte *data, size_t data_size) {
    ssize_t len;
    len = write(sock_, data, data_size);
    if (static_cast<size_t>(len) != data_size) {
        throw ServerRunException("Error in write");
    }

    delete[] data;
}

Transmitter::Byte *Transmitter::pack_up(uint64_t first_byte_num, std::deque<Transmitter::Byte> &audio_data) {
    auto *to_return = new Byte[transmitter_data_.getPsize()];

    uint64_t session_id = transmitter_data_.getSessionId();

    // TODO: Change it to a function
    to_return[0]=session_id>>56&0xFF;
    to_return[1]=session_id>>48&0xFF;
    to_return[2]=session_id>>40&0xFF;
    to_return[3]=session_id>>32&0xFF;
    to_return[4]=session_id>>24&0xFF;
    to_return[5]=session_id>>16&0xFF;
    to_return[6]=session_id>>8&0xFF;
    to_return[7]=session_id>>0&0xFF;

    /*to_return[8] = session_id>>0;
    to_return[9] = session_id>>8;
    to_return[10] = session_id>>16;
    to_return[11] = session_id>>24;
    to_return[12] = session_id>>32;
    to_return[13] = session_id>>40;
    to_return[14] = session_id>>48;
    to_return[15] = session_id>>56;*/

    to_return[8]=first_byte_num>>56&0xFF;
    to_return[9]=first_byte_num>>48&0xFF;
    to_return[10]=first_byte_num>>40&0xFF;
    to_return[11]=first_byte_num>>32&0xFF;
    to_return[12]=first_byte_num>>24&0xFF;
    to_return[13]=first_byte_num>>16&0xFF;
    to_return[14]=first_byte_num>>8&0xFF;
    to_return[15]=first_byte_num>>0&0xFF;

    for (uint64_t i = 16; i < transmitter_data_.getPsize(); ++i) {
        to_return[i] = audio_data.front();
        audio_data.pop_front();
    }

    return to_return;
}
