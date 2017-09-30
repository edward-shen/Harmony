#include <iostream>
#include <memory>
#include <string>

#include <event.h>
#include <spread.h>

#pragma comment(lib, "Ws2_32.lib")

void stub_stdin_reader() {
    std::cerr << "Starting reader thread" << std::endl;
    for (std::string line; std::getline(std::cin, line);) {
        std::string* heap_str = new std::string(line);
        std::unique_ptr<harmony::Event> evt = std::make_unique<harmony::Event>(harmony::EventType::SEND_CIPHERTEXT, heap_str);
        harmony::event_queue(std::move(evt));
    }
}

int main() {
    harmony::event_queue(std::make_unique<harmony::Event>(harmony::EventType::INIT_CONN, nullptr));

    std::thread reader(stub_stdin_reader);

    while (true) {
        harmony::event_process();
    }

    system("PAUSE");
    return 0;
}