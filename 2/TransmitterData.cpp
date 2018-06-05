#include "TransmitterData.h"

TransmitterData::TransmitterData()
    : mcast_addr_(""), data_port_(DEFAULT_DATA_PORT), ctrl_port_(DEFAULT_CTRL_PORT), psize_(DEFAULT_PSIZE),
      fsize_(DEFAULT_FSIZE), rtime_(DEFAULT_RTIME), nazwa_(DEFAULT_NAZWA) {

    session_id_ = static_cast<uint64_t>(time(NULL));
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
void TransmitterData::setPsize(uint64_t psize) {
    psize_ = psize;
}
void TransmitterData::setFsize(uint64_t fsize) {
    fsize_ = fsize;
}
void TransmitterData::setRtime(uint64_t rtime) {
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
uint64_t TransmitterData::getPsize() const {
    return psize_;
}
uint64_t TransmitterData::getFsize() const {
    return fsize_;
}
uint64_t TransmitterData::getRtime() const {
    return rtime_;
}
const std::string &TransmitterData::getNazwa() const {
    return nazwa_;
}

void htonull(TransmitterData::Byte *data, size_t start_index, uint64_t val) {
    static const int num = 42;

    // Check the endianness
    if (*reinterpret_cast<const char*>(&num) == num) {
        data[start_index] = val >> 56 & 0xFF;
        data[start_index + 1] = val >> 48 & 0xFF;
        data[start_index + 2] = val >> 40 & 0xFF;
        data[start_index + 3] = val >> 32 & 0xFF;
        data[start_index + 4] = val >> 24 & 0xFF;
        data[start_index + 5] = val >> 16 & 0xFF;
        data[start_index + 6] = val >> 8 & 0xFF;
        data[start_index + 7] = val >> 0 & 0xFF;
    }
    else {
        data[start_index] = val >> 0;
        data[start_index + 1] = val >> 8;
        data[start_index + 2] = val >> 16;
        data[start_index + 3] = val >> 24;
        data[start_index + 4] = val >> 32;
        data[start_index + 5] = val >> 40;
        data[start_index + 6] = val >> 48;
        data[start_index + 7] = val >> 56;
    }
}

TransmitterData::Byte *TransmitterData::packUp(uint64_t first_byte_num, TransmitterData::Byte *audio_data) {
    auto *to_return = new Byte[psize_ + PACKET_HEADER_SIZE];

    htonull(to_return, 0, session_id_);
    htonull(to_return, 8, first_byte_num);

    for (uint64_t i = 0; i < psize_; ++i) {
        to_return[i + 16] = audio_data[i];
    }

    return to_return;
}
