#include "CtrlPortListener.h"

/*void startTransmitter() {
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        throw ServerCreateException("socket");
    }

    ip_mreq_.imr_interface.s_addr = htonl(INADDR_ANY);
    if (inet_aton(mcast_addr_.c_str(), &ip_mreq_.imr_multiaddr) == 0) {
        throw ServerCreateException("inet_aton");
    }
    /* podpięcie się do grupy rozsyłania *
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&ip_mreq_, sizeof ip_mreq_) < 0) {
        throw ServerCreateException("setsockopt");
    }

    /* podpięcie się pod lokalny adres i port *
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(local_port);
    if (bind(sock, (struct sockaddr *)&local_address, sizeof local_address) < 0) {
        throw ServerCreateException("bind");
    }
}*/

CtrlPortListener::CtrlPortListener(in_port_t ctrl_port, DataQueuePtr data_queue, std::string mcast_addr)
    : ctrl_port_(ctrl_port), data_queue_(data_queue), mcast_addr_(mcast_addr) {

}
