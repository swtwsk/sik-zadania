#ifndef TELNET_H
#define TELNET_H

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <utility>
#include "Server.h"

class TelnetServer {
public:
    using PortType = Server::PortType;

    explicit TelnetServer(TelnetServer::PortType port) : server_(std::make_unique<Server>(port)) {}

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

    template <class Enum>
    static constexpr typename std::underlying_type_t<Enum> enumValue(Enum t) {
        return static_cast<typename std::underlying_type<Enum>::type>(t);
    }

    enum class Key {
        UP,
        DOWN,
        ENTER,
    };

    void acceptTelnetConnection();
    void endTelnetConnection();

    std::string sendWill(char option, bool read_response);
    std::string sendWont(char option, bool read_response);
    std::string sendDo(char option, bool read_response);
    std::pair<size_t, size_t> readNAWS();

    void sendString(const std::string &message);
    void sendString(const std::string &message, BackgroundColor background_color);
    void sendString(const std::string &message, ForegroundColor foreground_color,
                    BackgroundColor background_color, bool bright_display);
    void clearScreen();
    void setCursorPosition(int line, int column);

    Key readKeyDown();

    static constexpr char ESCAPE_CHARACTER = '\x1B';

private:
    std::string foregroundColorSetting(ForegroundColor fc);
    std::string backgroundColorSetting(BackgroundColor bc);
    std::string brightDisplaySetting();

    std::string sendIac(int command, char option, bool read_response);

    using ServerPtr = std::unique_ptr<Server>;

    ServerPtr server_{};
};

#endif //TELNET_H
