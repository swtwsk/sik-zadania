#ifndef CTRLPORTLISTENER_H
#define CTRLPORTLISTENER_H

#include <netinet/in.h>
#include <queue>
#include <memory>
#include <future>

#include "TransmitterData.h"
#include "ConcurrentContainers.h"
#include "ServerException.h"

class CtrlPortListener {
private:
    using Byte = uint8_t;
    using DataQueueT = ConcurrentQueue<Byte>;
    using DataQueuePtr = std::shared_ptr<DataQueueT>;
    using RexmitSetT = ConcurrentSet<uint64_t>;

public:
    explicit CtrlPortListener(TransmitterData *transmitter_data, DataQueuePtr data_queue);
    ~CtrlPortListener();

    void writeToRexmitCast(Byte *data, size_t data_size);
    void rexmitQueue(std::future<void> futureStopper);

    void handleLookup(struct sockaddr_in &client_address);
    void handleRexmit(const std::string &rexmit_message);
    void listenOnCtrlPort(std::future<void> futureStopper);

private:
    // CTRL_LISTENER
    int ctrl_recv_sock_;
    struct sockaddr_in ctrl_recv_address_;
    struct ip_mreq ctrl_ip_mreq_;

    // REXMIT_SENDER
    int rexm_send_sock_;
    struct sockaddr_in rexm_send_address_;

    TransmitterData *transmitter_data_;
    DataQueuePtr data_queue_;
    ConcurrentSet<uint64_t> rexmit_set_;

    uint64_t rtime_;

    const std::string LOOKUP_MSG = "ZERO_SEVEN_COME_IN";
    const std::string REXMIT_MSG = "LOUDER_PLEASE";
    const static size_t BUFFER_SIZE = 65536; // MAX UDP PACKET SIZE (including IP & UPD headers)
};

#endif //CTRLPORTLISTENER_H
