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
 *  Copyright (C) 1993-2016 Spread Concepts LLC <info@spreadconcepts.com>
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

#include <string.h>
#include <assert.h>
#include <errno.h>

#include "arch.h"
#include "spread_params.h"
#include "network.h"
#include "net_types.h"
#include "spu_data_link.h"
#include "spu_events.h"
#include "status.h"
#include "spu_alarm.h"
#include "configuration.h"
#include "membership.h"

/********************************************************************************
 ********************************************************************************/

static	channel		Bcast_channel[MAX_INTERFACES_PROC + MAX_ADDRS_SEGMENT];
static  channel		Token_channel[MAX_INTERFACES_PROC];
static	channel		Send_channel;

static  int             Num_bcast_channels;
static  int             Num_token_channels;

static	int		Bcast_needed = -1;
static  int             Bcast_addrs_num;
static	spu_addr	Bcast_addrs[MAX_ADDRS_SEGMENT];

static	int		Num_send_needed;
static  spu_addr        Send_addrs[MAX_SEGMENTS];

static	spu_addr	Token_addr;

static	configuration	Net_membership;
static	bool		Segment_leader;

static 	configuration  *Cn;
static	proc		My;

static	int16		Partition[MAX_PROCS_RING];
static	sp_time		Partition_timeout 	= { 60, 0};
static	int		Partition_my_index;

static	void		Clear_partition_cb(int dummy, void *dummy_p);
static	int		In_my_component( int32	proc_id );
static	void		Flip_pack( packet_header *pack_ptr );
static	void		Flip_token( token_header *token_ptr );

/********************************************************************************
 * Send scat to all Bcast_addrs through Send_channel.
 *
 * Returns minimum return value from each call to DL_sendto_gen.
 ********************************************************************************/

static int send_bcast( const sys_scatter *scat )
{
  int ret;
  int tmp;
  int i;

  if (Bcast_addrs_num < 1 || Bcast_addrs_num > MAX_ADDRS_SEGMENT)
    Alarmp(SPLOG_FATAL, NETWORK, "send_bcast: illegal Bcast_addrs_num (%d)!\n", Bcast_addrs_num);

  if ((ret = DL_sendto_gen(Send_channel, scat, &Bcast_addrs[0])) < 0)
    Alarmp(SPLOG_ERROR, NETWORK, "send_bcast: DL_sendto_gen(%s) #0 on channel %d failed with %d %d %s\n",
           SPU_ADDR_NTOP(&Bcast_addrs[0]), (int) Send_channel, ret, sock_errno, sock_strerror(sock_errno));
    
  for (i = 1; i < Bcast_addrs_num; ++i)
  {
    if ((tmp = DL_sendto_gen(Send_channel, scat, &Bcast_addrs[i])) < 0)
      Alarmp(SPLOG_ERROR, NETWORK, "send_bcast: DL_sendto_gen(%s) #%d on channel %d failed with %d %d %s\n",
             SPU_ADDR_NTOP(&Bcast_addrs[0]), i, (int) Send_channel, ret, sock_errno, sock_strerror(sock_errno));
        
    if (tmp != ret)
    {
      Alarmp(SPLOG_WARNING, NETWORK, "send_bcast: multiple DL_sendto_gen's had different return codes! %d != %d\n", tmp, ret);
      
      if (tmp < ret)
        ret = tmp;
    }
  }

  if (ALARMP_NEEDED(SPLOG_DEBUG, NETWORK))
    Alarmp(SPLOG_DEBUG, NETWORK, "send_bcast: ret = %d on channel %d\n", ret, (int) Send_channel);

  return ret;  /* return minimum value (e.g.: -1) from DL_sendto_gen's */
}

/********************************************************************************
 * Called from above when configuration file is reloaded.  Needs to
 * update any static variables that depend on current configuration.
 ********************************************************************************/

void Net_signal_conf_reload(void)
{
  int  old_bcast_needed = Bcast_needed;
  proc dummy_proc;
  int  i;

  Cn = Conf_ref();
  My = Conf_my();
  
  Partition_my_index = Conf_proc_by_id( My.id, &dummy_proc );
  Net_clear_partition();

  if (My.num_if > MAX_INTERFACES_PROC)
    Alarmp(SPLOG_FATAL, NETWORK, "BUG! Too many interfaces on my daemon!\n");
  
  if ((Bcast_addrs_num = Cn->segments[My.seg_index].num_seg_addrs) > MAX_ADDRS_SEGMENT)
    Alarmp(SPLOG_FATAL, NETWORK, "BUG! Too many segment addresses in my segment!\n");

  for (i = 0; i < Bcast_addrs_num; ++i)
    Bcast_addrs[i] = Cn->segments[My.seg_index].seg_addrs[i];
  
  if ((Bcast_needed = (Cn->segments[My.seg_index].num_procs > 1)))
    Alarmp(SPLOG_INFO, NETWORK, "Net_signal_conf_reload: Bcast needed\n");
  
  else
    Alarmp(SPLOG_INFO, NETWORK, "Net_signal_conf_reload: Bcast is not needed\n");

  if (old_bcast_needed != -1 && Bcast_needed && !old_bcast_needed)
    Alarmp(SPLOG_FATAL, NETWORK, "Net_signal_conf_reload: Bcast now needed when it wasn't originally; sockets need to be reestablished\n");
}

/********************************************************************************
 ********************************************************************************/

