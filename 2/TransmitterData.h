#ifndef TRANSMITTERDATA_H
#define TRANSMITTERDATA_H

#include <netinet/in.h>
#include <string>

class TransmitterData {
public:
    using Byte = uint8_t;

    explicit TransmitterData();

    void setMcastAddr(const std::string &mcast_addr);
    void setDataPort(in_port_t data_port);
    void setCtrlPort(in_port_t ctrl_port);
    void setPsize(uint64_t psize);
    void setFsize(uint64_t fsize);
    void setRtime(uint64_t rtime);
    void setNazwa(const std::string &nazwa);

    const std::string &getMcastAddr() const;
    in_port_t getDataPort() const;
    in_port_t getCtrlPort() const;
    uint64_t getPsize() const;
    uint64_t getFsize() const;
    uint64_t getRtime() const;
    const std::string &getNazwa() const;

    Byte *packUp(uint64_t first_byte_num, Byte *audio_data);

    const static uint64_t PACKET_HEADER_SIZE = 2 * sizeof(uint64_t);

private:
    const in_port_t DEFAULT_DATA_PORT = 26085;
    const in_port_t DEFAULT_CTRL_PORT = 36085;
    const uint64_t DEFAULT_PSIZE = 512;
    const uint64_t DEFAULT_FSIZE = 131072;
    const uint64_t DEFAULT_RTIME = 250;
    const std::string DEFAULT_NAZWA = "Nienazwany Nadajnik";

    std::string mcast_addr_;
    in_port_t data_port_;
    in_port_t ctrl_port_;
    uint64_t psize_;
    uint64_t fsize_;
    uint64_t rtime_;
    std::string nazwa_;

    uint64_t session_id_;
};

#endif //TRANSMITTERDATA_H
