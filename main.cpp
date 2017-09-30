#include "main.h"

#include <iostream>
#include <memory>
#include <string>
#include <cctype>
#include <thread>

#include "encrypt.h"
#include "event.h"
#include "spread.h"

#include "qt_main.h"

#pragma comment(lib, "Ws2_32.lib")

/*
static harmony::conv::invite_notification* current_inv = nullptr;
static std::string current_conv = "_none";

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

void temp_stdin_reader() {
    std::cerr << "Starting reader thread" << std::endl;
    for (std::string line; std::getline(std::cin, line);) {
        if (current_inv != nullptr) {
            if (line[0] == 'y' || line[0] == 'Y') {
                std::cout << "Accepting invite..." << std::endl;
                harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::JOIN_CONV, current_inv));
            } else {
                std::cout << "Ignoring invite" << std::endl;
            }
            current_inv = nullptr;
        } else if (line.substr(0, 5) == "conv ") {
            std::string conv = line.substr(5);
            trim(conv);
            std::cout << "changing conv to " << conv << std::endl;
            current_conv = conv;
        } else if (line.substr(0, 3) == "new") {
            std::cout << "creating conv" << std::endl;
            harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::MAKE_CONV, current_inv));
        } else if (line.substr(0, 7) == "invite ") {
            std::string who = line.substr(7);
            trim(who);
            harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::SEND_INVITE,
                new harmony::conv::invite_out(who, current_conv)));
        } else {
            harmony::conv::conv_message* msg =
                new harmony::conv::conv_message(current_conv, harmony::conv::my_username(), line);
            std::unique_ptr<harmony::Event> evt = std::make_unique<harmony::Event>(harmony::EventType::SEND_PLAINTEXT, msg);
            harmony::event_queue(std::move(evt));
        }
    }
}
*/
void temp_display_plaintext(harmony::conv::conv_message msg) {
    //std::cout << "Got plaintext on " << msg.conv << " from " << msg.sender << ": " << msg.message << std::endl;
}

void temp_notify_conv_joined(std::string conv) {
    /*std::cout << "============ joined conversation: " << conv << std::endl;
    if (current_conv == "_none") {
        current_conv = conv;
    }*/
}

void temp_prompt_invite(harmony::conv::invite_notification inv) {
   // std::cout << "============ invited to conv by " << inv.from << ". Accept? ";
    //current_inv = new harmony::conv::invite_notification(inv);
}

void harmony_events_thread() {
    harmony::event_process();
}

int main(int argc, char *argv[]) {
    qt_init(argc, argv);

    harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::INIT_CONN, nullptr));
    std::thread har_evt(harmony_events_thread);

    int res = qt_run();

    std::cout << "interrupting events" << std::endl;
    harmony::event_interrupt();
    har_evt.join();

    std::cout << "closing comm" << std::endl;
    harmony::conv::close_comm();
    std::cout << "exiting spread" << std::endl;
    harmony::spread_exit();
    std::cout << "cleaned up" << std::endl;
    return res;
}