void    Net_init()
{
  spu_addr  if_addr = { 0 };
  int16u    if_port;
  int       ret;
  int       i, j;

  Net_signal_conf_reload();

  if_port = spu_addr_ip_get_port(&My.proc_addr);
        
  if (My.num_if < 1)
    Alarmp(SPLOG_FATAL, NETWORK, "Net_init: BUG! No interfaces?! %d\n", My.num_if);

  for (i = 0; i < Bcast_addrs_num; ++i)
    if (if_port != spu_addr_ip_get_port(&Bcast_addrs[i]))
      Alarmp(SPLOG_FATAL, NETWORK, "Net_init: BUG! Proc and segment port mismatch?!\n");

  /* NOTE: we broke out the special case of no interface spec given to make the logic easier to follow between the two cases */
  
  if (Is_IfType_Any(My.ifc[0].iftype))
  {
    /* no interface spec was given: bind to base port (and +1) on all interfaces (e.g. - INADDRY_ANY) */
    
    if (My.num_if != 1)
      Alarmp(SPLOG_FATAL, NETWORK, "Net_init: BUG! No interfaces were specified but we have more than the implicit one?!\n");

    if_addr = My.ifc[0].ifaddr;
    
    if (spu_addr_ip_cmp(&if_addr, &My.proc_addr, TRUE))
      Alarmp(SPLOG_FATAL, NETWORK, "Net_init: BUG! Implicit interface didn't match proc?!\n");

    spu_addr_ip_set_unspecified(&if_addr);

    if (spu_addr_family(&if_addr) == AF_INET6)
      if_addr.ipv6.sin6_scope_id = 0;
    
    Alarmp(SPLOG_INFO, NETWORK, "Net_init: implicit: binding bcast recv channel to [%s]:%u\n", SPU_ADDR_NTOP(&if_addr), (unsigned) spu_addr_ip_get_port(&if_addr));
    Bcast_channel[Num_bcast_channels] = DL_init_channel_gen(RECV_CHANNEL, NULL, &if_addr);

    for (i = 0; i < Bcast_addrs_num; ++i)
      if (spu_addr_ip_is_multicast(&Bcast_addrs[i]))
      {
        if ((ret = DL_join_multicast_gen(Bcast_channel[Num_bcast_channels], &Bcast_addrs[i], &My.proc_addr)))
          Alarmp(SPLOG_FATAL, NETWORK, "Net_init: implicit: error joining multicast group %s on channel %d: %d %s\n",
                 SPU_ADDR_NTOP(&Bcast_addrs[i]), (int) Bcast_channel[Num_bcast_channels], sock_errno, sock_strerror(sock_errno));

        Alarmp(SPLOG_INFO, NETWORK, "Net_init: implicit: joined multicast group %s on channel %d\n", SPU_ADDR_NTOP(&Bcast_addrs[i]), (int) Bcast_channel[Num_bcast_channels]);
      }

    ++Num_bcast_channels;
    spu_addr_ip_set_port(&if_addr, if_port + 1);

    Alarmp(SPLOG_INFO, NETWORK, "Net_init: implicit: binding token recv channel to [%s]:%u\n", SPU_ADDR_NTOP(&if_addr), (unsigned) spu_addr_ip_get_port(&if_addr));
    Token_channel[Num_token_channels] = DL_init_channel_gen(RECV_CHANNEL, NULL, &if_addr);
    ++Num_token_channels;
  }
  else
  {
    /* an interface specification was given */

    bool bcast_bound[MAX_ADDRS_SEGMENT] = { 0 };  /* have we already explicitly or implicitly bound to Bcast_addrs yet? */
    
    for (i = 0; i < My.num_if; ++i)
    {
      int reuse_addr_opt = 0;      /* if the user explicitly binds to one of Bcast_addrs or a multicast address, then do the bind with REUSE_ADDR */
      
      if (Is_IfType_Any(My.ifc[i].iftype))
        Alarmp(SPLOG_FATAL, NETWORK, "Net_init: BUG! An ANY interface type within an interface specification?!\n");

      if (!Is_IfType_Daemon(My.ifc[i].iftype))
        continue;

      if_addr = My.ifc[i].ifaddr;

      if (if_port != spu_addr_ip_get_port(&if_addr))
        Alarmp(SPLOG_FATAL, NETWORK, "Net_init: BUG! Interface port didn't match daemon port?!\n");

      /* if the user explicitly binds to the unspecified address (e.g. - INADDR_ANY) or to a Bcast_addr, then skip the extra bind(s) to the related Bcast_addrs below */
      
      if (spu_addr_ip_is_unspecified(&if_addr) && spu_addr_family(&if_addr) == spu_addr_family(&My.proc_addr))
        for (j = 0; j < Bcast_addrs_num; ++j)
          bcast_bound[j] = TRUE;

      else
        for (j = 0; j < Bcast_addrs_num; ++j)
          if (!spu_addr_ip_cmp(&if_addr, &Bcast_addrs[j], TRUE))
          {
            bcast_bound[j] = TRUE;
            reuse_addr_opt = REUSE_ADDR;
            break;
          }
      
      if (spu_addr_ip_is_multicast(&if_addr))
        reuse_addr_opt = REUSE_ADDR;

      Alarmp(SPLOG_INFO, NETWORK, "Net_init: explicit: binding a recv bcast channel to [%s]:%u\n", SPU_ADDR_NTOP(&if_addr), (unsigned) spu_addr_ip_get_port(&if_addr));
      Bcast_channel[Num_bcast_channels] = DL_init_channel_gen(RECV_CHANNEL | reuse_addr_opt, NULL, &if_addr);

      for (j = 0; j < Bcast_addrs_num; ++j)
        if (spu_addr_ip_is_multicast(&Bcast_addrs[j]))
        {
          if ((ret = DL_join_multicast_gen(Bcast_channel[Num_bcast_channels], &Bcast_addrs[j], &My.proc_addr)))
            Alarmp(SPLOG_FATAL, NETWORK, "Net_init: explicit: error joining multicast group %s on channel %d: %d %s\n",
                   SPU_ADDR_NTOP(&Bcast_addrs[j]), (int) Bcast_channel[Num_bcast_channels], sock_errno, sock_strerror(sock_errno));
          
          Alarmp(SPLOG_INFO, NETWORK, "Net_init: explicit: joined multicast group %s on channel %d\n", SPU_ADDR_NTOP(&Bcast_addrs[j]), (int) Bcast_channel[Num_bcast_channels]);
        }

      ++Num_bcast_channels;
      spu_addr_ip_set_port(&if_addr, if_port + 1);

      Alarmp(SPLOG_INFO, NETWORK, "Net_init: explicit: binding a recv token channel to [%s]:%u\n", SPU_ADDR_NTOP(&if_addr), (unsigned) spu_addr_ip_get_port(&if_addr));
      Token_channel[Num_token_channels] = DL_init_channel_gen(RECV_CHANNEL, NULL, &if_addr);
      ++Num_token_channels;
    }
    
    /* On *nix, when binding to specific interfaces, we also need to
     * bind to the segment addresses to receive multicast or broadcast
     * packets bc of bind's destination address matching + delivery.
     *
     * The above statement is not true for Windows.  Also, binding to
     * a broadcast address is forbidden there. So, we do not compile
     * in these extra binds on windows.
     *
     * TODO: Check that multicast and broadcast actually work as
     * desired on Windows when done this way.
     */

#ifndef ARCH_PC_WIN95
    /* NOTE: for backwards compatability with old configurations we don't do this for singleton segments */
    /* TODO: figure out a better way because not setting up this
     * socket here means that if a singleton segment is reconfigured
     * to have multiple daemons then this daemon needs to reboot */
    
    for (i = 0; Bcast_needed && i < Bcast_addrs_num; ++i)
      if (!bcast_bound[i])                 /* didn't already explicity or implicitly bind to Bcast_addrs[i] above */
      {
        if_addr = Bcast_addrs[i];
        
        Alarmp(SPLOG_INFO, NETWORK, "Net_init: explicit: binding an extra recv bcast channel to [%s]:%u\n", SPU_ADDR_NTOP(&if_addr), (unsigned) spu_addr_ip_get_port(&if_addr));
        Bcast_channel[Num_bcast_channels] = DL_init_channel_gen(RECV_CHANNEL | REUSE_ADDR, NULL, &if_addr);

        for (j = 0; j < Bcast_addrs_num; ++j)
          if (spu_addr_ip_is_multicast(&Bcast_addrs[j]))
          {
            if ((ret = DL_join_multicast_gen(Bcast_channel[Num_bcast_channels], &Bcast_addrs[j], &My.proc_addr)))
              Alarmp(SPLOG_FATAL, NETWORK, "Net_init: explicit: error joining multicast group %s on channel %d: %d %s\n",
                     SPU_ADDR_NTOP(&Bcast_addrs[j]), (int) Bcast_channel[Num_bcast_channels], sock_errno, sock_strerror(sock_errno));
            
            Alarmp(SPLOG_INFO, NETWORK, "Net_init: explicit: joined multicast group %s on channel %d\n", SPU_ADDR_NTOP(&Bcast_addrs[j]), (int) Bcast_channel[Num_bcast_channels]);
          }
      
        ++Num_bcast_channels;
      }
#endif    
  }

  /* NOTE: currently we don't use NO_LOOP bc another daemon in our segment might be on the same machine but bound to a different interface */
  /* NOTE: we also use RECV_CHANNEL bc we want to bind to My.proc_addr so that our source address is My.proc_addr when our messages are received */

  if_addr = My.proc_addr;
  spu_addr_ip_set_port(&if_addr, 0);
  
  Alarmp(SPLOG_INFO, NETWORK, "Net_init: creating send channel bound to %s\n", SPU_ADDR_NTOP(&if_addr));
  Send_channel = DL_init_channel_gen(SEND_CHANNEL | RECV_CHANNEL, NULL, &if_addr);
}

