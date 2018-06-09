#include <cstring>
#include <endian.h>

#include "TransmitterData.h"

TransmitterData::TransmitterData()
    : mcast_addr_(""), data_port_(DEFAULT_DATA_PORT), ctrl_port_(DEFAULT_CTRL_PORT), psize_(DEFAULT_PSIZE),
      fsize_(DEFAULT_FSIZE), rtime_(DEFAULT_RTIME), nazwa_(DEFAULT_NAZWA) {

    session_id_ = static_cast<NumType>(time(NULL));
}

void TransmitterData::setMcastAddr(const std::string &mcast_addr) {
    mcast_addr_ = mcast_addr;
}
void TransmitterData::setDataPort(in_port_t data_port) {
    data_port_ = data_port;
}
void TransmitterData::setCtrlPort(in_port_t ctrl_port) {
    ctrl_port_ = ctrl_port;
}
void TransmitterData::setPsize(NumType psize) {
    psize_ = psize;
}
void TransmitterData::setFsize(NumType fsize) {
    fsize_ = fsize;
}
void TransmitterData::setRtime(NumType rtime) {
    rtime_ = rtime;
}
void TransmitterData::setNazwa(const std::string &nazwa) {
    nazwa_ = nazwa;
}

const std::string &TransmitterData::getMcastAddr() const {
    return mcast_addr_;
}
in_port_t TransmitterData::getDataPort() const {
    return data_port_;
}
in_port_t TransmitterData::getCtrlPort() const {
    return ctrl_port_;
}
TransmitterData::NumType TransmitterData::getPsize() const {
    return psize_;
}
TransmitterData::NumType TransmitterData::getFsize() const {
    return fsize_;
}
TransmitterData::NumType TransmitterData::getRtime() const {
    return rtime_;
}
const std::string &TransmitterData::getNazwa() const {
    return nazwa_;
}

void htonull(TransmitterData::Byte *data, size_t start_index, TransmitterData::NumType val) {
    uint64_t to_send = htobe64(val);
    std::memcpy(data + start_index, &to_send, sizeof (to_send));
}

TransmitterData::Byte *TransmitterData::packUp(NumType first_byte_num, TransmitterData::Byte *audio_data) {
    auto *to_return = new Byte[psize_ + PACKET_HEADER_SIZE];

    htonull(to_return, 0, session_id_);
    htonull(to_return, 8, first_byte_num);

    std::memcpy(to_return + PACKET_HEADER_SIZE, audio_data, psize_ * sizeof(Byte));

    return to_return;
}
