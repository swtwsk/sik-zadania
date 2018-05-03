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
            showMainMenu();
            //server_->readClient();

           /* char readChar;
            bool clientEndConnection = false;
            do {
                switch (readChar = server_->readCharacter()) {
                    case "\e["
                }
            } while (clientEndConnection);*/
            server_->endConnection();
            std::cout << "Connection closed" << std::endl;
        }
        catch (ServerClientConnectionException &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

std::string TelnetServer::clearScreen() {
    static const std::string CLEAR_SCREEN_CODE = "\e[2J";
    return CLEAR_SCREEN_CODE;
}

std::string TelnetServer::setCursorPosition(int line, int column) {
    static const std::string SET_CURSOR_CODE_PREF = "\e[";
    static const std::string SET_CURSOR_CODE_SUFF = "H";
    return SET_CURSOR_CODE_PREF + std::to_string(line) + ";" + std::to_string(column) + SET_CURSOR_CODE_SUFF;
}

void TelnetServer::showMainMenu() {
    /*std::stringstream ss;
    ss << '\n';
    ss << "======================================================\n";
    ss << '\n';
    ss << "  Welcome to Telnet Server  ";
    ss << "\n\n";
    ss << "======================================================\n";
    server_->writeClient(ss.str());
    sleep(5);*/
    //server_->writeClient("\e[2J");
    server_->writeClient("SEEMS WORKING XD");
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
        terminalWidth_ = (unsigned char) dimensions[4];
        terminalHeight_ = (unsigned char) dimensions[6];
        std::cout << "h: " << terminalHeight_ << ", w: " << terminalWidth_ << '\n';
    } else {
        std::cout << "NAWS NOT ACCEPTED\n";
    }

    sendWont(charSettingValue(TelnetSettings::LINEMODE));

    int i = 0;

    while (true) {
        char c = server_->readCharacter();
        printf("%d ", (unsigned char) c);
        //std::cout << (int)(unsigned char) c << " ";
        if (c == '\e') {
            printf("ESCAPE\n");
            c = server_->readCharacter();
            printf("s: %d \n", (unsigned char) c);
            if (c == '[') {
                c = server_->readCharacter();
                printf("s: %d \n", (unsigned char) c);
                if (c == 'B') {
                    printf("B\n");
                    break;
                }
            }
        }
        ++i;
        if (i == 30) {
            break;
        }
    }
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
