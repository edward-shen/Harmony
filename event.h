#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

namespace harmony {
    enum class EventType {
        // UI to np1sec
        /// event_data is conv_message* with the message
        SEND_PLAINTEXT,
        /// event_data is nullptr
        MAKE_CONV,
        /// event_data is invite_notification
        JOIN_CONV,
        /// event_data is nullptr
        INIT_CONN,
        /// event_data is the spread sender name
        INIT_NP1SEC,
        /// event_data is invite_out* of user to invite
        SEND_INVITE,
        CLOSE_CONN,

        // np1sec to UI
        /// event_data is invite_notification
        PROMPT_INVITE,
        /// event_data is the name of the conversation.
        CONV_JOINED,
        /// event_data is conv_message* with the message
        RECV_PLAINTEXT,
        NP1SEC_ERROR,

        // np1sec to spread
        /// event_data is std::string* with the message
        SEND_CIPHERTEXT,
        /// event_data is std::vector<std::string>* with the current room user list
        UPDATE_USERS,

        // spread to np1sec
        /// event_data is spread_recv* with the message
        RECV_CIPHERTEXT,
        /// event_data is std::string* of the username
        USER_LEFT,
        // User left the conversation
        CONV_LEAVE,
        // spread to UI
        SPREAD_ERROR
    };

    /// Represents an event. Will free data pointer on destruct.
    class Event {
    public:
        EventType type;
        void* event_data;
        Event(EventType type, void* event_data) : type(type), event_data(event_data) {}
    };

    void event_queue(std::unique_ptr<Event>);
    void event_process();
    void event_interrupt();
}
