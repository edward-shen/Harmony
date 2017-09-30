#include "event.h"
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include "interface.h"
#include "room.h"
#include "encrypt.h"
#include <json.hpp>
using nlohmann::json;

namespace harmony {
    namespace conv {
        static std::string username;
        static np1sec::Room* room;

        static std::unordered_map<std::string, np1sec::Conversation*> conversations;

        static bool first_conv_init = false;
        static std::string _default_conv;

        std::string default_conv() {
            return _default_conv;
        }

        /// Returns a unique local conversation identifier.
        std::string unique_id() {
            static int id = 0;
            std::ostringstream os;
            os << "conv" << id;
            id++;
            return os.str();
        }

        /// Returns our current username
        std::string my_username() {
            return username;
        }

        /// Initializes n+1sec. Assumes network layer is ready.
        /// nickname: your nickname
        void init_comm(std::string _username) {
            username = _username;
            HarmonyRoomInterface* itf = new HarmonyRoomInterface();
            room = new np1sec::Room(itf, username, np1sec::PrivateKey::generate(true));
            room->connect();
            make_conv();
        }

        /// creates an np1sec conversation
        void make_conv() {
            room->create_conversation();
        }

        /// joins an np1sec conversation
        void join_conv(invite_notification inv) {
            HarmonyConversationInterface* conv = (HarmonyConversationInterface*) inv.internal_ptr;
            conv->conversation->join();
            std::string id = unique_id();
            conv->id = id;
            conversations[id] = conv->conversation;
            event_queue(std::make_unique<Event>(EventType::CONV_JOINED, new std::string(id)));
        }

        void close_comm() {}

        void user_left(std::string username) {
            room->user_left(username);
        }

        void received_message(const std::string& sender, const std::string& message) {
            room->message_received(sender, message);
        }

        void leave(np1sec::Conversation* conversation) {
            if (room) {
                if (!conversation) {
                    return;
                }
                conversation->leave(false);
            }
        }

        void send_invite(invite_out inv) {
            np1sec::Conversation* conversation = conversations[inv.conv];
            if (room) {
                std::map<std::string, np1sec::PublicKey> users = room->users();
                std::cout << "inviting " << inv.who << std::endl;
                if (!users.count(inv.who)) {
                    std::cerr << "no such user" << std::endl;
                    return;
                }
                if (!conversation) {
                    std::cerr << "no such conv" << std::endl;
                    return;
                }
                conversation->invite(inv.who, users[inv.who]);
            }
        }

        /// Queues a plaintext message for encryption.
        void send_plaintext(const std::string& conv, const std::string& plaintext) {
            np1sec::Conversation* conversation = conversations[conv];
            if (room) {
                if (!conversation) {
                    return;
                }
                conversation->send_chat(plaintext);
            }
        }

        void HarmonyConversationInterface::user_invited(const std::string& inviter, const std::string& invitee) {
            std::cerr << "User " << invitee << " was invited by " << inviter << " on " << id << std::endl;
        }

        void HarmonyConversationInterface::invitation_cancelled(const std::string& inviter, const std::string& invitee) {
            std::cerr << "User " << invitee << " was uninvited by " << inviter << " on " << id << std::endl;
        }

        void HarmonyConversationInterface::user_authenticated(const std::string& username, const np1sec::PublicKey& public_key) {
            std::cerr << "User " << username << " auth'd " << id << std::endl;
        }

        void HarmonyConversationInterface::user_authentication_failed(const std::string& username) {
            std::cerr << "User " << username << " failed auth " << id << std::endl;
        }

        void HarmonyConversationInterface::user_joined(const std::string& username) {
            std::cerr << "User " << username << " partial joined " << id << std::endl;
        }

        void HarmonyConversationInterface::user_left(const std::string& username) {
            std::cerr << "User " << username << " left " << id << std::endl;
        }

        void HarmonyConversationInterface::votekick_registered(const std::string& kicker, const std::string& victim, bool kicked) {

        }

        void HarmonyConversationInterface::user_joined_chat(const std::string& username) {
            std::cerr << "User " << username << " full joined " << id << std::endl;
        }

