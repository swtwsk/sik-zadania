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
        std::string dimensions = server_->readClient(9);
        terminal_width_ = (unsigned char) dimensions[4];
        terminal_height_ = (unsigned char) dimensions[6];
        std::cout << "h: " << terminal_height_ << ", w: " << terminal_width_ << '\n';
    } else {
        std::cout << "NAWS NOT ACCEPTED\n";
    }

    sendWont(charSettingValue(TelnetSettings::LINEMODE));
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

void TelnetServer::setForegroundColor(ForegroundColor &fc) {
    static const std::string SET_FOREGROUND_COLOR_PREF = "\e[";
    static const std::string SET_FOREGROUND_COLOR_SUFF = "m";

    std::string to_write = SET_FOREGROUND_COLOR_PREF;
    to_write += charSettingValue(fc);
    to_write += SET_FOREGROUND_COLOR_SUFF;

    server_->writeClient(to_write);
}

void TelnetServer::setBackgroundColor(BackgroundColor &bc) {
    static const std::string SET_BACKGROUND_COLOR_PREF = "\e[";
    static const std::string SET_BACKGROUND_COLOR_SUFF = "m";

    std::string to_write = SET_BACKGROUND_COLOR_PREF;
    to_write += charSettingValue(bc);
    to_write += SET_BACKGROUND_COLOR_SUFF;

    server_->writeClient(to_write);
}

TelnetServer::Key TelnetServer::keyDownLoop() {
    const static char NULL_CHAR = '\0';
    const static char CR = '\15';
    const static char LF = '\12';

    const static char ARROW_UP_SUFF_CHAR = 'A';
    const static char ARROW_DOWN_SUFF_CHAR = 'B';

    while (true) {
        char c = server_->readCharacter();
        if (c == '\e') {
            c = server_->readCharacter();
            if (c == '[') {
                c = server_->readCharacter();
                if (c == ARROW_UP_SUFF_CHAR) {
                    return Key::UP;
                }
                else if (c == ARROW_DOWN_SUFF_CHAR) {
                    return Key::DOWN;
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

void TelnetServer::menuLoop() {
    MenuState active_menu = mainMenuA();
    while (true) {
        try {
            active_menu = ((*this).*active_menu)();
            if (active_menu == &TelnetServer::mainMenuSendEnd) {
                return;
            }
        }
        catch (ServerClientDisconnectedException &e) {
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
            chosen_option_ = "A";
            showMainMenu(2);
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
    server_->writeClient('\n');
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
            chosen_option_ = "B1";
            showBMenu(2);
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
            chosen_option_ = "B2";
            showBMenu(3);
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
            chosen_option_ = "";
            return &TelnetServer::mainMenuB;
        }
    }
}

void TelnetServer::showMenu(int cursor_line, const std::vector<std::string> &menu_options) {
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

    string to_write;
    for (auto &option : menu_options) {
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
    to_write += string(MENU_WIDTH, '');
    to_write += background_signs;

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

    setCursorPosition(cursor_line, (int) sign_count + 1);
}

void TelnetServer::showMainMenu(int cursor_line) {
    static const std::vector<std::string> main_menu_options = { "Opcja A", "Opcja B", "Koniec" };
    showMenu(cursor_line, main_menu_options);
}

void TelnetServer::showBMenu(int cursor_line) {
    static const std::vector<std::string> b_menu_options = { "Opcja B1", "Opcja B2", "Wstecz" };
    showMenu(cursor_line, b_menu_options);
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

template<class Enum>
constexpr typename std::underlying_type_t<Enum> TelnetServer::charSettingValue(Enum t) {
    return static_cast<typename std::underlying_type<Enum>::type>(t);
}

#pragma clang diagnostic pop
