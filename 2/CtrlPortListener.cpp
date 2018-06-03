#include <arpa/inet.h>
#include <unistd.h>
#include <future>
#include <iostream>
#include <sys/time.h>
#include <sys/socket.h>
#include <string>
#include <sstream>

#include <cerrno>
#include <cstring>

#include "CtrlPortListener.h"

CtrlPortListener::CtrlPortListener(TransmitterData *transmitter_data, DataQueuePtr data_queue)
    : transmitter_data_(transmitter_data), data_queue_(data_queue) {}

CtrlPortListener::~CtrlPortListener() {
    close(rexm_send_sock_);
    close(ctrl_recv_sock_);
}

void CtrlPortListener::writeToRexmitCast(CtrlPortListener::Byte *data, size_t data_size){
    ssize_t len;
    len = write(rexm_send_sock_, data, data_size);
    if (static_cast<size_t>(len) != data_size) {
        throw ServerRunException("Error in write");
    }

    delete[] data;
}

void CtrlPortListener::rexmitQueue(std::future<void> futureStopper) {
    while(futureStopper.wait_for(std::chrono::seconds(5)) != std::future_status::ready) {
        if (!data_queue_->empty()) {

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
    //std::cout << "LOOKUP: " << ss.str() << std::endl;
}

void CtrlPortListener::handleRexmit(const std::string &rexmit_message, bool msg_beginning) {
    using std::string;

    std::cout << "REXMIT" << std::endl;
    if (rexmit_message.length() == REXMIT_MSG.length()) {
        return;
    }

    string rexmit_list = rexmit_message.substr(REXMIT_MSG.length() + 1);  // get packages numbers
    char delimiter = ',';
    auto begin = 0UL;
    auto end = rexmit_list.find(delimiter);

    while (end != string::npos) {
        string package_nmb = rexmit_list.substr(begin, end - begin);
        std::cout << package_nmb << " ";
        try {
            rexmit_set_.insert(std::stoull(package_nmb));
        }
        catch (std::invalid_argument) {  // TODO: Implement catching errors

        }
        catch (std::out_of_range) {

        }
        begin = end + 1;
        end = rexmit_list.find(delimiter, begin);
    }

    // TODO: Ask for moar!
    std::cout << rexmit_list.substr(begin, end) << std::endl;
}

void CtrlPortListener::listenOnCtrlPort(std::future<void> futureStopper) {
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    if (setsockopt(ctrl_recv_sock_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        throw CtrlServerCreateException("setsockopt rcvtimeo");
    }

    struct timeval tValBefore, tValAfter;
    gettimeofday(&tValBefore, NULL);

    char buffer[1024];
    bool continue_rexmit_msg = false;

    /* waiting for 0 seconds seems like a bad idea (probably slaughtering the processor)
     * but inside of a loop there is a blocking, UNIX-provided function anyway so it is fairly safe
     */
    while(futureStopper.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
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
                handleRexmit(ctrl_message, true);
                continue_rexmit_msg = true;
            }
            else if (continue_rexmit_msg) {
                handleRexmit(ctrl_message, false);
            }
        }

        gettimeofday(&tValAfter, NULL);
        long elapsed = (tValAfter.tv_sec - tValBefore.tv_sec) * 1000000
            + tValAfter.tv_usec - tValBefore.tv_usec;

        if (elapsed > 5000000) {
            gettimeofday(&tValBefore, NULL);

            timeout.tv_sec = 5;
            timeout.tv_usec = 0;
        } else {
            timeout.tv_sec = tValAfter.tv_sec - tValBefore.tv_sec;
            timeout.tv_usec = tValAfter.tv_usec - tValBefore.tv_usec;
        }
        setsockopt(ctrl_recv_sock_, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));
    }
}

void CtrlPortListener::startCtrlPortListener() {
    createCtrlListener();
    createRexmitSender();
}

void CtrlPortListener::createCtrlListener() {
    // Create CTRL_LISTENER
    ctrl_recv_sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctrl_recv_sock_ < 0) {
        throw CtrlServerCreateException("socket");
    }

    ctrl_ip_mreq_.imr_interface.s_addr = htonl(INADDR_ANY);
    if (inet_aton(transmitter_data_->getMcastAddr().c_str(), &ctrl_ip_mreq_.imr_multiaddr) == 0) {
        throw CtrlServerCreateException("inet_aton");
    }
    if (setsockopt(ctrl_recv_sock_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&ctrl_ip_mreq_, sizeof ctrl_ip_mreq_) < 0) {
        throw CtrlServerCreateException("setsockopt");
    }

    if (setsockopt(ctrl_recv_sock_, SOL_SOCKET, SO_REUSEADDR, (void*)&ctrl_ip_mreq_, sizeof ctrl_ip_mreq_) < 0) {
        throw CtrlServerCreateException("setsockopt");
    }

    ctrl_recv_address_.sin_family = AF_INET;
    ctrl_recv_address_.sin_addr.s_addr = htonl(INADDR_ANY);
    ctrl_recv_address_.sin_port = htons(transmitter_data_->getCtrlPort());
    if (bind(ctrl_recv_sock_, (struct sockaddr *)&ctrl_recv_address_, sizeof ctrl_recv_address_) < 0) {
        throw CtrlServerCreateException("bind");
    }
}

void CtrlPortListener::createRexmitSender() {
    rexm_send_sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (rexm_send_sock_ < 0) {
        throw CtrlServerCreateException("socket");
    }

    int optval = 1;
    if (setsockopt(rexm_send_sock_, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<void *>(&optval), sizeof optval) < 0) {
        close(rexm_send_sock_);
        throw CtrlServerCreateException("setsockopt broadcast");
    }

    int TTL_VALUE = 4;
    optval = TTL_VALUE;
    if (setsockopt(rexm_send_sock_, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<void *>(&optval), sizeof optval) < 0) {
        close(rexm_send_sock_);
        throw CtrlServerCreateException("setsockopt multicast ttl");
    }

    rexm_send_address_.sin_family = AF_INET;
    rexm_send_address_.sin_port = htons(transmitter_data_->getCtrlPort() + 2); // TODO: CHECK IT!!!!
    if (inet_aton(transmitter_data_->getMcastAddr().c_str(), &rexm_send_address_.sin_addr) == 0) {
        close(rexm_send_sock_);
        throw CtrlServerCreateException("inet_aton");
    }
    if (connect(rexm_send_sock_, reinterpret_cast<struct sockaddr *>(&rexm_send_address_), sizeof rexm_send_address_) < 0) {
        close(rexm_send_sock_);
        throw CtrlServerCreateException("connect");
    }
}
