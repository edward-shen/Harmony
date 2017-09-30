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