/********************************************************************************
 ********************************************************************************/

void	Net_set_membership( configuration memb )
{
	int	i;
	int	my_index_in_seg;
	int	my_next_index;
	segment my_seg;

	Net_membership = memb;
	my_seg = Net_membership.segments[My.seg_index];
	my_index_in_seg = Conf_id_in_seg( &my_seg, My.id );

	if ( my_index_in_seg < 0 )
        {
		Conf_print( &Net_membership );
		Alarm( EXIT, "Net_set_membership: I am not in membership\n" );
	}
        
	if ( my_index_in_seg == 0)
        {
		Segment_leader = 1;
		Alarm( NETWORK,"Net_set_membership: I am a Segment leader\n");
	}
        else
                Segment_leader = 0;

	Num_send_needed = 0;
	my_next_index = -1;
        
	for( i=0; i < Conf_num_segments( Cn ); i++ )
	{
	    if( i == My.seg_index )
	    {
		/* 
		 * This is my segment.
		 * There is no need to send to it
		 * but I need my_next_index to calculate
		 * the token send address.
		 */
		my_next_index = Num_send_needed;
	    }
            else if( Net_membership.segments[i].num_procs > 0 )
            {
		Send_addrs[Num_send_needed] = Net_membership.segments[i].procs[0]->proc_addr;
		Num_send_needed++;
	    }
	}
        
	assert(my_next_index != -1);
        
	for( i=0; i < Num_send_needed; i++ )
          Alarm( NETWORK, "Net_set_membership: Send_addrs[%d] = [%s]:%u\n", i, SPU_ADDR_NTOP(&Send_addrs[i]), (unsigned) spu_addr_ip_get_port(&Send_addrs[i]));

	/* Calculate where to send the token */

	if ( my_index_in_seg < my_seg.num_procs - 1 )                 /* I am not last in my segment */
          Token_addr = my_seg.procs[my_index_in_seg + 1]->proc_addr;  /* send to next member in my segment */

	else if ( Num_send_needed == 0 )                              /* My segment is the only segment */
          Token_addr = my_seg.procs[0]->proc_addr;                    /* send to first member in my segment */
        
        else if ( Num_send_needed == my_next_index )                  /* My segment is the last segment */
          Token_addr = Send_addrs[0];                                 /* send to first in first valid segment */

        else                                                          /* There is a valid segment after mine */
          Token_addr = Send_addrs[my_next_index];                     /* send to first in next valid segment */

        spu_addr_ip_set_port(&Token_addr, spu_addr_ip_get_port(&Token_addr) + 1);
        
	Alarm(NETWORK, "Net_set_membership: Token_addr = [%s]:%u\n", SPU_ADDR_NTOP(&Token_addr), (unsigned) spu_addr_ip_get_port(&Token_addr));
}

