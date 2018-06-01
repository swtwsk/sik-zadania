#include <arpa/inet.h>
#include <unistd.h>
#include <future>
#include <iostream>
#include "CtrlPortListener.h"

CtrlPortListener::CtrlPortListener(in_port_t ctrl_port, DataQueuePtr data_queue, std::string mcast_addr)
    : ctrl_port_(ctrl_port), data_queue_(data_queue), mcast_addr_(mcast_addr) {

    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ < 0) {
        throw CtrlServerCreateException("socket");
    }

    ip_mreq_.imr_interface.s_addr = htonl(INADDR_ANY);
    if (inet_aton(mcast_addr_.c_str(), &ip_mreq_.imr_multiaddr) == 0) {
        throw CtrlServerCreateException("inet_aton");
    }
    /* podpięcie się do grupy rozsyłania */
    if (setsockopt(sock_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&ip_mreq_, sizeof ip_mreq_) < 0) {
        throw CtrlServerCreateException("setsockopt");
    }

    /* podpięcie się pod lokalny adres i port */
    local_address_.sin_family = AF_INET;
    local_address_.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address_.sin_port = htons(ctrl_port_);
    if (bind(sock_, (struct sockaddr *)&local_address_, sizeof local_address_) < 0) {
        throw CtrlServerCreateException("bind");
    }
}

CtrlPortListener::~CtrlPortListener() {
    close(sock_);
}

void CtrlPortListener::printQueue(std::future<void> futureStopper) {
    while(futureStopper.wait_for(std::chrono::seconds(1)) != std::future_status::ready) {
        std::cout << data_queue_->pop().first << std::endl;
    }
}