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

int main(int argc, char *argv[]) {
    qt_init(argc, argv);

    harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::INIT_CONN, nullptr));
    std::thread har_evt(&harmony::event_process);

    int res = qt_run();

    closeThreads();

    return res;
}

/**
 * @brief closeThreads Closes all active threads for graceful shutdown
 * @param har_evt Main Event handler
 */
void closeThreads() {
    harmony::event_interrupt();
    harmony::conv::close_comm();
    harmony::spread_exit();
}