/********************************************************************************
 ********************************************************************************/

int	Net_bcast( sys_scatter *scat )
{
	packet_header	*pack_ptr;
	int 		i;
	int		ret = 0;

	/* routing on channels if needed according to membership */
        
	pack_ptr                = (packet_header*) scat->elements[0].buf;
	pack_ptr->type          = Set_routed( pack_ptr->type );
	pack_ptr->type          = Set_endian( pack_ptr->type );
        pack_ptr->conf_hash     = Cn->hash_code;
	pack_ptr->transmiter_id = My.id;
        
	for ( i = 0; i < Num_send_needed; ++i )
          ret = DL_sendto_gen( Send_channel, scat, &Send_addrs[i] );

	pack_ptr->type = Clear_routed( pack_ptr->type );

	/* broadcasting if needed according to configuration */
        
	if( Bcast_needed )
          ret = send_bcast( scat );

        else if ( Num_send_needed == 0 )
          ret = 1; /* No actual send is needed, but 'packet' can be considered 'sent' */

	return( ret );
}

/********************************************************************************
 ********************************************************************************/

int	Net_scast( int16 seg_index, sys_scatter *scat )
{
	packet_header	*pack_ptr;
	int		ret               = 0;
        bool            send_not_needed_p = FALSE;

	pack_ptr                = (packet_header*) scat->elements[0].buf;
	pack_ptr->type          = Set_endian( pack_ptr->type );
        pack_ptr->conf_hash     = Cn->hash_code;
	pack_ptr->transmiter_id = My.id;
        
	if ( seg_index == My.seg_index )
	{
	    if( Bcast_needed )
                ret = send_bcast( scat );
            
	    else 
                send_not_needed_p = TRUE;
	}
        else
        {
	    if ( Net_membership.segments[seg_index].num_procs > 0 )
	    {
		pack_ptr->type = Set_routed( pack_ptr->type );
	    	ret            = DL_sendto_gen( Send_channel, scat, &Net_membership.segments[seg_index].procs[0]->proc_addr );
		pack_ptr->type = Clear_routed( pack_ptr->type );
	    }
            else
                send_not_needed_p = TRUE;
	}

        if (send_not_needed_p)
            ret = 1; /* notify that packet can be considered sent, even though no network send actually needed */

	return( ret );
}

/********************************************************************************
 ********************************************************************************/

int	Net_ucast( int32 proc_id, sys_scatter *scat )
{
	packet_header	*pack_ptr;
	proc		p;

	pack_ptr                = (packet_header*) scat->elements[0].buf;
	pack_ptr->type          = Set_endian( pack_ptr->type );
        pack_ptr->conf_hash     = Cn->hash_code;
	pack_ptr->transmiter_id = My.id;
        
	if( Conf_proc_by_id( proc_id, &p ) < 0 )
	{
		Alarm( PRINT, "Net_ucast: non existing proc_id %d\n",proc_id );
		return( -1 );
	}
        
	return DL_sendto_gen( Send_channel, scat, &p.proc_addr );
}

/********************************************************************************
 ********************************************************************************/

static int Net_handle_monitor(sys_scatter *scat, spu_addr src_addr)
{
  packet_header *pack_ptr    = (packet_header*) scat->elements[0].buf;
  size_t         recvd_bytes = sizeof(packet_header) + pack_ptr->data_len;
  int16         *cur_partition;
  int            i;

  if (!Is_partition(pack_ptr->type))  /* hand other control types back up to protocol layer */
    return (int) recvd_bytes;  
  
  if (!Conf_get_dangerous_monitor_state())
  {
    Alarmp(SPLOG_PRINT, NETWORK, "Net_handle_monitor: Request to set partition or kill daemons from [%s]:%u denied. Daemon in safe mode!\n",
           SPU_ADDR_NTOP(&src_addr), (unsigned) spu_addr_ip_get_port(&src_addr));
    return 0;
  }

  if (scat->num_elements <= 1 ||
      recvd_bytes < sizeof(packet_header) + Conf_num_procs(Cn) * sizeof(int16) ||
      scat->elements[1].len < Conf_num_procs(Cn) * sizeof(int16))
  {
    Alarmp(SPLOG_WARNING, NETWORK, "Net_handle_monitor: received monitor partition packet from [%s]:%u; proc_id = %s; msg too small! Ignoring!\n",
           SPU_ADDR_NTOP(&src_addr), (unsigned) spu_addr_ip_get_port(&src_addr), CONF_ID_TO_STR(pack_ptr->proc_id));
    return 0;
  }

  cur_partition = (int16*) scat->elements[1].buf;
                
  for (i = 0; i < Conf_num_procs(Cn); ++i)
    if (!Same_endian(pack_ptr->type))
      cur_partition[i] = Flip_int16(cur_partition[i]);

  Net_set_partition(cur_partition);

  E_queue(Clear_partition_cb, 0, NULL, Partition_timeout);
                
  if (Partition[Partition_my_index] == -1)
    Alarmp( SPLOG_FATAL, NETWORK, "Net_handle_monitor: Instructed to exit by monitor!\n");
                
  Alarmp(SPLOG_PRINT, NETWORK, "Net_handle_monitor: Got monitor partition message; I'm in component %d\n", (int) Partition[Partition_my_index]);

  return 0;
}

