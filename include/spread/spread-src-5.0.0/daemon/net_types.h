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


#ifndef	INC_NET_TYPES
#define	INC_NET_TYPES

#include "arch.h"
#include "spu_data_link.h"

/* Dont forget that 0x80000080 is kept for endians */
/* NOTE: protocol.c depends on UNRELIABLE_TYPE < ... < SAFE_TYPE < BLOCK_REGULAR_DELIVERY */

#define		UNRELIABLE_TYPE		0x00000001
#define		RELIABLE_TYPE		0x00000002
#define		FIFO_TYPE		0x00000004
#define		AGREED_TYPE		0x00000008
#define		SAFE_TYPE		0x00000010
#define		REGULAR_TYPE		0x0000001f
#define		BLOCK_REGULAR_DELIVERY  ( REGULAR_TYPE + 1 )  /* NOTE: used in Prot_set_delivery_threshold to only allow low level calls to Deliver_packet to deliver packets (e.g. - Discard_packets) */

#define		ROUTED_TYPE		0x00000020

#define		HURRY_TYPE		0x00000040

#define		ALIVE_TYPE		0x00000100
#define		JOIN_TYPE		0x00000200
#define		REFER_TYPE		0x00000400
#define		MEMBERSHIP_TYPE		0x00000700

#define		FORM1_TYPE		0x00001000
#define		FORM2_TYPE		0x00002000
#define		FORM_TYPE		0x00003000

#define		ARQ_TYPE		0x000f0000
#define	        RETRANS_TYPE		0x00f00000

#define		STATUS_TYPE		0x01000000
#define		PARTITION_TYPE		0x02000000
#define		FC_TYPE			0x04000000
#define		RELOAD_TYPE		0x08000000
#define		CONTROL_TYPE		0x0f000000


#define         Is_unreliable( t )      ( (t) &  UNRELIABLE_TYPE )
#define         Is_reliable( t )        ( (t) &  RELIABLE_TYPE   )
#define         Is_fifo( t )            ( (t) &  FIFO_TYPE       )
#define         Is_agreed( t )          ( (t) &  AGREED_TYPE     )
#define         Is_safe( t )            ( (t) &  SAFE_TYPE       )
#define         Is_regular( t )         ( (t) &  REGULAR_TYPE    )

#define         Is_routed( t )          ( (t) &  ROUTED_TYPE     )
#define         Set_routed( t )         ( (t) |  ROUTED_TYPE     )
#define         Clear_routed( t )       ( (t) & ~ROUTED_TYPE     )

#define         Is_hurry( t )           ( (t) &  HURRY_TYPE      )

#define         Is_alive( t )           ( (t) &  ALIVE_TYPE      )
#define         Is_join( t )            ( (t) &  JOIN_TYPE       )
#define         Is_refer( t )           ( (t) &  REFER_TYPE      )
#define         Is_membership( t )      ( (t) &  MEMBERSHIP_TYPE )

#define         Is_form( t )            ( (t) &  FORM_TYPE       )
#define         Is_form1( t )           ( (t) &  FORM1_TYPE      )
#define         Is_form2( t )           ( (t) &  FORM2_TYPE      )

#define         Get_arq( t )            ( ( (t) &  ARQ_TYPE       ) >> 16 )
#define         Set_arq( t, val )       ( ( (t) & ~ARQ_TYPE       ) | ( ( (val) << 16) & ARQ_TYPE ) )
#define         Get_retrans( t )        ( ( (t) &  RETRANS_TYPE   ) >> 20 )
#define         Set_retrans( t, val)    ( ( (t) & ~RETRANS_TYPE   ) | ( ( (val) << 20) & RETRANS_TYPE ) )

#define         Is_status( t )          ( (t) &  STATUS_TYPE     )
#define         Is_partition( t )       ( (t) &  PARTITION_TYPE  )
#define         Is_fc( t )              ( (t) &  FC_TYPE         )
#define         Is_conf_reload( t )     ( (t) &  RELOAD_TYPE     )
#define         Is_control( t )         ( (t) &  CONTROL_TYPE    )

typedef	struct
{
	int16		fragment_index;
	int16		fragment_len;
  
} fragment_header;

typedef	struct
{
	int32		type;
	int32		transmiter_id;
	int32		proc_id;
	membership_id	memb_id;
	int32		seq;
	int32		token_round; /* ### changed from fifo_seq */
        int32           conf_hash;
	int16u		data_len;
	int16           padding;
        fragment_header first_frag_header;
  
} packet_header;

typedef	char packet_body[MAX_PACKET_SIZE - sizeof(packet_header)];

typedef	struct
{
	int32		type;
	int32		transmiter_id;
        int32		proc_id;
        membership_id   memb_id;
	int32		seq;
	int32		aru;
        int32           aru_last_id;
	int16		flow_control;
	int16u		rtr_len;
        int32           conf_hash;

} token_header;

typedef	char token_body[((0x1 << 16) - 1) /* max IP packet size */ - 20 /* IP */ - 8 /* UDP */ - sizeof(token_header)];

typedef	struct
{
	membership_id	memb_id;
	int32		proc_id;
	int16		seg_index;
	int16		num_seq;

} ring_rtr;

#endif	/* INC_NET_TYPES */
