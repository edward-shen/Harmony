#include "event.h"
#include "spread.h"
#include "encrypt.h"
#include "harmony-ui/mainwindow.h"
#include "qt_main.h"

#include <iostream>

namespace harmony {
    volatile bool stop_events = false;

    // https://stackoverflow.com/a/16075550
    template <class T>
    class SafeQueue {
    public:
        SafeQueue(void) : q(), m(), c() {}

        ~SafeQueue(void) {}

        // Add an element to the queue.
        void enqueue(std::unique_ptr<T> t) {
            std::lock_guard<std::mutex> lock(m);
            q.push(std::move(t));
            c.notify_one();
        }

        // Get the "front"-element.
        // If the queue is empty, wait till a element is avaiable.
        std::unique_ptr<T> dequeue(void) {
            std::unique_lock<std::mutex> lock(m);
            while (q.empty()) {
                // release lock as long as the wait and reaquire it afterwards.
                c.wait(lock);
                if (stop_events) return std::unique_ptr<T>(nullptr);
            }
            std::unique_ptr<T> p2 = std::move(q.front());
            q.pop();
            return p2;
        }

    public:
        std::queue<std::unique_ptr<T>> q;
        mutable std::mutex m;
        std::condition_variable c;
    };

    static SafeQueue<Event> g_evt_queue;

    void event_queue(std::unique_ptr<Event> evt) {
        g_evt_queue.enqueue(std::move(evt));
    }

    void event_interrupt() {
        stop_events = true;
        g_evt_queue.c.notify_all();
    }

    void event_process() {
        while (!stop_events) {
            std::unique_ptr<Event> evt = g_evt_queue.dequeue();
            if (stop_events) return;

            switch (evt->type) {
                case EventType::SEND_PLAINTEXT: {
                    harmony::conv::conv_message* data = (harmony::conv::conv_message*) evt->event_data;
                    harmony::conv::send_plaintext(data->conv, data->message);
                    delete data;
                    break;
                }
                case EventType::MAKE_CONV:
                    harmony::conv::make_conv();
                    break;
                case EventType::JOIN_CONV: {
                    harmony::conv::invite_notification* inv = (harmony::conv::invite_notification*) evt->event_data;
                    harmony::conv::join_conv(*inv);
                    delete inv;
                    break;
                }
                case EventType::INIT_CONN:
                    harmony::spread_init();
                    break;
                case EventType::INIT_NP1SEC: {
                    std::string* data = (std::string*) evt->event_data;
                    harmony::conv::init_comm(*data);
                    delete data;
                    break;
                }
                case EventType::SEND_INVITE: {
                    harmony::conv::invite_out* inv = (harmony::conv::invite_out*) evt->event_data;
                    harmony::conv::send_invite(*inv);
                    delete inv;
                    break;
                }
                case EventType::CLOSE_CONN:

                    break;
                case EventType::PROMPT_INVITE: {
                    harmony::conv::invite_notification* inv = (harmony::conv::invite_notification*) evt->event_data;
                    MainWindow::recieve_conversation_invite(inv);
                    delete inv;
                    break;
                }
                case EventType::CONV_JOINED: {
                    std::string* conv_name = (std::string*) evt->event_data;
                    MainWindow::recieve_conversation_joined(*conv_name);
                    delete conv_name;
                    break;
                }
                case EventType::RECV_PLAINTEXT: {
                    harmony::conv::conv_message* msg = (harmony::conv::conv_message*) evt->event_data;
                    MainWindow::recieve_plaintext(msg);
                    delete msg;
                    break;
                }
                case EventType::NP1SEC_ERROR:

                    break;
                case EventType::SEND_CIPHERTEXT: {
                    std::string* data = (std::string*) evt->event_data;
                    spread_send(*data);
                    delete data;
                    break;
                }
                case EventType::UPDATE_USERS: {
                    std::vector<std::string>* data = (std::vector<std::string>*) evt->event_data;
                    MainWindow::recieve_user_list(data);
                    delete data;
                    break;
                }
                case EventType::RECV_CIPHERTEXT: {
                    spread_recv* data = (spread_recv*) evt->event_data;
                    harmony::conv::received_message(data->sender, data->data);
                    delete data;
                    break;
                }
                case EventType::USER_LEFT: {
                    std::string* data = (std::string*) evt->event_data;
                    harmony::conv::user_left(*data);
                    delete data;
                    break;
                }
                case EventType::CONV_LEAVE: {
                    std::string* data = (std::string*) evt->event_data;
                    harmony::conv::leave_conv(*data);
                    delete data;
                    break;
                }
                case EventType::SPREAD_ERROR:

                    break;
                default:
                    throw std::runtime_error("you programmed something wrong");
            }
        }
    }
}
