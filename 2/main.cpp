#include <iostream>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <string>
#include <netinet/in.h>

#include "Transmitter.h"

std::pair<bool, char **> cmdOptionExists(char** begin, char** end, const std::string& option) {
    char **it = std::find(begin, end, option);
    return std::make_pair(it != end && ++it != end, it);
}

int parseCommandLineArgs(int argc, char *argv[], std::unique_ptr<Transmitter> &transmitter) {
    if ((argc - 1) % 2 != 0) {
        std::cerr << "Wrong number of arguments" << std::endl;
        return 1;
    }

    auto mcast_addr_arg_pair = cmdOptionExists(argv, argv + argc, "-a");
    if (!mcast_addr_arg_pair.first) {
        std::cerr << "Usage: " << argv[0] << " -a mcast_addr_" << std::endl;
        return 1;
    }

    transmitter = std::make_unique<Transmitter>(*(mcast_addr_arg_pair.second));

    uint64_t unsigned_arg;
    in_port_t port_arg;

    for (int i = 1; i < argc; i += 2) {
        if (!strcmp(argv[i], "-a")) {
            continue;
        }
        else if (!strcmp(argv[i], "-P")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> port_arg)) {
                std::cerr << "Invalid data_port_ argument" << std::endl;
                return 1;
            }
            transmitter->setDataPort(port_arg);
        }
        else if (!strcmp(argv[i], "-C")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> port_arg)) {
                std::cerr << "Invalid ctrl_port_ argument" << std::endl;
                return 1;
            }
            transmitter->setCtrlPort(port_arg);
        }
        else if (!strcmp(argv[i], "-p")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> unsigned_arg)) {
                std::cerr << "Invalid psize_ argument" << std::endl;
                return 1;
            }
            transmitter->setPsize(unsigned_arg);
        }
        else if (!strcmp(argv[i], "-f")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> unsigned_arg)) {
                std::cerr << "Invalid fsize_ argument" << std::endl;
                return 1;
            }
            transmitter->setFsize(unsigned_arg);
        }
        else if (!strcmp(argv[i], "-R")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss >> unsigned_arg)) {
                std::cerr << "Invalid rtime_ argument" << std::endl;
                return 1;
            }
            transmitter->setRtime(unsigned_arg);
        }
        else if (!strcmp(argv[i], "-n")) {
            std::string nazwa = std::string(argv[i + 1]);
            if (nazwa.empty() || nazwa.length() > 64) {
                std::cerr << "Invalid nazwa_ argument (should be at most 64 characters)" << std::endl;
                return 1;
            }
            transmitter->setNazwa(nazwa);
        }
        else {
            std::cerr << "Invalid arguments, proper usage:\n"
                         << argv[0] << " -a mcast_addr_" << " [-P data_port_]" << " [-C ctrl_port_]" << " [-p psize_]"
                         << " [-f fsize_]" << " [-R rtime_]" << " [-n nazwa_]" << std::endl;
            return 1;
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    std::unique_ptr<Transmitter> transmitter;

    if (parseCommandLineArgs(argc, argv, transmitter) != 0) {
        return 1;
    }

    try {
        transmitter->startTransmitter();
    }
    catch (ServerException &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    transmitter->printTransmitter();
    transmitter->readStdIn();

    return 0;
}