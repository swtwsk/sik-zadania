#include <arpa/inet.h>
#include <unistd.h>
#include <future>
#include <iostream>
#include <sys/time.h>
#include <sys/socket.h>
#include <string>
#include <sstream>
#include <list>

#include <cerrno>
#include <cstring>

#include "CtrlPortListener.h"

CtrlPortListener::CtrlPortListener(TransmitterData *transmitter_data, DataQueuePtr data_queue)
    : transmitter_data_(transmitter_data), data_queue_(data_queue), rtime_(transmitter_data->getRtime()) {

    // Create CTRL_LISTENER
    ctrl_recv_sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctrl_recv_sock_ < 0) {
        throw CtrlServerCreateException("socket");
    }

    ctrl_ip_mreq_.imr_interface.s_addr = htonl(INADDR_ANY);
    if (inet_aton(transmitter_data_->getMcastAddr().c_str(), &ctrl_ip_mreq_.imr_multiaddr) == 0) {
        throw CtrlServerCreateException("inet_aton");
    }
    if (setsockopt(ctrl_recv_sock_, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<void *>(&ctrl_ip_mreq_),
                   sizeof ctrl_ip_mreq_) < 0) {
        throw CtrlServerCreateException("setsockopt");
    }

    if (setsockopt(ctrl_recv_sock_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<void *>(&ctrl_ip_mreq_),
                   sizeof ctrl_ip_mreq_) < 0) {
        throw CtrlServerCreateException("setsockopt");
    }

    ctrl_recv_address_.sin_family = AF_INET;
    ctrl_recv_address_.sin_addr.s_addr = htonl(INADDR_ANY);
    ctrl_recv_address_.sin_port = htons(transmitter_data_->getCtrlPort());
    if (bind(ctrl_recv_sock_, reinterpret_cast<struct sockaddr *>(&ctrl_recv_address_),
             sizeof ctrl_recv_address_) < 0) {
        throw CtrlServerCreateException("bind");
    }

    // Create REXMIT_SENDER
    rexm_send_sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (rexm_send_sock_ < 0) {
        throw CtrlServerCreateException("socket");
    }

    const static int ENABLE_BROADCAST_VALUE = 1;
    int optval = ENABLE_BROADCAST_VALUE;
    if (setsockopt(rexm_send_sock_, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<void *>(&optval),
                   sizeof optval) < 0) {
        close(rexm_send_sock_);
        throw CtrlServerCreateException("setsockopt broadcast");
    }

    const static int TTL_VALUE = 4;
    optval = TTL_VALUE;
    if (setsockopt(rexm_send_sock_, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<void *>(&optval),
                   sizeof optval) < 0) {
        close(rexm_send_sock_);
        throw CtrlServerCreateException("setsockopt multicast ttl");
    }

    rexm_send_address_.sin_family = AF_INET;
    rexm_send_address_.sin_port = htons(transmitter_data_->getCtrlPort());
    if (inet_aton(transmitter_data_->getMcastAddr().c_str(), &rexm_send_address_.sin_addr) == 0) {
        close(rexm_send_sock_);
        throw CtrlServerCreateException("inet_aton");
    }
    if (connect(rexm_send_sock_, reinterpret_cast<struct sockaddr *>(&rexm_send_address_),
                sizeof rexm_send_address_) < 0) {
        close(rexm_send_sock_);
        throw CtrlServerCreateException("connect");
    }
}

CtrlPortListener::~CtrlPortListener() {
    close(rexm_send_sock_);
    close(ctrl_recv_sock_);
}

void CtrlPortListener::writeRetransmission(CtrlPortListener::Byte *data, size_t data_size){
    ssize_t len;
    len = write(rexm_send_sock_, data, data_size);
    if (static_cast<size_t>(len) != data_size) {
        throw ServerRunException("Error in write");
    }
}

void CtrlPortListener::prepareAndSendRetransmissionPackets(std::vector<NumType> packages_requests) {
    auto packages_to_retransmit = data_queue_->getPackets(packages_requests, transmitter_data_->getPsize());
    for (auto &package_pair : packages_to_retransmit) {
        auto *arr = new Byte[transmitter_data_->getPsize()];
        std::copy(package_pair.second.begin(), package_pair.second.end(), arr);

        auto *pack_to_send = transmitter_data_->packUp(package_pair.first, arr);
        writeRetransmission(pack_to_send, transmitter_data_->getPsize() + TransmitterData::PACKET_HEADER_SIZE);

        delete[] arr;
        delete[] pack_to_send;
    }
}

