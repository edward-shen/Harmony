#include "event.h"
#include <iostream>

namespace harmony {
    // https://stackoverflow.com/a/16075550
    template <class T>
    class SafeQueue {
    public:
        SafeQueue(void)
            : q()
            , m()
            , c() {}

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
            }
            std::unique_ptr<T> p2 = std::move(q.front());
            q.pop();
            return p2;
        }

    private:
        std::queue<std::unique_ptr<T>> q;
        mutable std::mutex m;
        std::condition_variable c;
    };

    static SafeQueue<Event> g_evt_queue;

    void event_queue(std::unique_ptr<Event> evt) {
        g_evt_queue.enqueue(std::move(evt));
    }

    void event_process() {
        std::unique_ptr<Event> evt = g_evt_queue.dequeue();
        
        switch (evt->type) {
            case EventType::SEND_PLAINTEXT:

                break;
            case EventType::MAKE_ROOM:

                break;
            case EventType::JOIN_ROOM:

                break;
            case EventType::INIT_CONN:

                break;
            case EventType::CLOSE_CONN:

                break;
            case EventType::PROMPT_INVITE:

                break;
            case EventType::RECV_PLAINTEXT:

                break;
            case EventType::NP1SEC_ERROR:

                break;
            case EventType::SPREAD_ERROR:

                break;
            default:
                throw std::exception("you programmed something wrong");
        }
    }
}