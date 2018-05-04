#include <iostream>
#include <sstream>
#include <utility>
#include "TelnetTerminal.h"

using Key = TelnetServer::Key;
using std::string;
using std::stringstream;

TelnetTerminal::TelnetTerminal(std::unique_ptr<TelnetServer> telnet_server)
    : telnet_server_(std::move(telnet_server)), terminal_width_(DEFAULT_TERMINAL_WIDTH),
      terminal_height_(DEFAULT_TERMINAL_HEIGHT), menu_width_(DEFAULT_MENU_WIDTH),
      first_option_line_(DEFAULT_FIRST_OPTION_LINE), chosen_option_("") {

    /*option_lines_[&TelnetTerminal::mainMenuA] = 1;
    option_lines_[&TelnetTerminal::mainMenuB] = 2;
    option_lines_[&TelnetTerminal::mainMenuEnd] = 3;
    option_lines_[&TelnetTerminal::bMenuB1] = 1;
    option_lines_[&TelnetTerminal::bMenuB2] = 2;
    option_lines_[&TelnetTerminal::bMenuReturn] = 3;*/
}

void TelnetTerminal::handleClient() {
    while (true) {
        telnet_server_->acceptTelnetConnection();

        setTelnetSettings();
        menuLoop();

        telnet_server_->endTelnetConnection();
    }
}

void TelnetTerminal::setTelnetSettings() {
    using TelnetSettings = TelnetServer::TelnetSettings;
    auto enumVal = std::bind(TelnetServer::enumValue<TelnetSettings>, std::placeholders::_1);

    telnet_server_->sendWill(enumVal(TelnetSettings::ECHO), true);
    telnet_server_->sendWill(enumVal(TelnetSettings::SUPPRESS_GO_AHEAD), true);

    // Negotiate NAWS with client
    string client_message = telnet_server_->sendDo(enumVal(TelnetSettings::NAWS), true);
    stringstream naws_accept_ss;
    naws_accept_ss << enumVal(TelnetSettings::IAC) << enumVal(TelnetSettings::WILL) << enumVal(TelnetSettings::NAWS);

    if (client_message == naws_accept_ss.str()) {
        auto dimensions = telnet_server_->readNAWS();
        terminal_width_ = dimensions.first;
        terminal_height_ = dimensions.second;
        menu_width_ = terminal_width_ * 3 / 4;
    }

    telnet_server_->sendWont(enumVal(TelnetSettings::LINEMODE), false);
}

void TelnetTerminal::menuLoop() {
    try {
        MenuState active_menu = mainMenuA();

        while (true) {
            active_menu = ((*this).*active_menu)();
            if (active_menu == &TelnetTerminal::mainMenuSendEnd) {
                return;
            }
        }
    }
    catch (ServerClientDisconnectedException & e) {
        return;
    }
}

TelnetTerminal::MenuState_ TelnetTerminal::mainMenuA() {
    // TODO: Check if it does work!
    //showMainMenu(option_lines_[&TelnetTerminal::mainMenuA]);
    const static char MAIN_MENU_A_OPTION = 'A';
    const static int MAIN_MENU_A_LINE = 1;
    showMainMenu(MAIN_MENU_A_LINE);

    while (true) {
        Key keyDown = telnet_server_->readKeyDown();

        switch (keyDown) {
            case Key::DOWN:
                return &TelnetTerminal::mainMenuB;

            case Key::ENTER:
                chosen_option_ = MAIN_MENU_A_OPTION;
                showMainMenu(1);

            default:
                break;
        }
    }
}

TelnetTerminal::MenuState_ TelnetTerminal::mainMenuB() {
    const static int MAIN_MENU_B_LINE = 2;
    showMainMenu(MAIN_MENU_B_LINE);

    while (true) {
        Key keyDown = telnet_server_->readKeyDown();

        switch (keyDown) {
            case Key::DOWN:
                return &TelnetTerminal::mainMenuEnd;

            case Key::UP:
                return &TelnetTerminal::mainMenuA;

            case Key::ENTER:
                chosen_option_ = "";
                return &TelnetTerminal::bMenuB1;
        }
    }
}

TelnetTerminal::MenuState_ TelnetTerminal::mainMenuEnd() {
    const static int MAIN_MENU_END_LINE = 3;
    showMainMenu(MAIN_MENU_END_LINE);

    while (true) {
        Key keyDown = telnet_server_->readKeyDown();

        switch (keyDown) {
            case Key::UP:
                return &TelnetTerminal::mainMenuB;

            case Key::ENTER:
                telnet_server_->sendString("\n");
                telnet_server_->clearScreen();
                return &TelnetTerminal::mainMenuSendEnd;

            default:
                break;
        }
    }
}

