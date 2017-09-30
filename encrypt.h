#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include "room.h"

namespace harmony
{
	namespace conv
	{
        class HarmonyRoomInterface : public np1sec::RoomInterface {
        public:
            HarmonyRoomInterface() {}
            void send_message(const std::string& message);
            np1sec::TimerToken* set_timer(uint32_t interval, np1sec::TimerCallback* callback);
            void connected();
            void disconnected();
            void user_joined(const std::string& username, const np1sec::PublicKey& public_key);
            void user_left(const std::string& username, const np1sec::PublicKey& public_key);
            np1sec::ConversationInterface* created_conversation(np1sec::Conversation* conversation);
            np1sec::ConversationInterface* invited_to_conversation(np1sec::Conversation* conversation, const std::string& username);
        };

        class HarmonyConversationInterface final : public np1sec::ConversationInterface {
        public:
            HarmonyConversationInterface(np1sec::Conversation* conversation_) : conversation(conversation_) {}
            void user_invited(const std::string& inviter, const std::string& invitee);
            void invitation_cancelled(const std::string& inviter, const std::string& invitee);
            void user_authenticated(const std::string& username, const np1sec::PublicKey& public_key);
            void user_authentication_failed(const std::string& username);
            void user_joined(const std::string& username);
            void user_left(const std::string& username);
            void votekick_registered(const std::string& kicker, const std::string& victim, bool kicked);

            void user_joined_chat(const std::string& username);
            void message_received(const std::string& sender, const std::string& message);

            void joined();
            void joined_chat();
            void left();

        public:
            np1sec::Conversation* conversation;
            std::string id;
        };

        struct invite_notification {
            std::string from;
            HarmonyConversationInterface* internal_ptr;
            invite_notification(const std::string& from, HarmonyConversationInterface* internal_ptr)
                : from(from), internal_ptr(internal_ptr) {}
        };

        struct invite_out {
            std::string who;
            std::string conv;
            invite_out(std::string who, std::string conv) : who(who), conv(conv) {}
        };

        struct conv_message {
            std::string conv;
            std::string sender;
            std::string message;
            conv_message(std::string conv, std::string sender, std::string message) : conv(conv), sender(sender), message(message) {}
        };

		void send_plaintext(const std::string& conv, const std::string& message);
        void received_message(const std::string& sender, const std::string& message);
		void make_conv();
		void join_conv(invite_notification inv);
		void init_comm(std::string nickname);
        void send_invite(invite_out invite);
        void user_left(std::string username);
		void close_comm();

        std::string unique_id();

        std::string default_conv();
        std::string my_username();
	}
}
