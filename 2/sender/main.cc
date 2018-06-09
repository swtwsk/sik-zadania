#include <iostream>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <string>
#include <netinet/in.h>
#include <regex>

#include "TransmitterData.h"
#include "Transmitter.h"
#include "ServerException.h"

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
    char **it = std::find(begin, end, option);
    return it != end && ++it != end;
}

bool checkIPAddr(const std::string &ip_addr) {
    std::regex ip_addr_regex("(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                             "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                             "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                             "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])");


    return std::regex_match(ip_addr, ip_addr_regex);
}

int parseCommandLineArgs(int argc, char *argv[], TransmitterData &transmitter_data) {
    if ((argc - 1) % 2 != 0) {
        std::cerr << "Wrong number of arguments" << std::endl;
        return 1;
    }

    if (!cmdOptionExists(argv, argv + argc, "-a")) {
        std::cerr << "Usage: " << argv[0] << " -a mcast_addr" << std::endl;
        return 1;
    }

    uint64_t unsigned_arg;
    in_port_t port_arg;
    std::string string_arg;

    for (int i = 1; i < argc; i += 2) {
        if (!strcmp(argv[i], "-a")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> string_arg)) {
                std::cerr << "Invalid mcast_addr argument" << std::endl;
                return 1;
            }
            if (!checkIPAddr(string_arg)) {
                std::cerr << "Invalid mcast_addr argument" << std::endl;
                return 1;
            }
            transmitter_data.setMcastAddr(string_arg);
        }
        else if (!strcmp(argv[i], "-P")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> port_arg)) {
                std::cerr << "Invalid data_port argument" << std::endl;
                return 1;
            }
            transmitter_data.setDataPort(port_arg);
        }
        else if (!strcmp(argv[i], "-C")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> port_arg)) {
                std::cerr << "Invalid ctrl_port argument" << std::endl;
                return 1;
            }
            transmitter_data.setCtrlPort(port_arg);
        }
        else if (!strcmp(argv[i], "-p")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> unsigned_arg)) {
                std::cerr << "Invalid psize argument" << std::endl;
                return 1;
            }
            transmitter_data.setPsize(unsigned_arg);
        }
        else if (!strcmp(argv[i], "-f")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> unsigned_arg)) {
                std::cerr << "Invalid fsize argument" << std::endl;
                return 1;
            }
            transmitter_data.setFsize(unsigned_arg);
        }
        else if (!strcmp(argv[i], "-R")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> unsigned_arg)) {
                std::cerr << "Invalid rtime argument" << std::endl;
                return 1;
            }
            transmitter_data.setRtime(unsigned_arg);
        }
        else if (!strcmp(argv[i], "-n")) {
            std::string nazwa = std::string(argv[i + 1]);
            if (nazwa.empty() || nazwa.length() > 64) {
                std::cerr << "Invalid nazwa argument (should be at most 64 characters)" << std::endl;
                return 1;
            }
            transmitter_data.setNazwa(nazwa);
        }
        else {
            std::cerr << "Invalid arguments, proper usage:\n"
                         << argv[0] << " -a mcast_addr" << " [-P data_port]" << " [-C ctrl_port]" << " [-p psize]"
                         << " [-f fsize]" << " [-R rtime]" << " [-n nazwa]" << std::endl;
            return 1;
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    TransmitterData transmitter_data;

    if (parseCommandLineArgs(argc, argv, transmitter_data) != 0) {
        return 1;
    }

    std::unique_ptr<Transmitter> transmitter = std::make_unique<Transmitter>(&transmitter_data);

    try {
        transmitter->startTransmitter();
    }
    catch (TCPServerException &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    transmitter->readStdIn();
    transmitter->stopTransmitter();

    return 0;
}