        void HarmonyConversationInterface::message_received(const std::string& sender, const std::string& message) {
            conv_message* msg = new conv_message(id, sender, message);
            event_queue(std::make_unique<Event>(EventType::RECV_PLAINTEXT, msg));
        }

        void HarmonyConversationInterface::joined() {
            std::cerr << "Partial join on " << id << std::endl;
        }

        void HarmonyConversationInterface::joined_chat() {
            std::cerr << "Full join on " << id << std::endl;
        }

        void HarmonyConversationInterface::left() {
            std::cerr << "Left " << id << std::endl;
        }

        void HarmonyRoomInterface::send_message(const std::string& message) {
            std::string* heap_str = new std::string(message);
            event_queue(std::make_unique<Event>(EventType::SEND_CIPHERTEXT, heap_str));
        }

        class HarmonyTimerToken final : public np1sec::TimerToken {
        public:
            class Storage : private std::set<HarmonyTimerToken*> {
            public:
                friend class HarmonyTimerToken;

                ~Storage() {
                    for (auto* p : *this) delete p;
                }
            };
        private:
            Storage& storage;
            np1sec::TimerCallback* callback;
            std::condition_variable cv;
            std::mutex cv_m;
            volatile bool canceled;
            std::chrono::time_point<std::chrono::system_clock> when;
            std::thread wait_thread;
        public:
            HarmonyTimerToken(Storage& storage, uint32_t interval_ms, np1sec::TimerCallback* callback)
                : storage(storage), callback(callback), canceled(false),
                when(std::chrono::system_clock::now() + std::chrono::milliseconds(interval_ms)),
                wait_thread(std::thread(std::bind(&HarmonyTimerToken::timer_func, this))) {

                storage.insert(this);
            }

            void unset() override {
                canceled = true;
                cv.notify_all();
                wait_thread.join();

                #define delet delete
                
                delet this;
            }

            ~HarmonyTimerToken() {
                storage.erase(this);
            }
        private:
            void timer_func() {
                std::unique_lock<std::mutex> lk(cv_m);
                while (!canceled) {
                    auto ret = cv.wait_until(lk, when);
                    if (ret == std::cv_status::timeout) {
                        callback->execute();
                        return;
                    }
                }
            }
        };

        np1sec::TimerToken* HarmonyRoomInterface::set_timer(uint32_t interval, np1sec::TimerCallback* callback) {
            static HarmonyTimerToken::Storage storage;
            return new HarmonyTimerToken(storage, interval, callback);
        }

        void HarmonyRoomInterface::connected() {
            std::cerr << "Room connected" << std::endl;
        }

        void HarmonyRoomInterface::disconnected() {
            std::cerr << "Room disconnected" << std::endl;
        }

        void HarmonyRoomInterface::user_joined(const std::string& username, const np1sec::PublicKey& public_key) {
            std::cerr << "Room user joined " << username << std::endl;
        }

        void HarmonyRoomInterface::user_left(const std::string& username, const np1sec::PublicKey& public_key) {
            std::cerr << "Room user left " << username << std::endl;
        }

        np1sec::ConversationInterface* HarmonyRoomInterface::created_conversation(np1sec::Conversation* conversation) {
            HarmonyConversationInterface* interface = new HarmonyConversationInterface(conversation);
            std::string id = unique_id();
            interface->id = id;
            conversations[id] = conversation;
            event_queue(std::make_unique<Event>(EventType::CONV_JOINED, new std::string(id)));
            if (!first_conv_init) {
                first_conv_init = true;
                _default_conv = id;
            }
            return interface;
        }

        np1sec::ConversationInterface* HarmonyRoomInterface::invited_to_conversation(np1sec::Conversation* conversation, const std::string& username) {
            HarmonyConversationInterface* interface = new HarmonyConversationInterface(conversation);
            invite_notification* inv = new invite_notification(username, interface);
            event_queue(std::make_unique<Event>(EventType::PROMPT_INVITE, inv));
            return interface;
        }
    }
}