void CtrlPortListener::handleRetransmissions(std::future<void> future_stopper) {
    std::list<std::future<void>> retransmission_tasks;

    while(future_stopper.wait_for(std::chrono::milliseconds(rtime_)) != std::future_status::ready) {
        if(!rexmit_set_.empty()) {
            std::vector<NumType> packages_requests = rexmit_set_.elements();
            retransmission_tasks.push_back(std::async(
                &CtrlPortListener::prepareAndSendRetransmissionPackets, this, packages_requests));
        }

        for (auto it = retransmission_tasks.begin(); it != retransmission_tasks.end(); ++it) {
            if ((*it).wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                (*it).get();
                it = retransmission_tasks.erase(it);
            }
        }
    }
}

void CtrlPortListener::handleLookup(struct sockaddr_in &client_address) {
    std::stringstream ss;
    ss << "BOREWICZ_HERE " << transmitter_data_->getMcastAddr() << " " << transmitter_data_->getDataPort()
       << " " << transmitter_data_->getNazwa();

    std::string response = ss.str();
    sendto(ctrl_recv_sock_, response.c_str(), response.length(), 0,
           reinterpret_cast<struct sockaddr *>(&client_address), (socklen_t) sizeof(client_address));
}

void CtrlPortListener::handleRexmit(const std::string &rexmit_message) {
    using std::string;

    if (rexmit_message.length() == REXMIT_MSG.length()) {
        return;
    }

    string rexmit_list = rexmit_message.substr(REXMIT_MSG.length() + 1);  // get packages numbers
    char delimiter = ',';
    auto begin = 0UL;
    auto end = rexmit_list.find(delimiter);

    while (end != string::npos) {
        string package_nmb = rexmit_list.substr(begin, end - begin);

        try {
            NumType to_insert = std::stoull(package_nmb);
            rexmit_set_.insert(to_insert);
        }
        catch (std::invalid_argument) {}
        catch (std::out_of_range) {}

        begin = end + 1;
        end = rexmit_list.find(delimiter, begin);
    }

    try {
        NumType to_insert = std::stoull(rexmit_list.substr(begin, end));
        rexmit_set_.insert(to_insert);
    }
    catch (std::invalid_argument) {}
    catch (std::out_of_range) {}
}

void CtrlPortListener::listenOnCtrlPort(std::future<void> future_stopper) {
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = rtime_ * 1000; // milliseconds to microseconds

    if (setsockopt(ctrl_recv_sock_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        throw CtrlServerCreateException("setsockopt rcvtimeo");
    }

    struct timeval tValBefore, tValAfter;
    gettimeofday(&tValBefore, NULL);

    char buffer[BUFFER_SIZE];

    /* waiting for 0 seconds seems like a bad idea (probably slaughtering the processor)
     * but inside of a loop there is a blocking, UNIX-provided function anyway so it is fairly safe
     */
    while(future_stopper.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        struct sockaddr_in client_address;
        socklen_t rcva_len;
        ssize_t rcv_len = 0;

        rcva_len = (socklen_t) sizeof(client_address);
        rcv_len = recvfrom(ctrl_recv_sock_, buffer, sizeof(buffer), 0,
                           (struct sockaddr *) &client_address, &rcva_len);  // blocking

        if (rcv_len < 0 && errno != EAGAIN) {
            throw ServerRunException("recvfrom in CtrlPortListener");
        }
        else if (rcv_len > 0) {
            std::string ctrl_message(buffer, static_cast<unsigned long>(rcv_len));
            if (ctrl_message == LOOKUP_MSG) {
                handleLookup(client_address);
            }
            else if (ctrl_message.length() > REXMIT_MSG.length()
                && ctrl_message.compare(0, REXMIT_MSG.length(), REXMIT_MSG) == 0) {
                handleRexmit(ctrl_message);
            }
        }

        gettimeofday(&tValAfter, NULL);
        long elapsed = (tValAfter.tv_sec - tValBefore.tv_sec) * 1000000
            + tValAfter.tv_usec - tValBefore.tv_usec;

        if (static_cast<NumType>(elapsed) > rtime_ * 1000) {
            gettimeofday(&tValBefore, NULL);

            timeout.tv_sec = 0;
            timeout.tv_usec = rtime_ * 1000; // milliseconds to microseconds
        } else {
            timeout.tv_sec = tValAfter.tv_sec - tValBefore.tv_sec;
            timeout.tv_usec = tValAfter.tv_usec - tValBefore.tv_usec;
        }
        setsockopt(ctrl_recv_sock_, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));
    }
}
