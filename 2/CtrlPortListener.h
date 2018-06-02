#ifndef CTRLPORTLISTENER_H
#define CTRLPORTLISTENER_H

#include <netinet/in.h>
#include <queue>
#include <memory>

#include "ConcurrentQueue.h"
#include "ServerException.h"

class CtrlPortListener {
private:
    using Byte = uint8_t;
    using DataQueueT = ConcurrentQueue<Byte>;
    using DataQueuePtr = std::shared_ptr<DataQueueT>;

public:
    explicit CtrlPortListener(in_port_t ctrl_port, DataQueuePtr data_queue, std::string mcast_addr, uint64_t psize);
    ~CtrlPortListener();

    void writeToRexmitCast(Byte *data, size_t data_size);
    void rexmitQueue(std::future<void> futureStopper);
    void listenOnCtrlPort(std::future<void> futureStopper);

private:
    // CTRL_LISTENER
    int ctrl_recv_sock_;
    struct sockaddr_in ctrl_recv_address_;
    struct ip_mreq ctrl_ip_mreq_;
    in_port_t ctrl_port_;

    //REXMIT_SENDER
    int rexm_send_sock_;
    struct sockaddr_in rexm_send_address_;
    in_port_t rexm_port_;

    DataQueuePtr data_queue_;
    std::string mcast_addr_;
    uint64_t psize_;
};

#endif //CTRLPORTLISTENER_H
