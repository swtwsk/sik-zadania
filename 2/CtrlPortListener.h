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
    explicit CtrlPortListener(in_port_t ctrl_port, DataQueuePtr data_queue, std::string mcast_addr);
    ~CtrlPortListener();

    // DEBUG
    void printQueue(std::future<void> futureStopper);

private:
    int sock_;
    struct sockaddr_in local_address_;
    struct ip_mreq ip_mreq_;
    in_port_t ctrl_port_;
    DataQueuePtr data_queue_;

    std::string mcast_addr_;
};

#endif //CTRLPORTLISTENER_H
