#ifndef CTRLPORTLISTENER_H
#define CTRLPORTLISTENER_H

#include <netinet/in.h>
#include <queue>
#include <memory>

class CtrlPortListener {
private:
    using Byte = uint8_t;
    using DataQueueT = std::queue<Byte>;
    using DataQueuePtr = std::shared_ptr<DataQueueT>;

public:
    explicit CtrlPortListener(in_port_t ctrl_port, DataQueuePtr data_queue, std::string mcast_addr);

private:
    int sock_;
    struct ip_mreq ip_mreq_;
    in_port_t ctrl_port_;
    DataQueuePtr data_queue_;

    std::string mcast_addr_;
};

#endif //CTRLPORTLISTENER_H