/********************************************************************************
 ********************************************************************************/

int	Net_recv ( channel fd, sys_scatter *scat )
{
	packet_header  *pack_ptr = (packet_header*) scat->elements[0].buf;
        spu_addr        src_addr = { 0 };
	int		received_bytes;
        proc           *pp;
        size_t          i;

        if (scat->num_elements == 0 || scat->elements[0].len < sizeof(packet_header))
          Alarmp(SPLOG_FATAL, NETWORK, "Net_recv: BUG! Scatter is too small for packet header!\n");
        
        for (i = 0; (int) i < Num_bcast_channels && fd != Bcast_channel[i]; ++i);

        if ((int) i == Num_bcast_channels)
          Alarmp(SPLOG_FATAL, NETWORK, "Net_recv: Listening and received packet on non-bcast channel %d\n", fd);

	received_bytes = DL_recvfrom_gen( fd, scat, &src_addr ); 

	if ( received_bytes < 0 )
        {
          Alarmp( SPLOG_ERROR, NETWORK, "Net_recv: error: %d %d '%s' receiving on channel %d\n", received_bytes, sock_errno, sock_strerror(sock_errno), fd);
          return( received_bytes );
        }

	if ( (size_t) received_bytes < sizeof( packet_header ) ) 
	{
          Alarmp( SPLOG_WARNING, NETWORK, "Net_recv: ignoring msg of size %d, smaller than packet header size %lu from [%s]:%u on channel %d\n", 
                  received_bytes, (unsigned long) sizeof(packet_header), SPU_ADDR_NTOP(&src_addr), (unsigned) spu_addr_ip_get_port(&src_addr), fd );
          return( 0 );
	}

	if ( !Same_endian( pack_ptr->type ) )
          Flip_pack( pack_ptr );

	if ( (size_t) received_bytes != sizeof( packet_header ) + pack_ptr->data_len)
        {
          Alarmp( SPLOG_WARNING, NETWORK, "Net_recv: Received invalid msg: received bytes (%d) != expected length (%lu)\n", 
                  received_bytes, (unsigned long) (sizeof( packet_header ) + pack_ptr->data_len) );
          return( 0 );
        }

        if ( pack_ptr->conf_hash != Cn->hash_code )
        {
          Alarmp( SPLOG_WARNING, NETWORK, "Net_recv: ignoring msg from different spread configuration; hash (%u) != local hash (%u); from [%s]:%u on channel %d\n",
                  (unsigned) pack_ptr->conf_hash, (unsigned) Cn->hash_code, SPU_ADDR_NTOP(&src_addr), (unsigned) spu_addr_ip_get_port(&src_addr), fd);
          return( 0 );
        }

        if ( Conf_proc_ref_by_id_in_conf( Cn, pack_ptr->transmiter_id, &pp ) < 0 )
        {
          Alarmp( SPLOG_INFO, NETWORK, "Net_recv: ignoring msg from transmitter (0x%08X) bc not in my configuration; from [%s]:%u on channel %d\n",
                  (unsigned) pack_ptr->transmiter_id, SPU_ADDR_NTOP(&src_addr), (unsigned) spu_addr_ip_get_port(&src_addr), fd);
          return( 0 );
        }

        if ( spu_addr_ip_cmp(&src_addr, &pp->proc_addr, FALSE) )
        {
          Alarmp( SPLOG_WARNING, NETWORK, "Net_recv: ignoring msg from transmitter (0x%08X) bc src_addr doesn't match my configuration; from [%s]:%u on channel %d\n",
                  (unsigned) pack_ptr->transmiter_id, SPU_ADDR_NTOP(&src_addr), (unsigned) spu_addr_ip_get_port(&src_addr), fd );
          return( 0 );
        }
        
        if ( Is_control( pack_ptr->type ) )                 /* handle monitor messages (which can have (transmiter_id, proc_id) == My.id) */
        {
          Alarmp( SPLOG_INFO, NETWORK, "Net_recv: got monitor control msg of type 0x%08X from [%s]:%u on channel %d\n",
                  (unsigned) pack_ptr->type, SPU_ADDR_NTOP(&src_addr), (unsigned) spu_addr_ip_get_port(&src_addr), fd);
          return Net_handle_monitor(scat, src_addr);
        }
          
	if ( pack_ptr->transmiter_id == My.id )             /* no need to return my own msgs */
        {
          if ( ALARMP_NEEDED( SPLOG_DEBUG, NETWORK ) ) Alarmp( SPLOG_DEBUG, NETWORK, "Net_recv: ignoring msg from myself.\n" );
          return( 0 );
        }
        
	if ( !In_my_component( pack_ptr->transmiter_id ) )  /* drop msgs from daemons in different monitor-caused partition */
        {
          if ( ALARMP_NEEDED( SPLOG_DEBUG, NETWORK ) ) Alarmp( SPLOG_DEBUG, NETWORK, "Net_recv: ignoring msg from transmitter (0x%08X) not in my component!\n", (unsigned) pack_ptr->transmiter_id );
          return( 0 );
        }

	if ( Is_routed( pack_ptr->type ) && Bcast_needed )           /* if requested to send to our segment and we need to do so */
	{
                size_t old_num_elements = scat->num_elements;        /* save so we can restore */
                size_t old_len;
                size_t num_bytes;
          
		if ( !Segment_leader )
                  Alarmp( SPLOG_WARNING, NETWORK, "Net_recv: routed msg from 0x%08X but I'm not seg leader?! Sending to segment anyway.\n", (unsigned) pack_ptr->proc_id );

		/* truncate scat for sending: find the truncation point based on received_bytes; NOTE: search overshoots by one element */
                
                for (i = 0, num_bytes = 0; i < scat->num_elements && num_bytes < (size_t) received_bytes; num_bytes += scat->elements[i++].len);

                scat->num_elements = i;
                assert(scat->num_elements >= 1 && scat->num_elements <= old_num_elements);
                
                num_bytes -= scat->elements[--i].len;                /* undo last loop iteration */
                assert(num_bytes < (size_t) received_bytes);
                
                old_len               = scat->elements[i].len;       /* save so we can restore */
                scat->elements[i].len = (size_t) received_bytes - num_bytes;
                assert(scat->elements[i].len <= old_len);
                
		pack_ptr->type = Clear_routed( pack_ptr->type );
		pack_ptr->transmiter_id = My.id;

		/* flip header to original form; NOTE: we do this rather than set our own endianness because body contents have original sender's endianness */
                
		if ( !Same_endian( pack_ptr->type ) )
                  Flip_pack( pack_ptr );
                
                send_bcast( scat );
                
		/* re-fliping to my form */
                
		if ( !Same_endian( pack_ptr->type ) )
                  Flip_pack( pack_ptr );

		/* restore scat */

                scat->num_elements    = old_num_elements;
                scat->elements[i].len = old_len;
	}

	pack_ptr->type = Clear_routed( pack_ptr->type );  /* any necessary routing already handled */

        if ( ALARMP_NEEDED( SPLOG_DEBUG, NETWORK ) )
          Alarmp( SPLOG_DEBUG, NETWORK, 
                  "Net_recv: type = 0x%08X; transmiter_id = 0x%08X; proc_id = 0x%08X; "
                  "memb_id = { proc_id = 0x%08X, time = %ld }; "
                  "seq = %ld; token_round = %ld; conf_hash = %u; data_len = %u; padding = %d; "
                  "first_frag_header = { fragment_index = %d, fragment_len = %d }; "
                  "src_addr = [%s]:%u; received_bytes = %d\n", 
                  (unsigned) pack_ptr->type, (unsigned) pack_ptr->transmiter_id, (unsigned) pack_ptr->proc_id,
                  (unsigned) pack_ptr->memb_id.proc_id, (long) pack_ptr->memb_id.time, 
                  (long) pack_ptr->seq, (long) pack_ptr->token_round, (unsigned) pack_ptr->conf_hash, (unsigned) pack_ptr->data_len, (int) pack_ptr->padding,
                  (int) pack_ptr->first_frag_header.fragment_index, (int) pack_ptr->first_frag_header.fragment_len,
                  SPU_ADDR_NTOP(&src_addr), (unsigned) spu_addr_ip_get_port(&src_addr), received_bytes );
        
	return( received_bytes );
}

