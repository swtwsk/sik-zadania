#ifndef TELNET_H
#define TELNET_H

#include <string>
#include <sstream>
#include <memory>
#include "Server.h"

class TelnetServer {
public:
    using PortType = Server::PortType;

    explicit TelnetServer(TelnetServer::PortType port)
            : server_(std::make_unique<Server>(port)), terminalWidth_(80), terminalHeight_(24) {}

    void handleTelnetConnection();

    void sendWill(char option);
    void sendWont(char option);
    void sendDo(char option);

private:
    void telnetSettings();
    static std::string clearScreen();
    static std::string setCursorPosition(int line, int column);

    void showMainMenu();

    enum class TelnetSettings : char {
        IAC = '\377',
        WILL = '\373',
        WONT = '\374',
        DO = '\375',
        ECHO = '\1',
        SUPPRESS_GO_AHEAD = '\3',
        LINEMODE = '\42',
        NAWS = '\37',
        SE = '\360',
    };
    static constexpr char charSettingValue(TelnetSettings t);
    void sendIac(TelnetSettings ts, char option);

    using ServerPtr = std::unique_ptr<Server>;
    ServerPtr server_;
    int terminalHeight_;
    int terminalWidth_;
};


#endif //TELNET_H
