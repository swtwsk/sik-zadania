#include <arpa/inet.h>
#include <unistd.h>
#include <future>
#include <iostream>
#include <sys/time.h>
#include "CtrlPortListener.h"

CtrlPortListener::CtrlPortListener(in_port_t ctrl_port, DataQueuePtr data_queue, std::string mcast_addr, uint64_t psize)
    : ctrl_port_(ctrl_port), rexm_port_(ctrl_port), data_queue_(data_queue), mcast_addr_(mcast_addr), psize_(psize) {

    // Create CTRL_LISTENER
    ctrl_recv_sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctrl_recv_sock_ < 0) {
        throw CtrlServerCreateException("socket");
    }

    ctrl_ip_mreq_.imr_interface.s_addr = htonl(INADDR_ANY);
    if (inet_aton(mcast_addr_.c_str(), &ctrl_ip_mreq_.imr_multiaddr) == 0) {
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
    ctrl_recv_address_.sin_port = htons(ctrl_port_);
    if (bind(ctrl_recv_sock_, (struct sockaddr *)&ctrl_recv_address_, sizeof ctrl_recv_address_) < 0) {
        throw CtrlServerCreateException("bind");
    }

    // Create REXMIT_SENDER
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
    rexm_send_address_.sin_port = htons(rexm_port_);
    if (inet_aton(mcast_addr_.c_str(), &rexm_send_address_.sin_addr) == 0) {
        close(rexm_send_sock_);
        throw CtrlServerCreateException("inet_aton");
    }
    if (connect(rexm_send_sock_, reinterpret_cast<struct sockaddr *>(&rexm_send_address_), sizeof rexm_send_address_) < 0) {
        close(rexm_send_sock_);
        throw CtrlServerCreateException("connect");
    }
}

CtrlPortListener::~CtrlPortListener() {
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

            //std::cout << data_queue_->pop().first << std::endl;
        }
    }
}
void CtrlPortListener::listenOnCtrlPort(std::future<void> futureStopper) {
    // create timeout on receive
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    if (setsockopt (ctrl_recv_sock_, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        throw CtrlServerCreateException("setsockopt");
    }

    /* waiting for 0 seconds seems like a bad idea (probably slaughtering the processor)
     * but inside of a loop there is a blocking, UNIX-provided function anyway so it is fairly safe
     */
    while(futureStopper.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        
    }
}
