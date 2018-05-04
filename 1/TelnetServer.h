#ifndef TELNET_H
#define TELNET_H

#include <string>
#include <sstream>
#include <memory>
#include <functional>
#include <vector>
#include "Server.h"

enum class ForegroundColor : int {
    BLACK = 30,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37,
};

enum class BackgroundColor : int {
    BLACK = 40,
    RED = 41,
    GREEN = 42,
    YELLOW = 43,
    BLUE = 44,
    MAGENTA = 45,
    CYAN = 46,
    WHITE = 47,
};

class TelnetServer {
public:
    using PortType = Server::PortType;

    explicit TelnetServer(TelnetServer::PortType port)
            : server_(std::make_unique<Server>(port)), terminal_width_(80), terminal_height_(24),
              menu_width_(60), first_option_line_((80 - 7) / 2 + 2), chosen_option_("") {}

    void handleTelnetConnection();

    void sendWill(char option);
    void sendWont(char option);
    void sendDo(char option);

private:
    void telnetSettings();

    void clearScreen();
    void setCursorPosition(int line, int column);
    std::string changeForegroundColorSetting(ForegroundColor fc);
    std::string changeBackgroundColorSetting(BackgroundColor bc);
    std::string changeBrightDisplaySetting();
    std::string resetColorSetting();

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

    void showMenu(int option_number, const std::vector<std::string> &menu_options);
    void showMainMenu(int option_number);
    void showBMenu(int option_number);

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

    template <class Enum>
    static constexpr typename std::underlying_type_t<Enum> enumValue(Enum t);

    void sendIac(TelnetSettings ts, char option);

    using ServerPtr = std::unique_ptr<Server>;

    ServerPtr server_;
    size_t terminal_width_;
    size_t terminal_height_;
    size_t menu_width_;
    int first_option_line_;
    std::string chosen_option_;
};

#endif //TELNET_H
