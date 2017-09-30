#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

namespace harmony {
    enum class EventType {
        // UI to np1sec
        SEND_PLAINTEXT,
        MAKE_ROOM,
        JOIN_ROOM,
        INIT_CONN,
        CLOSE_CONN,

        // np1sec to UI
        PROMPT_INVITE,
        RECV_PLAINTEXT,
        NP1SEC_ERROR,

        // np1sec to spread
        SEND_CIPHERTEXT,

        // spread to np1sec
        RECV_CIPHERTEXT,

        // spread to UI
        SPREAD_ERROR
    };

    /// Represents an event. Will free data pointer on destruct.
    class Event {
    public:
        EventType type;
        void* event_data;

        virtual ~Event() {
            delete event_data;
        }
    };

    void event_queue(std::unique_ptr<Event>);
    void event_process();
}