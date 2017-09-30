#include "spread.h"
#include "event.h"

#include <iostream>
#include <thread>
#include <sp.h>

static mailbox mbox;

namespace harmony {
    std::thread recv;

    void spread_init() {
        char priv_grp[MAX_GROUP_NAME];

        sp_time test_timeout;

        test_timeout.sec = 5;
        test_timeout.usec = 0;
        int ret = SP_connect_timeout("3333@localhost", "harmony", 0, 1, &mbox, priv_grp, test_timeout);
        if (ret != ACCEPT_SESSION) {
            SP_error(ret);
            throw std::runtime_error("spread error");
        }

        std::cerr << "Spread connected to 3333@localhost with self group: " << priv_grp << std::endl;

        ret = SP_join(mbox, "harmony");
        if (ret != 0) {
            SP_error(ret);
            throw std::runtime_error("spread error");
        }

        std::cerr << "Joined group harmony" << std::endl;

        recv = std::thread(spread_recv_thread);

        std::string* heap_str = new std::string(priv_grp);
        event_queue(std::make_unique<Event>(EventType::INIT_NP1SEC, heap_str));
    }

    void spread_send(std::string message) {
        int ret = SP_multicast(mbox, AGREED_MESS, "harmony", 0, message.size(), message.c_str());
        if (ret < 0) {
            SP_error(ret);
            throw std::runtime_error("spread error");
        }
    }

    void spread_recv_thread() {
        while (true) {
            harmony::spread_recv mess = harmony::spread_block_recv();
            if (mess.type == harmony::spread_recv::ERROR) {
                std::unique_ptr<Event> evt = std::make_unique<Event>(EventType::SPREAD_ERROR, nullptr);
                event_queue(std::move(evt));
            } else if (mess.type == harmony::spread_recv::STATUS) {
                // do something
            } else if (mess.type == harmony::spread_recv::MESSAGE) {
                harmony::spread_recv* heap_sv = new harmony::spread_recv(mess);
                std::unique_ptr<Event> evt = std::make_unique<Event>(EventType::RECV_CIPHERTEXT, heap_sv);
                event_queue(std::move(evt));
            } else {
                throw std::runtime_error("wtf");
            }
        }
    }

