#include <iostream>

#include <event.h>

int main() {
    // todo init

    while (true) {
        harmony::event_process();
    }

    system("PAUSE");
    return 0;
}