#pragma once

#define MAX_MESSLEN     102400
#define MAX_VSSETS      10
#define MAX_MEMBERS     100

#include <string>

namespace harmony {
    void spread_init();

    void spread_send(std::string message);

    void spread_recv_thread();

    struct spread_recv {
        enum mt {
            ERROR,
            MESSAGE,
            STATUS
        } type;
        std::string sender;
        std::string data;

        spread_recv(mt type, char* sender, char* data)
            : type(type), sender(sender), data(data) {}
    };

    spread_recv spread_block_recv();
}