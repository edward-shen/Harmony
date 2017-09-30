#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

namespace harmony {
    enum class EventType {
        // UI to np1sec
        /// event_data is std::string* with the message
        SEND_PLAINTEXT,
        MAKE_ROOM,
        JOIN_ROOM,
        /// event_data is nullptr
        INIT_CONN,
        CLOSE_CONN,

        // np1sec to UI
        PROMPT_INVITE,
        /// event_data is std::string* with the message
        RECV_PLAINTEXT,
        NP1SEC_ERROR,

        // np1sec to spread
        /// event_data is std::string* with the message
        SEND_CIPHERTEXT,

        // spread to np1sec
        /// event_data is std::string* with the message
        RECV_CIPHERTEXT,

        // spread to UI
        SPREAD_ERROR
    };

    /// Represents an event. Will free data pointer on destruct.
    class Event {
    public:
        EventType type;
        void* event_data;

        Event(EventType type, void* event_data) : type(type), event_data(event_data) {
        }

        virtual ~Event() {
            delete event_data;
        }
    };

    void event_queue(std::unique_ptr<Event>);
    void event_process();
}