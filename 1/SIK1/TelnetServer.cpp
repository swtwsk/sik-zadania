#include <iostream>
#include <functional>
#include "TelnetServer.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void TelnetServer::handleTelnetConnection() {
    while(true) {
        try {
            server_->acceptConnection();
            std::cout << "Connection accepted" << std::endl;

            telnetSettings();
            menuLoop();

            server_->endConnection();
            std::cout << "Connection closed" << std::endl;
        }
        catch (ServerClientConnectionException &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

void TelnetServer::clearScreen() {
    static const std::string CLEAR_SCREEN_CODE = "\e[2J";
    server_->writeClient(CLEAR_SCREEN_CODE);
}

void TelnetServer::setCursorPosition(int line, int column) {
    static const std::string SET_CURSOR_CODE_PREF = "\e[";
    static const std::string SET_CURSOR_CODE_SUFF = "H";

    std::string to_write = SET_CURSOR_CODE_PREF;
    to_write += std::to_string(line);
    to_write += ";";
    to_write += std::to_string(column);
    to_write += SET_CURSOR_CODE_SUFF;

    server_->writeClient(to_write);
}

TelnetServer::Key TelnetServer::keyDownLoop() {
    while (true) {
        char c = server_->readCharacter();
        if (c == '\e') {
            c = server_->readCharacter();
            if (c == '[') {
                c = server_->readCharacter();
                if (c == 'A') {
                    return Key::UP;
                }
                else if (c == 'B') {
                    return Key::DOWN;
                }
            }
        }
        else if (c == '\15') {
            c = server_->readCharacter();
            if (c == '\0' || c == '\12') {
                return Key::ENTER;
            }
        }
    }
}

void TelnetServer::menuLoop() {
    MenuState active_menu = mainMenuA();
    while (true) {
        active_menu = ((*this).*active_menu)();
        if (active_menu == &TelnetServer::mainMenuSendEnd) {
            return;
        }
    }
}

TelnetServer::MenuState_ TelnetServer::mainMenuA() {
    showMainMenu(2);

    while (true) {
        Key keyDown = keyDownLoop();
        if (keyDown == Key::DOWN) {
            return &TelnetServer::mainMenuB;
        }
        else if (keyDown == Key::ENTER) {
            std::cout << "A" << std::endl;
        }
    }
}

TelnetServer::MenuState_ TelnetServer::mainMenuB() {
    showMainMenu(3);
    while (true) {
        Key keyDown = keyDownLoop();
        if (keyDown == Key::DOWN) {
            return &TelnetServer::mainMenuEnd;
        }
        else if (keyDown == Key::UP) {
            return &TelnetServer::mainMenuA;
        }
        else if (keyDown == Key::ENTER) {
            return &TelnetServer::bMenuB1;
        }
    }
}

TelnetServer::MenuState_ TelnetServer::mainMenuEnd() {
    showMainMenu(4);
    while (true) {
        Key keyDown = keyDownLoop();
        if (keyDown == Key::UP) {
            return &TelnetServer::mainMenuB;
        }
        else if (keyDown == Key::ENTER) {
            return &TelnetServer::mainMenuSendEnd;
        }
    }
}

TelnetServer::MenuState_ TelnetServer::mainMenuSendEnd() {
    return &TelnetServer::mainMenuSendEnd;
}

TelnetServer::MenuState_ TelnetServer::bMenuB1() {
    showBMenu(2);
    while (true) {
        Key keyDown = keyDownLoop();
        if (keyDown == Key::DOWN) {
            return &TelnetServer::bMenuB2;
        }
        else if (keyDown == Key::ENTER) {
            std::cout << "B1" << std::endl;
        }
    }
}

TelnetServer::MenuState_ TelnetServer::bMenuB2() {
    showBMenu(3);
    while (true) {
        Key keyDown = keyDownLoop();
        if (keyDown == Key::DOWN) {
            return &TelnetServer::bMenuReturn;
        }
        else if (keyDown == Key::UP) {
            return &TelnetServer::bMenuB1;
        }
        else if (keyDown == Key::ENTER) {
            std::cout << "B2" << std::endl;
        }
    }
}

TelnetServer::MenuState_ TelnetServer::bMenuReturn() {
    showBMenu(4);
    while (true) {
        Key keyDown = keyDownLoop();
        if (keyDown == Key::UP) {
            return &TelnetServer::bMenuB2;
        }
        else if (keyDown == Key::ENTER) {
            return &TelnetServer::mainMenuB;
        }
    }
}

void TelnetServer::showMainMenu(int cursor_line) {
    using std::string;

    clearScreen();

    auto line = 1;
    setCursorPosition(line, 0);

    server_->writeClient(string(terminal_width_,' '));
    server_->writeClient('\n');
    ++line;
    setCursorPosition(line, 0);

    auto sign_count = (terminal_width_ - MENU_WIDTH) / 2;
    string background_signs = string(sign_count, ' ');
    string main_menu_options[] = { "Opcja A", "Opcja B", "Koniec" };

    string to_write;
    for (auto &option : main_menu_options) {
        to_write = background_signs;
        to_write += option;
        to_write += string(MENU_WIDTH - option.size(), ' ');
        to_write += background_signs;
        to_write += '\n';
        server_->writeClient(to_write);
        ++line;
        setCursorPosition(line, 0);
    }

    to_write = background_signs;
    to_write += string(MENU_WIDTH, ' ');
    to_write += background_signs;
    server_->writeClient(to_write);
    server_->writeClient('\n');
    ++line;
    setCursorPosition(line, 0);

    for (auto i = line; i < terminal_height_; ++i) {
        server_->writeClient(string(terminal_width_, ' '));
        server_->writeClient('\n');
        ++line;
        setCursorPosition(line, 0);
    }
    server_->writeClient(string(terminal_width_, ' '));

    setCursorPosition(cursor_line, 2);
}

void TelnetServer::showBMenu(int cursor_line) {
    using std::string;

    clearScreen();

    auto line = 1;
    setCursorPosition(line, 0);

    server_->writeClient(string(terminal_width_,' '));
    server_->writeClient('\n');
    ++line;
    setCursorPosition(line, 0);

    auto sign_count = (terminal_width_ - MENU_WIDTH) / 2;
    string background_signs = string(sign_count, ' ');
    string main_menu_options[] = { "Opcja B1", "Opcja B2", "Wstecz" };

    string to_write;
    for (auto &option : main_menu_options) {
        to_write = background_signs;
        to_write += option;
        to_write += string(MENU_WIDTH - option.size(), ' ');
        to_write += background_signs;
        to_write += '\n';
        server_->writeClient(to_write);
        ++line;
        setCursorPosition(line, 0);
    }

    to_write = background_signs;
    to_write += string(MENU_WIDTH, ' ');
    to_write += background_signs;
    server_->writeClient(to_write);
    server_->writeClient('\n');
    ++line;
    setCursorPosition(line, 0);

    for (auto i = line; i < terminal_height_; ++i) {
        server_->writeClient(string(terminal_width_, ' '));
        server_->writeClient('\n');
        ++line;
        setCursorPosition(line, 0);
    }
    server_->writeClient(string(terminal_width_, ' '));

    setCursorPosition(cursor_line, 2);
}

void TelnetServer::telnetSettings() {
    sendWill(charSettingValue(TelnetSettings::ECHO));
    server_->readClient(3);
    sendWill(charSettingValue(TelnetSettings::SUPPRESS_GO_AHEAD));
    server_->readClient(3);
    sendDo(charSettingValue(TelnetSettings::NAWS));
    std::string client_message = server_->readClient(3);

    auto csv = std::bind(charSettingValue, std::placeholders::_1);
    using ts = TelnetSettings;
    std::stringstream naws_accept_ss;
    naws_accept_ss << csv(ts::IAC) << csv(ts::WILL) << csv(ts::NAWS);

    if (client_message == naws_accept_ss.str()) {
        //std::cout << "NAWS ACCEPTED\n";
        std::string dimensions = server_->readClient(9);
        terminal_width_ = (unsigned char) dimensions[4];
        terminal_height_ = (unsigned char) dimensions[6];
        std::cout << "h: " << terminal_height_ << ", w: " << terminal_width_ << '\n';
    } else {
        std::cout << "NAWS NOT ACCEPTED\n";
    }

    sendWont(charSettingValue(TelnetSettings::LINEMODE));
}

void TelnetServer::sendIac(TelnetServer::TelnetSettings ts, char option) {
    std::stringstream msgOption;
    msgOption << charSettingValue(TelnetSettings::IAC) << charSettingValue(ts) << option;
    server_->writeClient(msgOption.str());
}

void TelnetServer::sendWill(char option) {
    sendIac(TelnetSettings::WILL, option);
}

void TelnetServer::sendWont(char option) {
    sendIac(TelnetSettings::WONT, option);
}

void TelnetServer::sendDo(char option) {
    sendIac(TelnetSettings::DO, option);
}

constexpr char TelnetServer::charSettingValue(TelnetServer::TelnetSettings t) {
    return static_cast<char>(t);
}

#pragma clang diagnostic pop
