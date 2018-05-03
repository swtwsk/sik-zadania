#ifndef TELNET_H
#define TELNET_H

#include <string>
#include <sstream>
#include <memory>
#include <utility>
#include <functional>
#include "Server.h"

class TelnetServer {
public:
    using PortType = Server::PortType;

    explicit TelnetServer(TelnetServer::PortType port)
            : server_(std::make_unique<Server>(port)), terminal_width_(80), terminal_height_(24) {}

    void handleTelnetConnection();

    void sendWill(char option);
    void sendWont(char option);
    void sendDo(char option);

private:
    void telnetSettings();
    void clearScreen();
    void setCursorPosition(int line, int column);

    enum class Key {
        UP,
        DOWN,
        ENTER,
    };
    Key keyDownLoop();

    struct MenuState_;
    typedef MenuState_ (TelnetServer::*MenuState)();

    struct MenuState_ {
        MenuState_(MenuState menu_state) : menu_state_(menu_state) {}
        operator MenuState() { return menu_state_; }
        MenuState menu_state_;
    };

    void menuLoop();

    MenuState_ mainMenuA();
    MenuState_ mainMenuB();
    MenuState_ mainMenuEnd();
    MenuState_ mainMenuSendEnd();
    MenuState_ bMenuB1();
    MenuState_ bMenuB2();
    MenuState_ bMenuReturn();

    void showMainMenu(int cursor_line);
    void showBMenu(int cursor_line);

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
    size_t terminal_height_;
    size_t terminal_width_;

    const size_t MENU_WIDTH = 16;
};


#endif //TELNET_H