/********************************************************************************
 ********************************************************************************/

int	Net_send_token( sys_scatter *scat )
{
	token_header *token_ptr = (token_header *)scat->elements[0].buf;
        size_t        send_len  = 0;
        size_t        i;

        for ( i = 0; i < scat->num_elements; ++i )
                send_len += scat->elements[i].len;

        if ( send_len != sizeof( token_header ) + token_ptr->rtr_len )
        {
                Alarmp( SPLOG_FATAL, NETWORK, "Net_send_token: Wrong size token %lu (send_len) != %lu (expected)!\n",
                        (unsigned long) send_len, (unsigned long) (sizeof( token_header) + token_ptr->rtr_len) );
        }
        else if ( send_len > sizeof( token_header) + sizeof( token_body ) )
        {
                Alarmp( SPLOG_FATAL, NETWORK, "Net_send_token: Token too long (%lu > %lu bytes)!\n",
                        (unsigned long) send_len, (unsigned long) ( sizeof( token_header) + sizeof( token_body ) ) );
        }
        else if ( send_len > MAX_PACKET_SIZE )
        {
                Alarmp( SPLOG_WARNING, PRINT, "Net_send_token: WARNING!!! Token is longer (%lu bytes) than a single fast ethernet MTU (%lu bytes)! "
                        "IP fragmentation likely to occur and can greatly increase the chance the token is lost!\n",
                        (unsigned long) send_len, (unsigned long) MAX_PACKET_SIZE );
        }

	token_ptr->type          = Set_endian( token_ptr->type );
        token_ptr->conf_hash     = Cn->hash_code;
	token_ptr->transmiter_id = My.id;

        if (ALARMP_NEEDED(SPLOG_DEBUG, NETWORK))
          Alarmp( SPLOG_DEBUG, NETWORK, 
                  "Net_send_token: type = 0x%08X; transmitter_id = 0x%08X; seq = %d; proc_id = 0x%08X; "
                  "aru = %d; aru_last_id = 0x%08X; Token_addr = [%s]:%u; send_len = %lu\n", 
                  (unsigned) token_ptr->type, (unsigned) token_ptr->transmiter_id, (unsigned) token_ptr->seq, (unsigned) token_ptr->proc_id,
                  (int) token_ptr->aru, (unsigned) token_ptr->aru_last_id,
                  SPU_ADDR_NTOP(&Token_addr), (unsigned) spu_addr_ip_get_port(&Token_addr), (unsigned long) send_len );

        return ( DL_sendto_gen( Send_channel, scat, &Token_addr ) );
}

/********************************************************************************
 ********************************************************************************/

