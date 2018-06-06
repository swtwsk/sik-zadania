#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <iostream>
#include <string>
#include <memory>
#include <netinet/in.h>
#include <future>
#include <thread>
#include <deque>

#include "TransmitterData.h"
#include "CtrlPortListener.h"
#include "ConcurrentContainers.h"
#include "ServerException.h"

class Transmitter {
public:
    using Byte = TransmitterData::Byte;

    explicit Transmitter(TransmitterData *transmitter_data);

    void startTransmitter();
    void stopTransmitter();

    void readStdIn();
    void writeToClient(Byte *data, size_t data_size);

private:
    using NumType = TransmitterData::NumType;
    using DataQueueT = ConcurrentDeque<Byte>;
    using DataQueuePtr = std::shared_ptr<DataQueueT>;
    using CtrlPortListenerPtr = CtrlPortListener *;

    int sock_;
    struct sockaddr_in remote_address_;

    TransmitterData *transmitter_data_;
    DataQueuePtr data_queue_;
    CtrlPortListenerPtr ctrl_port_listener_;

    std::promise<void> exit_rexmit_signal_;
    std::promise<void> exit_listener_signal_;
    std::thread ctrl_port_rexmit_thread_;
    std::thread ctrl_port_listener_thread_;
};

#endif //TRANSMITTER_H
