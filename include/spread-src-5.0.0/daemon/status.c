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


#define status_ext
#include <stdio.h>

#include "arch.h"
#include "spread_params.h"
#include "spu_scatter.h"
#include "net_types.h"
#include "spu_data_link.h"
#include "configuration.h"
#include "status.h"
#include "spu_events.h"
#include "spu_alarm.h"

static	sp_time		Start_time;
static	channel		Report_channel;
static	sys_scatter	Report_scat;
static	packet_header	Pack;

void	Stat_init( void )
{
        proc my          = Conf_my();
        spu_addr if_addr = my.proc_addr;
        
	Start_time = E_get_time();

        spu_addr_ip_set_port(&if_addr, 0);
	Report_channel = DL_init_channel_gen( SEND_CHANNEL | RECV_CHANNEL, NULL, &if_addr );  /* maybe just use Send_channel instead? */

	Report_scat.num_elements    = 2;
	Report_scat.elements[0].len = sizeof( packet_header );
	Report_scat.elements[0].buf = (char*) &Pack;
	Report_scat.elements[1].len = sizeof( status );
	Report_scat.elements[1].buf = (char*) &GlobalStatus;

	Pack.type     = Set_endian( STATUS_TYPE );
	Pack.proc_id  = my.id;
	Pack.data_len = sizeof( status );

	GlobalStatus.major_version = SP_MAJOR_VERSION;
	GlobalStatus.minor_version = SP_MINOR_VERSION;
	GlobalStatus.patch_version = SP_PATCH_VERSION;

	Alarm( STATUS, "Stat_init: went ok\n" );
}

void	Stat_handle_message( sys_scatter *scat )
{
        spu_addr        dst_addr = { 0 };
	sp_time		now;
	sp_time		delta;
	packet_header	*pack_ptr;
	proc		p;

	pack_ptr = (packet_header*) scat->elements[0].buf;
        
	if ( Conf_proc_by_id( pack_ptr->proc_id, &p ) < 0 )
	{
		Alarm( STATUS, "Stat_handle_message: Illegal monitor request\n");
		return;
	}

        dst_addr = p.proc_addr;
        spu_addr_ip_set_port(&dst_addr, (int16u) pack_ptr->seq);  /* NOTE: monitor return port stuffed into seq */

	now   = E_get_time();
	delta = E_sub_time( now, Start_time );
	GlobalStatus.sec = delta.sec;

	DL_sendto_gen( Report_channel, &Report_scat, &dst_addr );
}

