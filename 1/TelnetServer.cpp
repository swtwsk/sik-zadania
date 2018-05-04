#include <iostream>
#include <sstream>
#include "TelnetServer.h"

void TelnetServer::acceptTelnetConnection() {
    while (true) {
        try {
            server_->acceptConnection();
            std::cout << "Connection accepted" << std::endl;
            return;
        }
        catch (ServerClientConnectionException &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}
void TelnetServer::endTelnetConnection() {
    try {
        server_->endConnection();
        std::cout << "Connection closed" << std::endl;
    }
    catch (ServerClientConnectionException &e) {
        std::cerr << e.what() << std::endl;
    }
}

void TelnetServer::clearScreen() {
    static const std::string GO_UP_SCREEN_CODE = "\x1B[H\x1B[J";
    static const std::string CLEAR_SCREEN_CODE = "\x1B[2J";
    sendString(GO_UP_SCREEN_CODE);
    sendString(CLEAR_SCREEN_CODE, BackgroundColor::BLACK);
}

void TelnetServer::setCursorPosition(int line, int column) {
    static const std::string SET_CURSOR_CODE_PREF = "\x1B[";
    static const std::string SET_CURSOR_CODE_SUFF = "H";

    std::stringstream to_write;
    to_write << SET_CURSOR_CODE_PREF << std::to_string(line) << ";" << std::to_string(column) << SET_CURSOR_CODE_SUFF;

    sendString(to_write.str());
}

std::string TelnetServer::foregroundColorSetting(ForegroundColor fc) {
    static const std::string SET_FOREGROUND_COLOR_PREF = "\x1B[";
    static const std::string SET_FOREGROUND_COLOR_SUFF = "m";

    std::stringstream to_write;

    to_write << SET_FOREGROUND_COLOR_PREF << std::to_string(enumValue(fc)) << SET_FOREGROUND_COLOR_SUFF;

    return to_write.str();
}

std::string TelnetServer::backgroundColorSetting(BackgroundColor bc) {
    static const std::string SET_BACKGROUND_COLOR_PREF = "\x1B[";
    static const std::string SET_BACKGROUND_COLOR_SUFF = "m";

    std::stringstream to_write;

    to_write << SET_BACKGROUND_COLOR_PREF << std::to_string(enumValue(bc)) << SET_BACKGROUND_COLOR_SUFF;

    return to_write.str();
}

std::string TelnetServer::brightDisplaySetting() {
    static const std::string RESET_DISPLAY_ATTR = "\x1B[1m";
    return RESET_DISPLAY_ATTR;
}

TelnetServer::Key TelnetServer::readKeyDown() {
    const static char NULL_CHAR = '\0';
    const static char CR = '\15';
    const static char LF = '\12';

    const static char ARROW_UP_SUFF_CHAR = 'A';
    const static char ARROW_DOWN_SUFF_CHAR = 'B';

    while (true) {
        char c = server_->readCharacter();
        if (c == ESCAPE_CHARACTER) {
            c = server_->readCharacter();
            if (c == '[') {
                c = server_->readCharacter();

                switch (c) {
                    case ARROW_UP_SUFF_CHAR:
                        return Key::UP;

                    case ARROW_DOWN_SUFF_CHAR:
                        return Key::DOWN;

                    default:
                        break;
                }
            }
        }
        else if (c == CR) {
            c = server_->readCharacter();
            if (c == NULL_CHAR || c == LF) {
                return Key::ENTER;
            }
        }
    }
}

std::string TelnetServer::sendIac(TelnetServer::TelnetSettings ts, char option, bool read_response) {
    std::stringstream msgOption;
    msgOption << enumValue(TelnetSettings::IAC) << enumValue(ts) << option;
    server_->writeToClient(msgOption.str());

    return (read_response ? server_->readClient(3) : "");
}

std::string TelnetServer::sendWill(char option, bool read_response) {
    return sendIac(TelnetSettings::WILL, option, read_response);
}

std::string TelnetServer::sendWont(char option, bool read_response) {
    return sendIac(TelnetSettings::WONT, option, read_response);
}

std::string TelnetServer::sendDo(char option, bool read_response) {
    return sendIac(TelnetSettings::DO, option, read_response);
}

std::pair<size_t, size_t> TelnetServer::readNAWS() {
    // NAWS client dimensions information is in format:
    // IAC SB NAWS 0 WIDTH 0 HEIGHT IAC SE
    std::string dimensions = server_->readClient(9);

    size_t width = static_cast<size_t>(static_cast<unsigned char>(dimensions[4]));
    size_t height = static_cast<size_t>(static_cast<unsigned char>(dimensions[6]));

    return std::make_pair(width, height);
}

void TelnetServer::sendString(const std::string &message) {
    server_->writeToClient(message);
}

void TelnetServer::sendString(const std::string &message, BackgroundColor background_color) {
    std::stringstream message_to_write;

    message_to_write << backgroundColorSetting(background_color) << message;

    server_->writeToClient(message_to_write.str());
}

void TelnetServer::sendString(const std::string &message, ForegroundColor foreground_color,
                              BackgroundColor background_color, bool bright_display) {
    std::stringstream message_to_write;

    message_to_write << backgroundColorSetting(background_color) << foregroundColorSetting(foreground_color);
    if (bright_display) {
        message_to_write << brightDisplaySetting();
    }
    message_to_write << message;

    server_->writeToClient(message_to_write.str());
}
