#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <memory>
#include <netinet/in.h>
#include <cstring>

struct Transmitter {
    const uint64_t DEFAULT_DATA_PORT = 26085;
    const uint64_t DEFAULT_CTRL_PORT = 36085;
    const uint64_t DEFAULT_PSIZE = 512;
    const uint64_t DEFAULT_FSIZE = 131072;
    const uint64_t DEFAULT_RTIME = 250;
    const std::string DEFAULT_NAZWA = "Nienazwany Nadajnik";

    std::string MCAST_ADDR;
    uint64_t DATA_PORT;
    uint64_t CTRL_PORT;
    uint64_t PSIZE;
    uint64_t FSIZE;
    uint64_t RTIME;
    std::string NAZWA;

    Transmitter(const std::string &MCAST_ADDR_ARG)
        : MCAST_ADDR(MCAST_ADDR_ARG), DATA_PORT(DEFAULT_DATA_PORT), CTRL_PORT(DEFAULT_CTRL_PORT), PSIZE(DEFAULT_PSIZE),
          FSIZE(DEFAULT_FSIZE), RTIME(DEFAULT_RTIME), NAZWA(DEFAULT_NAZWA) {}
};

std::pair<bool, char **> cmdOptionExists(char** begin, char** end, const std::string& option)
{
    char **it = std::find(begin, end, option);
    return std::make_pair(it != end && ++it != end, it);
}

int parseCommandLineArgs(int argc, char *argv[], std::unique_ptr<Transmitter> &transmitter) {
    if ((argc - 1) % 2 != 0) {
        std::cout << "Wrong number of arguments" << std::endl;
        return 1;
    }

    auto mcast_addr_arg_pair = cmdOptionExists(argv, argv + argc, "-a");
    if (!mcast_addr_arg_pair.first) {
        std::cout << "Usage: " << argv[0] << " -a MCAST_ADDR" << std::endl;
        return 1;
    }

    transmitter = std::make_unique<Transmitter>(*(mcast_addr_arg_pair.second));

    for (int i = 1; i < argc; i += 2) {
        if (!strcmp(argv[i], "-a")) {
            continue;
        }
        else if (!strcmp(argv[i], "-P")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss>>transmitter->DATA_PORT)) {
                std::cout << "Invalid DATA_PORT argument" << std::endl;
                return 1;
            }
        }
        else if (!strcmp(argv[i], "-C")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss>>transmitter->CTRL_PORT)) {
                std::cout << "Invalid CTRL_PORT argument" << std::endl;
                return 1;
            }
        }
        else if (!strcmp(argv[i], "-p")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss>>transmitter->PSIZE)) {
                std::cout << "Invalid PSIZE argument" << std::endl;
                return 1;
            }
        }
        else if (!strcmp(argv[i], "-f")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss>>transmitter->FSIZE)) {
                std::cout << "Invalid FSIZE argument" << std::endl;
                return 1;
            }
        }
        else if (!strcmp(argv[i], "-R")) {
            std::istringstream ss(argv[i + 1]);
            if (!(ss>>transmitter->RTIME)) {
                std::cout << "Invalid RTIME argument" << std::endl;
                return 1;
            }
        }
        else if (!strcmp(argv[i], "-n")) {
            transmitter->NAZWA = std::string(argv[i + 1]);
            if (transmitter->NAZWA.empty() || transmitter->NAZWA.length() > 64) {
                std::cout << "Invalid NAZWA argument (should be at most 64 characters)" << std::endl;
                return 1;
            }
        }
        else {
            std::cout << "Invalid arguments, proper usage:\n"
                         << argv[0] << " -a MCAST_ADDR" << " [-P DATA_PORT]" << " [-C CTRL_PORT]" << " [-p PSIZE]"
                         << " [-f FSIZE]" << " [-R RTIME]" << " [-n NAZWA]" << std::endl;
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

    std::cout << "TRANSMITTER:\n  MCAST_ADDR = " << transmitter->MCAST_ADDR
              << "\n  DATA_PORT = " << transmitter->DATA_PORT << "\n  CTRL_PORT = " << transmitter->CTRL_PORT
              << "\n  PSIZE = " << transmitter->PSIZE << "\n  FSIZE = " << transmitter->FSIZE
              << "\n  RTIME = " << transmitter->RTIME << "\n  NAZWA = " << transmitter->NAZWA << std::endl;

    return 0;
}