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

/**
 * @brief closeThreads Closes all active threads for graceful shutdown
 * @param har_evt Main Event handler
 */
void closeThreads() {
    harmony::event_interrupt();
    harmony::conv::close_comm();
    harmony::spread_exit();
}

int main(int argc, char *argv[]) {
    qt_init(argc, argv);

    // Initializes our Spread toolkit client by passing an initialization event into the queue on start.
    harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::INIT_CONN, nullptr));

    // Initializses the event loop thread
    std::thread har_evt(&harmony::event_process);

    // Runs our UI thread.
    int res = qt_run();

    // After our UI closes, close our stuff
    closeThreads();
    return res;
}

