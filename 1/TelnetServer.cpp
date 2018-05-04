#include <iostream>
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
    sendWill(enumValue(TelnetSettings::ECHO));
    server_->readClient(3);
    sendWill(enumValue(TelnetSettings::SUPPRESS_GO_AHEAD));
    server_->readClient(3);
    sendDo(enumValue(TelnetSettings::NAWS));
    std::string client_message = server_->readClient(3);

    auto csv = std::bind(enumValue<TelnetSettings>, std::placeholders::_1);
    using ts = TelnetSettings;
    std::stringstream naws_accept_ss;
    naws_accept_ss << csv(ts::IAC) << csv(ts::WILL) << csv(ts::NAWS);

    if (client_message == naws_accept_ss.str()) {
        std::string dimensions = server_->readClient(9);
        terminal_width_ = (unsigned char) dimensions[4];
        terminal_height_ = (unsigned char) dimensions[6];
        menu_width_ = terminal_width_ * 3 / 4;
        std::cout << "h: " << terminal_height_ << ", w: " << terminal_width_ << '\n';
    } else {
        std::cout << "NAWS NOT ACCEPTED\n";
    }

    sendWont(enumValue(TelnetSettings::LINEMODE));
}

void TelnetServer::clearScreen() {
    static const std::string CLEAR_SCREEN_CODE = "\x1B[2J";
    server_->writeToClient(CLEAR_SCREEN_CODE);
}

void TelnetServer::setCursorPosition(int line, int column) {
    static const std::string SET_CURSOR_CODE_PREF = "\x1B[";
    static const std::string SET_CURSOR_CODE_SUFF = "H";

    std::string to_write = SET_CURSOR_CODE_PREF;
    to_write += std::to_string(line);
    to_write += ";";
    to_write += std::to_string(column);
    to_write += SET_CURSOR_CODE_SUFF;

    server_->writeToClient(to_write);
}

std::string TelnetServer::changeForegroundColorSetting(ForegroundColor fc) {
    static const std::string SET_FOREGROUND_COLOR_PREF = "\x1B[";
    static const std::string SET_FOREGROUND_COLOR_SUFF = "m";

    std::stringstream to_write;

    to_write << SET_FOREGROUND_COLOR_PREF << std::to_string(enumValue(fc)) << SET_FOREGROUND_COLOR_SUFF;

    return to_write.str();
}

std::string TelnetServer::changeBackgroundColorSetting(BackgroundColor bc) {
    static const std::string SET_BACKGROUND_COLOR_PREF = "\x1B[";
    static const std::string SET_BACKGROUND_COLOR_SUFF = "m";

    std::stringstream to_write;

    to_write << SET_BACKGROUND_COLOR_PREF << std::to_string(enumValue(bc)) << SET_BACKGROUND_COLOR_SUFF;

    return to_write.str();
}

std::string TelnetServer::changeBrightDisplaySetting() {
    static const std::string RESET_DISPLAY_ATTR = "\x1B[1m";
    return RESET_DISPLAY_ATTR;
}

std::string TelnetServer::resetColorSetting() {
    static const std::string RESET_DISPLAY_ATTR = "\x1B[0m";
    return RESET_DISPLAY_ATTR;
}

TelnetServer::Key TelnetServer::keyDownLoop() {
    const static char NULL_CHAR = '\0';
    const static char CR = '\15';
    const static char LF = '\12';

    const static char ARROW_UP_SUFF_CHAR = 'A';
    const static char ARROW_DOWN_SUFF_CHAR = 'B';

    while (true) {
        char c = server_->readCharacter();
        if (c == '\x1B') {
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
    showMainMenu(1);

    while (true) {
        Key keyDown = keyDownLoop();
        if (keyDown == Key::DOWN) {
            return &TelnetServer::mainMenuB;
        }
        else if (keyDown == Key::ENTER) {
            chosen_option_ = "A";
            showMainMenu(1);
        }
    }
}

TelnetServer::MenuState_ TelnetServer::mainMenuB() {
    showMainMenu(2);

    while (true) {
        Key keyDown = keyDownLoop();
        if (keyDown == Key::DOWN) {
            return &TelnetServer::mainMenuEnd;
        }
        else if (keyDown == Key::UP) {
            return &TelnetServer::mainMenuA;
        }
        else if (keyDown == Key::ENTER) {
            chosen_option_ = "";
            return &TelnetServer::bMenuB1;
        }
    }
}

TelnetServer::MenuState_ TelnetServer::mainMenuEnd() {
    showMainMenu(3);

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
    server_->writeToClient('\n');
    return &TelnetServer::mainMenuSendEnd;
}

TelnetServer::MenuState_ TelnetServer::bMenuB1() {
    showBMenu(1);

    while (true) {
        Key keyDown = keyDownLoop();
        if (keyDown == Key::DOWN) {
            return &TelnetServer::bMenuB2;
        }
        else if (keyDown == Key::ENTER) {
            chosen_option_ = "B1";
            showBMenu(1);
        }
    }
}

TelnetServer::MenuState_ TelnetServer::bMenuB2() {
    showBMenu(2);

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
            showBMenu(2);
        }
    }
}

