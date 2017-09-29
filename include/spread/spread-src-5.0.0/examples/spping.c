/*
 * The Spread Toolkit.
 *     
 * The contents of this file are subject to the Spread Open-Source
 * License, Version 1.0 (the ``License''); you may not use
 * this file except in compliance with the License.  You may obtain a
 * copy of the License at:
 *
 * http://www.spread.org/license/
 *
 * or in the file ``license.txt'' found in this distribution.
 *
 * Software distributed under the License is distributed on an AS IS basis, 
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License 
 * for the specific language governing rights and limitations under the 
 * License.
 *
 * The Creators of Spread are:
 *  Yair Amir, Michal Miskin-Amir, Jonathan Stanton, John Schultz.
 *
 *  Copyright (C) 1993-2014 Spread Concepts LLC <info@spreadconcepts.com>
 *
 *  All Rights Reserved.
 *
 * Major Contributor(s):
 * ---------------
 *    Amy Babay            babay@cs.jhu.edu - accelerated ring protocol.
 *    Ryan Caudy           rcaudy@gmail.com - contributions to process groups.
 *    Claudiu Danilov      claudiu@acm.org - scalable wide area support.
 *    Cristina Nita-Rotaru crisn@cs.purdue.edu - group communication security.
 *    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.
 *    Dan Schoenblum       dansch@cnds.jhu.edu - Java interface.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <signal.h>

#include "sp.h"

typedef struct
{
  uint32_t      seq;
  uint32_t      secs;
  uint32_t      usecs;
  unsigned char is_ping;
  char          origin[MAX_GROUP_NAME];

} ping_pkt;

#define MAX_MESSLEN 100000

typedef union
{
  ping_pkt ping;
  char     mess[MAX_MESSLEN];
  
} msg_union;

static char      Spread_name[300]      = "4803";
static char     *User                  = NULL;
static char      Group[MAX_GROUP_NAME] = "spping";
static int       Period                = 1000;
static int       Num_bytes             = (int) sizeof(ping_pkt);
static int       Num_messages          = -1;
static int       Read_only             = 0;
static int       Send_service_type     = (RELIABLE_MESS | SELF_DISCARD);

static char      Private_group[MAX_GROUP_NAME];
static mailbox   Mbox;

static sp_time   SP_period;

#define SPPING_MAX_GROUPS 100

static int       Ret;
static int       Service_type;
static int       Num_groups;
static char      Sender[MAX_GROUP_NAME];
static char      Ret_groups[SPPING_MAX_GROUPS][MAX_GROUP_NAME];
static int16     Mess_type;
static int       Endian_mismatch;
static msg_union Recv_mess;

static uint32_t  Send_counter;
static msg_union Send_mess;

static long     *Samples;
static int       Samples_num;
static int       Samples_size;

static void print_help(const char *msg)
{
  if (msg != NULL)
    fprintf(stderr, "Error: %s\n\n", msg);
  
  fprintf(stderr, "Usage: spping\n"
          "\t[-s <spread name>]   : either port or port@hostname (default: 4803)\n"
          "\t[-u <user name>]     : unique (on daemon) user name (default: <random>)\n"
          "\t[-g <group name>]    : group to which to send/recv (default: spping)\n"
          "\t[-p <period>]        : time in ms between sending pings (default: 1000)\n"
          "\t[-b <num bytes>]     : number of bytes per message (default: 48)\n"
          "\t[-m <num messages>]  : number of messages (default: infinite)\n"
          "\t[-ro]                : reflect only\n");
  
  exit(1);
}

static void Usage(int argc, char *argv[])
{
  char junk;
  
  for (--argc, ++argv; argc > 0; --argc, ++argv)
  {
    if (!strncmp(*argv, "-s", 2))
    {
      if (argc < 2 || strlen(argv[1]) >= sizeof(Spread_name))
        print_help("Invalid -s parameter");
      
      strncpy(Spread_name, argv[1], sizeof(Spread_name));
      --argc, ++argv;
    }
    else if (!strncmp(*argv, "-u", 2))
    {
      if (argc < 2)
        print_help("Invalid -u parameter");

      User = argv[1];
      --argc, ++argv;
    }
    else if (!strncmp(*argv, "-g", 2))
    {
      if (argc < 2 || strlen(argv[1]) >= sizeof(Group))
        print_help("Invalid -g parameter");
      
      strncpy(Group, argv[1], sizeof(Group));
      --argc, ++argv;
    }
    else if (!strncmp(*argv, "-p", 2))
    {
      if (argc < 2 || sscanf(argv[1], "%d%c", &Period, &junk) != 1 || Period < 0)
        print_help("Invalid -p parameter");
      
      --argc, ++argv;
    }
    else if (!strncmp(*argv, "-b", 2))
    {
      if (argc < 2 || sscanf(argv[1], "%d%c", &Num_bytes, &junk) != 1 || Num_bytes < (int) sizeof(ping_pkt) || Num_bytes > MAX_MESSLEN)
        print_help("Invalid -b parameter");
      
      --argc, ++argv;
    }
    else if (!strncmp(*argv, "-m", 2))
    {
      if (argc < 2 || sscanf(argv[1], "%d%c", &Num_messages, &junk) != 1)
        print_help("Invalid -m parameter");
      
      --argc, ++argv;
    }
    else if (!strncmp(*argv, "-ro", 3))
      Read_only  = 1;
    
    else
      print_help("Unrecognized parameter");
  }

  SP_period.sec  = Period / 1000;
  SP_period.usec = (Period % 1000) * 1000;
}

int long_cmp(const void *l, const void *r)
{
  return (*(const long*) l < *(const long*) r ? -1 : (*(const long*) l != *(const long*) r ? 1 : 0));
}

static void compute_print_stats(void)
{
  printf("\n"
    "--- %s ping statistics ---\n"
    "%d packets transmitted, %d packets received, %f%% packet loss\n",
    Group, Send_counter, Samples_num, (Send_counter ? (Send_counter - Samples_num) * 100.0 / Send_counter : 0.0));
  
  if (Samples)
  {
    long   p0, p01, p05, p1, p5, p10, p25, p50, p75, p90, p95, p99, p995, p999, p100;
    double mean = 0.0, kvar = 0.0, stddev;
    int    i;

    qsort(Samples, Samples_num, sizeof(long), long_cmp);

    p0   = Samples[0];
    p01  = Samples[Samples_num / 1000];
    p05  = Samples[Samples_num / 200];
    p1   = Samples[Samples_num / 100];
    p5   = Samples[Samples_num / 20];
    p10  = Samples[Samples_num / 10];
    p25  = Samples[Samples_num / 4];
    p50  = Samples[Samples_num / 2];
    p75  = Samples[Samples_num - Samples_num / 4];
    p90  = Samples[Samples_num - Samples_num / 10];
    p95  = Samples[Samples_num - Samples_num / 20];
    p99  = Samples[Samples_num - Samples_num / 100];
    p995 = Samples[Samples_num - Samples_num / 200];
    p999 = Samples[Samples_num - Samples_num / 1000];
    p100 = Samples[Samples_num - 1];
    
    printf("round-trip percentiles (ms) (slightly biased outwards):\n"
           "   0: %9.3f\n"
           " 0.1: %9.3f\n"
           " 0.5: %9.3f\n"
           "   1: %9.3f\n"
           "   5: %9.3f\n"
           "  10: %9.3f\n"
           "  25: %9.3f\n"
           "  50: %9.3f\n"
           "  75: %9.3f\n"
           "  90: %9.3f\n"
           "  95: %9.3f\n"
           "  99: %9.3f\n"
           "99.5: %9.3f\n"
           "99.9: %9.3f\n"
           " 100: %9.3f\n",
           p0 / 1000.0, p01 / 1000.0, p05 / 1000.0, p1 / 1000.0, p5 / 1000.0, p10 / 1000.0, p25 / 1000.0, p50 / 1000.0,
           p75 / 1000.0, p90 / 1000.0, p95 / 1000.0, p99 / 1000.0, p995 / 1000.0, p999 / 1000.0, p100 / 1000.0);
    
    for (i = 0; i < Samples_num; ++i)
    {
      double prev_mean = mean;
      
      mean += (Samples[i] - mean) / (i + 1);
      kvar += (Samples[i] - prev_mean) * (Samples[i] - mean);
    }

    stddev = (Samples_num > 1 ? sqrt(kvar / (Samples_num - 1)) : 0.0);

    printf("round-trip mean/stddev = %.3f/%.3f ms\n", mean / 1000.0, stddev / 1000.0);
  }
}

static void catch_signal(int signum)
{
  E_exit_events_async_safe();
}

static void send_ping(int dmy_code, void *dmy_data)
{
  sp_time   now = E_get_time();
  ping_pkt *pkt = &Send_mess.ping;

  if (Num_messages >= 0 && Send_counter >= Num_messages)
  {
    E_exit_events();
    return;
  }

  pkt->seq     = Send_counter++;
  pkt->secs    = (uint32_t) now.sec;
  pkt->usecs   = (uint32_t) now.usec;
  pkt->is_ping = 1;
  strncpy(pkt->origin, Private_group, MAX_GROUP_NAME);

  Ret = SP_multicast(Mbox, Send_service_type, Group, 0, Num_bytes, Send_mess.mess);

  if (Ret != Num_bytes)
  {
    if (Ret < 0)
      SP_error(Ret);
    
    else
      fprintf(stderr, "Bytes sent (%d) != Num_bytes (%d)?!\n", Ret, Num_bytes);
    
    exit(1);
  }

  E_queue(send_ping, 0, NULL, SP_period);
}

static void recv_from_spread(int dmy_fd, int dmy_code, void *dmy_data)
{
  ping_pkt *pkt = &Recv_mess.ping;
  
  Ret = SP_receive(Mbox, (Service_type = 0, &Service_type), Sender, SPPING_MAX_GROUPS, &Num_groups, Ret_groups, 
                   &Mess_type, &Endian_mismatch, sizeof(Recv_mess), Recv_mess.mess);
            
  if (Ret < 0)
    SP_error(Ret), exit(1);

  if (!Is_regular_mess(Service_type))
    return;
  
  if (pkt->is_ping)
  {
    int ret;
    
    /* recvd a ping: turn is_ping off and reflect it back to sender with same contents, size, service_type, etc. */
    
    pkt->is_ping = 0;
    
    ret = SP_multicast(Mbox, Service_type, Sender, 0, Ret, Recv_mess.mess);
    
    if (ret != Ret)
    {
      if (ret < 0)
        SP_error(ret);
      
      else
        fprintf(stderr, "Bytes sent (%d) != Ret (%d)?!\n", ret, Ret);
      
      exit(1);
    }
  }
  else if (!strncmp(pkt->origin, Private_group, MAX_GROUP_NAME))
  {
    sp_time now = E_get_time();
    long    us  = ((long) now.sec - (long) pkt->secs) * 1000000L + ((long) now.usec - (long) pkt->usecs);
    
    /* got a pong back from one of our pings */

    printf("%d bytes from %s: seq = %d time = %.3f ms\n", Ret, Sender, pkt->seq, us / 1000.0);

    if (Samples_num >= Samples_size)
    {
      long *new_samps;
      
      Samples_size = (Samples_size != 0 ? Samples_size * 2 : 256);
      
      if ((new_samps = realloc(Samples, sizeof(long) * Samples_size)) == NULL)
      {
        fprintf(stderr, "\nrealloc(%lu) failed!\n", (unsigned long) sizeof(long) * Samples_size);
        E_exit_events();
        return;
      }

      Samples = new_samps;
    }

    Samples[Samples_num++] = us;
  }
}

int main(int argc, char *argv[])
{
  signal(SIGINT, catch_signal);
  signal(SIGTERM, catch_signal);
  
  E_init();

  Usage(argc, argv);

  printf("spping: connecting to '%s'\n", Spread_name);
  
  if ((Ret = SP_connect(Spread_name, User, 0, 1, &Mbox, Private_group)) < 0) 
    SP_error(Ret), exit(1);

  printf("spping: connected as '%s'\n", Private_group);
  E_attach_fd(Mbox, READ_FD, recv_from_spread, 0, 0, HIGH_PRIORITY);
  
  if ((Ret = SP_join(Mbox, Group)) < 0)
    SP_error(Ret), exit(1);

  if (Read_only)
    printf("spping: only reflecting pings!\n");
  
  else
  {
    printf("PING %s: %d data bytes\n", Group, Num_bytes);
    send_ping(0, NULL);
  }
  
  E_handle_events();

  compute_print_stats();

  return 0;
}