TelnetTerminal::MenuState_ TelnetTerminal::mainMenuSendEnd() {
    return &TelnetTerminal::mainMenuSendEnd;
}

TelnetTerminal::MenuState_ TelnetTerminal::bMenuB1() {
    const static string B_MENU_B1_OPTION = "B1";
    const static int B_MENU_B1_LINE = 1;
    showBMenu(B_MENU_B1_LINE);

    while (true) {
        Key keyDown = telnet_server_->readKeyDown();

        switch (keyDown) {
            case Key::DOWN:
                return &TelnetTerminal::bMenuB2;

            case Key::ENTER:
                chosen_option_ = B_MENU_B1_OPTION;
                showBMenu(B_MENU_B1_LINE);

            default:
                break;
        }
    }
}

TelnetTerminal::MenuState_ TelnetTerminal::bMenuB2() {
    const static string B_MENU_B2_OPTION = "B2";
    const static int B_MENU_B2_LINE = 2;
    showBMenu(B_MENU_B2_LINE);

    while (true) {
        Key keyDown = telnet_server_->readKeyDown();

        switch(keyDown) {
            case Key::DOWN:
                return &TelnetTerminal::bMenuReturn;

            case Key::UP:
                return &TelnetTerminal::bMenuB1;

            case Key::ENTER:
                chosen_option_ = B_MENU_B2_OPTION;
                showBMenu(B_MENU_B2_LINE);
        }
    }
}

TelnetTerminal::MenuState_ TelnetTerminal::bMenuReturn() {
    const static int B_MENU_RETURN_LINE = 3;
    showBMenu(B_MENU_RETURN_LINE);

    while (true) {
        Key keyDown = telnet_server_->readKeyDown();

        switch(keyDown) {
            case Key::UP:
                return &TelnetTerminal::bMenuB2;

            case Key::ENTER:
                chosen_option_ = "";
                return &TelnetTerminal::mainMenuB;

            default:
                break;
        }
    }
}

const string LOWER_CASE_ASCII = "\x1B(0";
const string SWITCH_BACK_ASCII = "\x1B(B";
const char HORIZONTAL_BORDER = '\x71';
const char VERTICAL_BORDER = '\x78';
const string VERTICAL_BORDER_STR = LOWER_CASE_ASCII + VERTICAL_BORDER + SWITCH_BACK_ASCII;

void TelnetTerminal::upperBorder(const std::string &background_signs,
                                 TelnetServer::BackgroundColor outer_background,
                                 TelnetServer::ForegroundColor inner_foreground,
                                 TelnetServer::BackgroundColor inner_background) {

    const char LEFT_UPPER_BORDER_PART = '\x6C';
    const char RIGHT_UPPER_BORDER_PART = '\x6B';

    stringstream ss;
    ss << LOWER_CASE_ASCII << LEFT_UPPER_BORDER_PART << string(menu_width_, HORIZONTAL_BORDER)
       << RIGHT_UPPER_BORDER_PART << SWITCH_BACK_ASCII;

    telnet_server_->sendString(background_signs, outer_background);
    telnet_server_->sendString(ss.str(), inner_foreground, inner_background, true);
    telnet_server_->sendString(background_signs, outer_background);
    telnet_server_->sendString("\n");
}

unsigned int TelnetTerminal::optionsMenu(unsigned int line, const std::string &background_signs,
                                         const std::vector<std::string> &menu_options,
                                         TelnetServer::BackgroundColor outer_background,
                                         TelnetServer::ForegroundColor inner_foreground,
                                         TelnetServer::BackgroundColor inner_background) {

    stringstream ss;

    for (auto &option : menu_options) {
        telnet_server_->sendString(background_signs, outer_background);
        ss.str("");
        ss << VERTICAL_BORDER_STR << option << string(menu_width_ - option.size(), ' ') << VERTICAL_BORDER_STR;
        telnet_server_->sendString(ss.str(), inner_foreground, inner_background, true);
        telnet_server_->sendString(background_signs, outer_background);
        telnet_server_->sendString("\n");
        ++line;
        telnet_server_->setCursorPosition(line, 0);
    }

    return line;
}