TelnetServer::MenuState_ TelnetServer::bMenuReturn() {
    showBMenu(3);

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

void TelnetServer::showMenu(int option_number, const std::vector<std::string> &menu_options) {
    using std::string;

    const int MENU_HEIGHT = 7; // 1 (border) + 3 (options) + 1 (border) + 1 (chosen) + 1 (border)

    //clearScreen();
    setCursorPosition(1, 0);
    string blue_background = changeBackgroundColorSetting(BackgroundColor::BLUE);
    string magenta_background = changeBackgroundColorSetting(BackgroundColor::MAGENTA);
    string white_foreground = changeForegroundColorSetting(ForegroundColor::WHITE);

    unsigned int line = 1;
    setCursorPosition(line, 0);

    auto sign_count = (terminal_width_ - menu_width_) / 2 - 1;
    string background_signs = string(sign_count, ' ');

    for (size_t i = 0; i < (terminal_height_ - MENU_HEIGHT) / 2; ++i) {
        server_->writeToClient(blue_background + string(terminal_width_, ' '));
        ++line;
        setCursorPosition(line, 0);
    }

    /* Upper border */
    string to_write;
    to_write = blue_background + background_signs;
    to_write += magenta_background;
    to_write += white_foreground;
    to_write += "\x1B(0\x6c";
    to_write += string(menu_width_, '\x71');
    to_write += "\x6b\x1B(B";
    to_write += resetColorSetting();
    to_write += blue_background + background_signs;
    to_write += '\n';
    server_->writeToClient(to_write);
    ++line;
    setCursorPosition(line, 0);

    first_option_line_ = line;

    /* Options menu */
    for (auto &option : menu_options) {
        to_write = blue_background + background_signs;
        to_write += magenta_background;
        to_write += white_foreground;
        to_write += "\x1B(0\x78\x1B(B";
        to_write += changeBrightDisplaySetting();
        to_write += option;
        to_write += string(menu_width_ - option.size(), ' ');
        to_write += resetColorSetting();
        to_write += magenta_background;
        to_write += "\x1B(0\x78\x1B(B";
        to_write += resetColorSetting();
        to_write += blue_background + background_signs;
        to_write += '\n';
        server_->writeToClient(to_write);
        ++line;
        setCursorPosition(line, 0);
    }

    /* In-between border */
    to_write = blue_background + background_signs;
    to_write += magenta_background;
    to_write += "\x1B(0\x74";
    to_write += string(menu_width_, '\x71');
    to_write += resetColorSetting();
    to_write += magenta_background;
    to_write += "\x75\x1B(B";
    to_write += resetColorSetting();
    to_write += blue_background + background_signs;
    to_write += '\n';
    server_->writeToClient(to_write);
    ++line;
    setCursorPosition(line, 0);

    /* Chosen option */
    to_write = blue_background + background_signs;
    to_write += magenta_background;
    to_write += white_foreground;
    to_write += "\x1B(0\x78\x1B(B";
    to_write += changeBrightDisplaySetting();
    to_write += chosen_option_;
    to_write += string(menu_width_ - chosen_option_.size(), ' ');
    to_write += resetColorSetting();
    to_write += magenta_background;
    to_write += "\x1B(0\x78\x1B(B";
    to_write += resetColorSetting();
    to_write += blue_background + background_signs;
    to_write += '\n';
    server_->writeToClient(to_write);
    ++line;
    setCursorPosition(line, 0);

    /* Bottom border */
    to_write = blue_background + background_signs;
    to_write += magenta_background;
    to_write += "\x1B(0\x6d";
    to_write += string(menu_width_, '\x71');
    to_write += "\x6a\x1B(B";
    to_write += resetColorSetting();
    to_write += blue_background + background_signs;
    to_write += '\n';
    server_->writeToClient(to_write);
    ++line;
    setCursorPosition(line, 0);

    for (size_t i = line; i < terminal_height_; ++i) {
        server_->writeToClient(string(terminal_width_, ' '));
        server_->writeToClient('\n');
        ++line;
        setCursorPosition(line, 0);
    }
    server_->writeToClient(string(terminal_width_, ' '));

    setCursorPosition(first_option_line_ + option_number - 1, (int) sign_count + 2);
}

void TelnetServer::showMainMenu(int option_number) {
    static const std::vector<std::string> main_menu_options = { "Opcja A", "Opcja B", "Koniec" };
    showMenu(option_number, main_menu_options);
}

void TelnetServer::showBMenu(int option_number) {
    static const std::vector<std::string> b_menu_options = { "Opcja B1", "Opcja B2", "Wstecz" };
    showMenu(option_number, b_menu_options);
}

void TelnetServer::sendIac(TelnetServer::TelnetSettings ts, char option) {
    std::stringstream msgOption;
    msgOption << enumValue(TelnetSettings::IAC) << enumValue(ts) << option;
    server_->writeToClient(msgOption.str());
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
constexpr typename std::underlying_type_t<Enum> TelnetServer::enumValue(Enum t) {
    return static_cast<typename std::underlying_type<Enum>::type>(t);
}

#pragma clang diagnostic pop
