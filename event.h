#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

namespace harmony {
    class Event {
    public:
        std::string name;
    };
}