    spread_recv spread_block_recv() {
        static char      mess[MAX_MESSLEN];
        char             sender[MAX_GROUP_NAME];
        char             target_groups[MAX_MEMBERS][MAX_GROUP_NAME];
        membership_info  memb_info;
        vs_set_info      vssets[MAX_VSSETS];
        unsigned int     my_vsset_index;
        int              num_vs_sets;
        char             members[MAX_MEMBERS][MAX_GROUP_NAME];
        int              num_groups;
        int              service_type;
        int16            mess_type;
        int              endian_mismatch;
        int              i, j;
        int              ret;

        service_type = 0;

        ret = SP_receive(mbox, &service_type, sender, 100, &num_groups, target_groups,
            &mess_type, &endian_mismatch, sizeof(mess), mess);
        if (ret < 0) {
            if ((ret == GROUPS_TOO_SHORT) || (ret == BUFFER_TOO_SHORT)) {
                service_type = DROP_RECV;
                std::cerr << "Buffers too short to receive" << std::endl;
                ret = SP_receive(mbox, &service_type, sender, MAX_MEMBERS, &num_groups, target_groups,
                    &mess_type, &endian_mismatch, sizeof(mess), mess);
            }
        }
        if (ret < 0) {
            SP_error(ret);
            return spread_recv(spread_recv::ERROR, "<error>", "<error>");
        }

        if (Is_regular_mess(service_type)) {
            mess[ret] = 0;
            if (Is_unreliable_mess(service_type)) fprintf(stderr, "received UNRELIABLE ");
            else if (Is_reliable_mess(service_type)) fprintf(stderr, "received RELIABLE ");
            else if (Is_fifo_mess(service_type)) fprintf(stderr, "received FIFO ");
            else if (Is_causal_mess(service_type)) fprintf(stderr, "received CAUSAL ");
            else if (Is_agreed_mess(service_type)) fprintf(stderr, "received AGREED ");
            else if (Is_safe_mess(service_type)) fprintf(stderr, "received SAFE ");
            fprintf(stderr, "message from %s, of type %d, (endian %d) to %d groups \n(%d bytes): %s\n",
                sender, mess_type, endian_mismatch, num_groups, ret, mess);
        } else if (Is_membership_mess(service_type)) {
            ret = SP_get_memb_info(mess, service_type, &memb_info);
            if (ret < 0) {
                fprintf(stderr, "BUG: membership message does not have valid body\n");
                SP_error(ret);
                exit(1);
            }
            if (Is_reg_memb_mess(service_type)) {
                fprintf(stderr, "Received REGULAR membership for group %s with %d members, where I am member %d:\n",
                    sender, num_groups, mess_type);
                for (i = 0; i < num_groups; i++)
                    fprintf(stderr, "\t%s\n", &target_groups[i][0]);
                fprintf(stderr, "grp id is %d %d %d\n", memb_info.gid.id[0], memb_info.gid.id[1], memb_info.gid.id[2]);

                if (Is_caused_join_mess(service_type)) {
                    fprintf(stderr, "Due to the JOIN of %s\n", memb_info.changed_member);
                } else if (Is_caused_leave_mess(service_type)) {
                    fprintf(stderr, "Due to the LEAVE of %s\n", memb_info.changed_member);

                    event_queue(std::make_unique<Event>(EventType::USER_LEFT, new std::string(memb_info.changed_member)));

                } else if (Is_caused_disconnect_mess(service_type)) {
                    fprintf(stderr, "Due to the DISCONNECT of %s\n", memb_info.changed_member);

                    event_queue(std::make_unique<Event>(EventType::USER_LEFT, new std::string(memb_info.changed_member)));

                } else if (Is_caused_network_mess(service_type)) {
                    fprintf(stderr, "Due to NETWORK change with %u VS sets\n", memb_info.num_vs_sets);
                    num_vs_sets = SP_get_vs_sets_info(mess, &vssets[0], MAX_VSSETS, &my_vsset_index);
                    if (num_vs_sets < 0) {
                        fprintf(stderr, "BUG: membership message has more then %d vs sets. Recompile with larger MAX_VSSETS\n", MAX_VSSETS);
                        SP_error(num_vs_sets);
                        exit(1);
                    }
                    for (i = 0; i < num_vs_sets; i++) {
                        fprintf(stderr, "%s VS set %d has %u members:\n",
                            (i == my_vsset_index) ?
                            ("LOCAL") : ("OTHER"), i, vssets[i].num_members);
                        ret = SP_get_vs_set_members(mess, &vssets[i], members, MAX_MEMBERS);
                        if (ret < 0) {
                            fprintf(stderr, "VS Set has more then %d members. Recompile with larger MAX_MEMBERS\n", MAX_MEMBERS);
                            SP_error(ret);
                            exit(1);
                        }
                        for (j = 0; j < vssets[i].num_members; j++)
                            fprintf(stderr, "\t%s\n", members[j]);
                    }
                }
            } else if (Is_transition_mess(service_type)) {
                fprintf(stderr, "received TRANSITIONAL membership for group %s\n", sender);
            } else if (Is_caused_leave_mess(service_type)) {
                fprintf(stderr, "received membership message that left group %s\n", sender);
            } else fprintf(stderr, "received incorrecty membership message of type 0x%x\n", service_type);
        } else if (Is_reject_mess(service_type)) {
            fprintf(stderr, "REJECTED message from %s, of servicetype 0x%x messtype %d, (endian %d) to %d groups \n(%d bytes): %s\n",
                sender, service_type, mess_type, endian_mismatch, num_groups, ret, mess);
        } else fprintf(stderr, "received message of unknown message type 0x%x with ret %d\n", service_type, ret);
       
        return spread_recv(Is_regular_mess(service_type) ? spread_recv::MESSAGE : spread_recv::STATUS, sender, mess);
    }
}
