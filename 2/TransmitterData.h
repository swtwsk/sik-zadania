#ifndef TRANSMITTERDATA_H
#define TRANSMITTERDATA_H

#include <netinet/in.h>
#include <string>

class TransmitterData {
public:
    explicit TransmitterData()
        : mcast_addr_(""), data_port_(DEFAULT_DATA_PORT), ctrl_port_(DEFAULT_CTRL_PORT), psize_(DEFAULT_PSIZE),
          fsize_(DEFAULT_FSIZE), rtime_(DEFAULT_RTIME), nazwa_(DEFAULT_NAZWA) {

        data_size_ = psize_ - 2 * sizeof(uint64_t);
        session_id_ = static_cast<uint64_t>(time(NULL));
    }

    void setMcastAddr(const std::string &mcast_addr) {
        mcast_addr_ = mcast_addr;
    }
    void setDataPort(in_port_t data_port) {
        data_port_ = data_port;
    }
    void setCtrlPort(in_port_t ctrl_port) {
        ctrl_port_ = ctrl_port;
    }
    void setPsize(uint64_t psize) {
        psize_ = psize;
        data_size_ = psize_ - 2 * sizeof(uint64_t);
    }
    void setFsize(uint64_t fsize) {
        fsize_ = fsize;
    }
    void setRtime(uint64_t rtime) {
        rtime_ = rtime;
    }
    void setNazwa(const std::string &nazwa) {
        nazwa_ = nazwa;
    }

    const std::string &getMcastAddr() const {
        return mcast_addr_;
    }
    in_port_t getDataPort() const {
        return data_port_;
    }
    in_port_t getCtrlPort() const {
        return ctrl_port_;
    }
    uint64_t getPsize() const {
        return psize_;
    }
    uint64_t getFsize() const {
        return fsize_;
    }
    uint64_t getRtime() const {
        return rtime_;
    }
    const std::string &getNazwa() const {
        return nazwa_;
    }
    uint64_t getDataSize() const {
        return data_size_;
    }
    uint64_t getSessionId() const {
        return session_id_;
    }

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

    uint64_t data_size_;
    uint64_t session_id_;
};

#endif //TRANSMITTERDATA_H
