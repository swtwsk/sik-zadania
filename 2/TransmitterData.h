#ifndef TRANSMITTERDATA_H
#define TRANSMITTERDATA_H

#include <netinet/in.h>
#include <string>

class TransmitterData {
public:
    using Byte = uint8_t;
    using NumType = uint64_t;

    explicit TransmitterData();

    void setMcastAddr(const std::string &mcast_addr);
    void setDataPort(in_port_t data_port);
    void setCtrlPort(in_port_t ctrl_port);
    void setPsize(NumType psize);
    void setFsize(NumType fsize);
    void setRtime(NumType rtime);
    void setNazwa(const std::string &nazwa);

    const std::string &getMcastAddr() const;
    in_port_t getDataPort() const;
    in_port_t getCtrlPort() const;
    NumType getPsize() const;
    NumType getFsize() const;
    NumType getRtime() const;
    const std::string &getNazwa() const;

    Byte *packUp(NumType first_byte_num, Byte *audio_data);

    const static NumType PACKET_HEADER_SIZE = 2 * sizeof(NumType);

private:
    const in_port_t DEFAULT_DATA_PORT = 26085;
    const in_port_t DEFAULT_CTRL_PORT = 36085;
    const NumType DEFAULT_PSIZE = 512;
    const NumType DEFAULT_FSIZE = 131072;
    const NumType DEFAULT_RTIME = 250;
    const std::string DEFAULT_NAZWA = "Nienazwany Nadajnik";

    std::string mcast_addr_;
    in_port_t data_port_;
    in_port_t ctrl_port_;
    NumType psize_;
    NumType fsize_;
    NumType rtime_;
    std::string nazwa_;

    NumType session_id_;
};

#endif //TRANSMITTERDATA_H