int	Net_recv_token( channel fd, sys_scatter *scat )
{
	token_header   *token_ptr = (token_header*) scat->elements[0].buf;
        spu_addr        src_addr  = { 0 };
        proc           *pp;
	int		received_bytes;
        size_t          i;

        if (scat->num_elements <= 0 || scat->elements[0].len < sizeof(token_header))
          Alarmp(SPLOG_FATAL, NETWORK, "Net_recv_token: BUG! Scatter is too small for token header!\n");
        
        for (i = 0; i < Num_token_channels && fd != Token_channel[i]; ++i);

        if (i == Num_token_channels)
          Alarm(EXIT, "Net_recv_token: Listening and received packet on non-token channel %d\n", fd);

	received_bytes = DL_recvfrom_gen( fd, scat, &src_addr );

	if( received_bytes < 0 )
        {
          Alarmp( SPLOG_ERROR, NETWORK, "Net_recv_token: error: %d %d '%s' receiving on channel %d\n", received_bytes, sock_errno, sock_strerror(sock_errno), fd);
          return( received_bytes );
        }

        if ( (size_t) received_bytes < sizeof( token_header ) )
        {
          Alarmp( SPLOG_INFO, NETWORK, "Net_recv_token: ignoring token of size %d, smaller than token header size %lu from [%s]:%u on channel %d\n", 
                  received_bytes, (unsigned long) sizeof(token_header), SPU_ADDR_NTOP(&src_addr), (unsigned) spu_addr_ip_get_port(&src_addr), fd);
          return( 0 );
        }

	if( !Same_endian( token_ptr->type ) )
          Flip_token( token_ptr );

        /* TODO: recv size integrity check?

	if ( (size_t) received_bytes != sizeof( token_header ) + token_ptr->rtr_len)
        {
          Alarmp( SPLOG_INFO, NETWORK, "Net_recv_token: Received invalid token: received bytes (%d) != expected length (%lu)\n", 
                  received_bytes, (unsigned long) (sizeof( token_header ) + token_ptr->rtr_len) );
          return( 0 );
        }
        */
        
        if (token_ptr->conf_hash != Cn->hash_code)
        {
          Alarmp( SPLOG_INFO, NETWORK, "Net_recv_token: ignoring token from different spread configuration; hash (%u) != local hash (%u); from [%s]:%u on channel %d\n",
                  (unsigned) token_ptr->conf_hash, (unsigned) Cn->hash_code, SPU_ADDR_NTOP(&src_addr), (unsigned) spu_addr_ip_get_port(&src_addr), fd);
          return( 0 );
        }

        if ( Conf_proc_ref_by_id_in_conf( Cn, token_ptr->transmiter_id, &pp ) < 0 )
        {
          Alarmp( SPLOG_WARNING, NETWORK, "Net_recv_token: ignoring token from transmitter (0x%08X) bc not in my configuration; from [%s]:%u on channel %d\n",
                  (unsigned) token_ptr->transmiter_id, SPU_ADDR_NTOP(&src_addr), (unsigned) spu_addr_ip_get_port(&src_addr), fd);
          return( 0 );
        }

        if ( spu_addr_ip_cmp(&src_addr, &pp->proc_addr, FALSE) )
        {
          Alarmp( SPLOG_WARNING, NETWORK, "Net_recv_token: ignoring token from transmitter (0x%08X) bc src_addr doesn't match my configuration; from [%s]:%u on channel %d\n",
                  (unsigned) token_ptr->transmiter_id, SPU_ADDR_NTOP(&src_addr), (unsigned) spu_addr_ip_get_port(&src_addr), fd );
          return( 0 );
        }

	/* Monitor : drop token from daemon in different monitor-caused partition */
        
	if ( !In_my_component( token_ptr->transmiter_id ) )
        {
          Alarmp( SPLOG_INFO, NETWORK, "Net_recv_token: ignoring token from transmitter (0x%08X) not in my component!\n", (unsigned) token_ptr->transmiter_id );
          return( 0 );
        }

        if ( ALARMP_NEEDED( SPLOG_DEBUG, NETWORK ) )
            Alarmp( SPLOG_DEBUG, NETWORK, 
                    "Net_recv_token: type = 0x%08X; transmiter_id = 0x%08X; proc_id = 0x%08X; "
                    "memb_id = { proc_id = 0x%08X, time = %ld }; "
                    "seq = %ld; aru = %ld; aru_last_id = 0x%08X; "
                    "flow_control = %d; rtr_len = %u; conf_hash = %u; "
                    "src_addr = [%s]:%u; received_bytes = %d\n", 
                    (unsigned) token_ptr->type, (unsigned) token_ptr->transmiter_id, (unsigned) token_ptr->proc_id,
                    (unsigned) token_ptr->memb_id.proc_id, (long) token_ptr->memb_id.time,                    
                    (long) token_ptr->seq, (long) token_ptr->aru, (unsigned) token_ptr->aru_last_id,
                    (int) token_ptr->flow_control, (unsigned) token_ptr->rtr_len, (unsigned) token_ptr->conf_hash,
                    SPU_ADDR_NTOP(&src_addr), (unsigned) spu_addr_ip_get_port(&src_addr), received_bytes );

	return ( received_bytes );
}

/********************************************************************************
 ********************************************************************************/

