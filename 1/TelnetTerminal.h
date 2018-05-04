#ifndef TELNET_TERMINAL_H
#define TELNET_TERMINAL_H

#include <memory>
#include "TelnetServer.h"

class TelnetTerminal {
public:
    explicit TelnetTerminal(std::unique_ptr<TelnetServer> telnet_server);

    void handleClient();

private:
    struct MenuState_;
    typedef MenuState_ (TelnetTerminal::*MenuState)();

    struct MenuState_ {
        MenuState_(MenuState menu_state) : menu_state_(menu_state) {}
        operator MenuState() { return menu_state_; }
        MenuState menu_state_;
    };

    void setTelnetSettings();

    void menuLoop();

    MenuState_ mainMenuA();
    MenuState_ mainMenuB();
    MenuState_ mainMenuEnd();
    MenuState_ mainMenuSendEnd();
    MenuState_ bMenuB1();
    MenuState_ bMenuB2();
    MenuState_ bMenuReturn();

    void upperBorder(const std::string &background_signs, TelnetServer::BackgroundColor outer_background,
                     TelnetServer::ForegroundColor inner_foreground, TelnetServer::BackgroundColor inner_background);
    unsigned int optionsMenu(unsigned int line, const std::string &background_signs,
                             const std::vector<std::string> &menu_options,
                             TelnetServer::BackgroundColor outer_background,
                             TelnetServer::ForegroundColor inner_foreground,
                             TelnetServer::BackgroundColor inner_background);
    void inBetweenBorder(const std::string &background_signs, TelnetServer::BackgroundColor outer_background,
                         TelnetServer::ForegroundColor inner_foreground,
                         TelnetServer::BackgroundColor inner_background);
    void chosenOption(const std::string &background_signs, TelnetServer::BackgroundColor outer_background,
                      TelnetServer::ForegroundColor inner_foreground, TelnetServer::BackgroundColor inner_background);
    void bottomBorder(const std::string &background_signs, TelnetServer::BackgroundColor outer_background,
                      TelnetServer::ForegroundColor inner_foreground, TelnetServer::BackgroundColor inner_background);

    void showMenu(int option_number, const std::vector<std::string> &menu_options);
    void showMainMenu(int option_number);
    void showBMenu(int option_number);

    /* Based on VT100 display size */
    /* It seems it is also a standard for all kinds of Linux terminal (especially GNOME) */
    constexpr static size_t DEFAULT_TERMINAL_WIDTH = 80;
    constexpr static size_t DEFAULT_TERMINAL_HEIGHT = 24;
    constexpr static size_t DEFAULT_MENU_WIDTH = DEFAULT_TERMINAL_WIDTH * 3 / 4;
    constexpr static size_t MENU_HEIGHT = 7; // 1 (border) + 3 (options) + 1 (border) + 1 (chosen) + 1 (border)
    constexpr static size_t DEFAULT_FIRST_OPTION_LINE = (DEFAULT_TERMINAL_WIDTH - MENU_HEIGHT) / 2 + 2;

    std::unique_ptr<TelnetServer> telnet_server_;
    size_t terminal_width_;
    size_t terminal_height_;
    size_t menu_width_;
    int first_option_line_;
    std::string chosen_option_;
};

#endif //TELNET_TERMINAL_H
