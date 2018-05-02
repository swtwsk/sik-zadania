#ifndef TELNET_H
#define TELNET_H

#include <string>
#include <sstream>
#include <memory>
#include "Server.h"

class TelnetServer {
public:
    using PortType = Server::PortType;

    explicit TelnetServer(TelnetServer::PortType port) : server_(std::make_unique<Server>(port)) {}

    void handleTelnetConnection();

private:
    std::string telnetSettings();
    void showMainMenu();

    const char IAC = '\377';
    const char WILL = '\373';
    const char ECHO = '\1';
    const char SUPPRESS_GO_AHEAD = '\3';
    const char WONT = '\374';
    const char LINEMODE = '\42';

    using ServerPtr = std::unique_ptr<Server>;
    ServerPtr server_;
};


#endif //TELNET_H