int	Net_ucast_token( int32 proc_id, sys_scatter *scat )
{
	token_header *token_ptr = (token_header*) scat->elements[0].buf;
        size_t        send_len  = 0;
	proc	      p;
        size_t        i;

        for ( i = 0; i < scat->num_elements; ++i )
                send_len += scat->elements[i].len;

        if ( send_len != sizeof( token_header ) + token_ptr->rtr_len )
        {
                Alarmp( SPLOG_FATAL, NETWORK, "Net_ucast_token: Wrong size token %lu (send_len) != %lu (expected)!\n",
                        (unsigned long) send_len, (unsigned long) ( sizeof( token_header ) + token_ptr->rtr_len ) );
        }
        else if ( send_len > sizeof( token_header ) + sizeof( token_body ) )
        {
                Alarmp( SPLOG_FATAL, NETWORK, "Net_ucast_token: Token too long (%lu > %lu bytes)!\n",
                        (unsigned long) send_len, (unsigned long) ( sizeof( token_header ) + sizeof( token_body ) ) );
        }
        else if ( send_len > MAX_PACKET_SIZE )
        {
                Alarmp( SPLOG_WARNING, PRINT, "Net_ucast_token: WARNING!!! Token is longer (%lu bytes) than a single fast ethernet MTU (%lu bytes)! "
                        "IP fragmentation will likely occur and greatly increase the chance the token is lost!\n",
                        (unsigned long) send_len, (unsigned long) MAX_PACKET_SIZE );
        }

	token_ptr->type          = Set_endian( token_ptr->type );
        token_ptr->conf_hash     = Cn->hash_code;
	token_ptr->transmiter_id = My.id;

	if ( Conf_proc_by_id( proc_id, &p ) < 0 )
	{
		Alarm( PRINT, "Net_ucast_token: non existing proc_id %d\n", proc_id );
		return( -1 );
	}

        spu_addr_ip_set_port(&p.proc_addr, spu_addr_ip_get_port(&p.proc_addr) + 1);
        
        if (ALARMP_NEEDED(SPLOG_DEBUG, NETWORK))
            Alarmp( SPLOG_DEBUG, NETWORK, 
                    "Net_ucast_token: type = 0x%08X; transmitter_id = 0x%08X; seq = %d; proc_id = 0x%08X; "
                    "aru = %d; aru_last_id = 0x%08X; dst_addr = [%s]:%u; send_len = %lu\n", 
                    (unsigned) token_ptr->type, (unsigned) token_ptr->transmiter_id, (int) token_ptr->seq, (unsigned) token_ptr->proc_id, 
                    (int) token_ptr->aru, (unsigned) token_ptr->aru_last_id,
                    SPU_ADDR_NTOP(&p.proc_addr), (unsigned) spu_addr_ip_get_port(&p.proc_addr), (unsigned long) send_len );

        return( DL_sendto_gen( Send_channel, scat, &p.proc_addr ) );
}

/********************************************************************************
 ********************************************************************************/

void     Net_num_channels(int *num_bcast, int *num_token)
{
    *num_bcast = Num_bcast_channels;
    *num_token = Num_token_channels;
}

/********************************************************************************
 ********************************************************************************/

channel *Net_bcast_channel()
{
	return( &(Bcast_channel[0]) );
}

/********************************************************************************
 ********************************************************************************/

channel *Net_token_channel()
{
	return( &(Token_channel[0]) );
}

/********************************************************************************
 ********************************************************************************/

void    Net_set_partition(int16 *new_partition)
{
        int     i;

        if ( Conf_in_reload_singleton_state() )
        {
                Alarmp(SPLOG_WARNING, NETWORK, "Net_set_partition: Can not change partition since daemon configuration change in progress\n");
                return;
        }

        for( i=0; i < Conf_num_procs( Cn ); i++ )
                Partition[i] = new_partition[i];
}

/********************************************************************************
 ********************************************************************************/

void    Net_clear_partition(void)
{
	int	i;

	for( i=0; i < Conf_num_procs( Cn ); i++ )
		Partition[i] = 0;
}

/********************************************************************************
 ********************************************************************************/

static	void	Clear_partition_cb(int dummy, void *dummy_p)
{
        Net_clear_partition();
}

/********************************************************************************
 ********************************************************************************/

static	int	In_my_component( int32	proc_id )
{
	int	proc_index;
	proc	dummy_proc;
	char	ip[MAX_ID_SIZE];

	proc_index = Conf_proc_by_id( proc_id, &dummy_proc );
        
	if ( proc_index < 0 )
	{
		Alarmp( SPLOG_PRINT, NETWORK, "In_my_component: unknown proc %s\n", Conf_id_to_str( proc_id, ip ) );
		return( 0 );
	}

	return( Partition[Partition_my_index] == Partition[proc_index] );
}

/********************************************************************************
 * The first fragment header is not flipped here, even though it is
 * part of the packet header. It will be flipped with the other
 * fragment headers in Prot_handle_bcast.
 ********************************************************************************/

void	Flip_pack( packet_header *pack_ptr )
{
	pack_ptr->type		  = Flip_int32( pack_ptr->type );
	pack_ptr->transmiter_id	  = Flip_int32( pack_ptr->transmiter_id );
	pack_ptr->proc_id	  = Flip_int32( pack_ptr->proc_id );
	pack_ptr->memb_id.proc_id = Flip_int32( pack_ptr->memb_id.proc_id );
	pack_ptr->memb_id.time	  = Flip_int32( pack_ptr->memb_id.time );
	pack_ptr->seq		  = Flip_int32( pack_ptr->seq );
	pack_ptr->token_round	  = Flip_int32( pack_ptr->token_round );
	pack_ptr->conf_hash	  = Flip_int32( pack_ptr->conf_hash );
	pack_ptr->data_len	  = Flip_int16( pack_ptr->data_len );
}

/********************************************************************************
 ********************************************************************************/

void	Flip_token( token_header *token_ptr )
{
	token_ptr->type		   = Flip_int32( token_ptr->type );
	token_ptr->transmiter_id   = Flip_int32( token_ptr->transmiter_id );
	token_ptr->proc_id	   = Flip_int32( token_ptr->proc_id );
	token_ptr->memb_id.proc_id = Flip_int32( token_ptr->memb_id.proc_id );
	token_ptr->memb_id.time	   = Flip_int32( token_ptr->memb_id.time );
	token_ptr->seq		   = Flip_int32( token_ptr->seq );
	token_ptr->aru		   = Flip_int32( token_ptr->aru );
	token_ptr->aru_last_id	   = Flip_int32( token_ptr->aru_last_id );
	token_ptr->flow_control	   = Flip_int16( token_ptr->flow_control );
	token_ptr->rtr_len	   = Flip_int16( token_ptr->rtr_len );
        token_ptr->conf_hash       = Flip_int32( token_ptr->conf_hash );
}

/********************************************************************************
 ********************************************************************************/
