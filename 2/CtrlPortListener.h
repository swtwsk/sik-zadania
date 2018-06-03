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

    void startCtrlPortListener();

    void writeToRexmitCast(Byte *data, size_t data_size);
    void rexmitQueue(std::future<void> futureStopper);

    void handleLookup(struct sockaddr_in &client_address);
    void handleRexmit(const std::string &rexmit_message, bool msg_beginning);
    void listenOnCtrlPort(std::future<void> futureStopper);

private:
    void createCtrlListener();
    void createRexmitSender();

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

    const std::string LOOKUP_MSG = "ZERO_SEVEN_COME_IN";
    const std::string REXMIT_MSG = "LOUDER_PLEASE";
};

#endif //CTRLPORTLISTENER_H