void TelnetTerminal::inBetweenBorder(const std::string &background_signs,
                                     TelnetServer::BackgroundColor outer_background,
                                     TelnetServer::ForegroundColor inner_foreground,
                                     TelnetServer::BackgroundColor inner_background) {

    const char LEFT_INBETWEEN_BORDER = '\x74';
    const char RIGHT_INBETWEEN_BORDER = '\x75';

    const string left_vertical_border = LOWER_CASE_ASCII + LEFT_INBETWEEN_BORDER;
    const string right_vertical_border = RIGHT_INBETWEEN_BORDER + SWITCH_BACK_ASCII;

    telnet_server_->sendString(background_signs, outer_background);
    telnet_server_->sendString(left_vertical_border + string(menu_width_, HORIZONTAL_BORDER) + right_vertical_border,
                               inner_foreground, inner_background, true);
    telnet_server_->sendString(background_signs, outer_background);
    telnet_server_->sendString("\n");
}

void TelnetTerminal::chosenOption(const std::string &background_signs,
                                  TelnetServer::BackgroundColor outer_background,
                                  TelnetServer::ForegroundColor inner_foreground,
                                  TelnetServer::BackgroundColor inner_background) {

    telnet_server_->sendString(background_signs, outer_background);

    stringstream ss;
    ss << VERTICAL_BORDER_STR << chosen_option_ << string(menu_width_ - chosen_option_.size(), ' ')
       << VERTICAL_BORDER_STR;
    telnet_server_->sendString(ss.str(), inner_foreground, inner_background, true);

    telnet_server_->sendString(background_signs, outer_background);
    telnet_server_->sendString("\n");
}

void TelnetTerminal::bottomBorder(const std::string &background_signs,
                                  TelnetServer::BackgroundColor outer_background,
                                  TelnetServer::ForegroundColor inner_foreground,
                                  TelnetServer::BackgroundColor inner_background) {

    const char LEFT_BOTTOM_BORDER_PART = '\x6D';
    const char RIGHT_BOTTOM_BORDER_PART = '\x6A';

    stringstream ss;
    ss << LOWER_CASE_ASCII << LEFT_BOTTOM_BORDER_PART << string(menu_width_, HORIZONTAL_BORDER)
       << RIGHT_BOTTOM_BORDER_PART << SWITCH_BACK_ASCII;

    telnet_server_->sendString(background_signs, outer_background);
    telnet_server_->sendString(ss.str(), inner_foreground, inner_background, false);
    telnet_server_->sendString(background_signs, outer_background);
    telnet_server_->sendString("\n");
}

void TelnetTerminal::showMenu(int option_number, const std::vector<string> &menu_options) {
    using Background = TelnetServer::BackgroundColor;
    using Foreground = TelnetServer::ForegroundColor;
    const static char SIGN = ' ';

    unsigned int line = 1;
    telnet_server_->setCursorPosition(line, 0);

    auto sign_count = (terminal_width_ - menu_width_) / 2 - 1;
    string background_signs = string(sign_count, SIGN);

    auto outer_background = Background::BLUE;
    auto inner_foreground = Foreground::WHITE;
    auto inner_background = Background::MAGENTA;

    for (size_t i = 0; i < (terminal_height_ - MENU_HEIGHT) / 2; ++i) {
        telnet_server_->sendString(string(terminal_width_, SIGN) + "\n", outer_background);
        ++line;
        telnet_server_->setCursorPosition(line, 0);
    }

    upperBorder(background_signs, outer_background, inner_foreground, inner_background);
    ++line;
    telnet_server_->setCursorPosition(line, 0);

    first_option_line_ = line;

    line = optionsMenu(line, background_signs, menu_options, outer_background, inner_foreground, inner_background);

    inBetweenBorder(background_signs, outer_background, inner_foreground, inner_background);
    ++line;
    telnet_server_->setCursorPosition(line, 0);

    chosenOption(background_signs, outer_background, inner_foreground, inner_background);
    ++line;
    telnet_server_->setCursorPosition(line, 0);

    bottomBorder(background_signs, outer_background, inner_foreground, inner_background);
    ++line;
    telnet_server_->setCursorPosition(line, 0);

    for (size_t i = line; i < terminal_height_; ++i) {
        telnet_server_->sendString(string(terminal_width_, SIGN) + "\n", outer_background);
        ++line;
        telnet_server_->setCursorPosition(line, 0);
    }

    telnet_server_->sendString(string(terminal_width_, ' '));
    telnet_server_->setCursorPosition(first_option_line_ + option_number - 1, (int) sign_count + 2);
}

void TelnetTerminal::showMainMenu(int option_number) {
    static const std::vector<std::string> main_menu_options = { "Opcja A", "Opcja B", "Koniec" };
    showMenu(option_number, main_menu_options);
}

void TelnetTerminal::showBMenu(int option_number) {
    static const std::vector<std::string> b_menu_options = { "Opcja B1", "Opcja B2", "Wstecz" };
    showMenu(option_number, b_menu_options);
}