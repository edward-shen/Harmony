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

#include "arch.h"

#ifndef	ARCH_PC_WIN95

#include <netdb.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#else 	/* ARCH_PC_WIN95 */

#include <winsock2.h>

#endif	/* ARCH_PC_WIN95 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <assert.h>

#include <stdutil/stdutil.h>

#include "configuration.h"
#include "ip_enum.h"

#define ext_conf_body
#include "conf_body.h"
#undef  ext_conf_body

#include "spu_alarm.h"
#include "spu_memory.h"
#include "spread_params.h"

static	proc	My;

/* True means allow dangerous monitor commands like partition and flow control
 * to be handled. 
 * False means to ignore requests for those actions. FALSE IS THE SAFE SETTING
 */
static  bool    EnableDangerousMonitor = FALSE;

static  port_reuse SocketPortReuse = PORT_REUSE_AUTO;

static  char    *RuntimeDir = NULL;

static	char	*User = NULL;

static	char	*Group = NULL;

static  int     MaxSessionMessages = DEFAULT_MAX_SESSION_MESSAGES;

static  int     Window = DEFAULT_WINDOW;
static  int     PersonalWindow = DEFAULT_PERSONAL_WINDOW;

static  bool    AcceleratedRingFlag = FALSE;
static  bool    AcceleratedRing     = FALSE;
static  int     AcceleratedWindow   = DEFAULT_ACCELERATED_WINDOW;

enum 
{
  TOKEN_TIMEOUT_CONF  = (0x1 << 0),
  HURRY_TIMEOUT_CONF  = (0x1 << 1),
  ALIVE_TIMEOUT_CONF  = (0x1 << 2),
  JOIN_TIMEOUT_CONF   = (0x1 << 3),
  REP_TIMEOUT_CONF    = (0x1 << 4),
  SEG_TIMEOUT_CONF    = (0x1 << 5),
  GATHER_TIMEOUT_CONF = (0x1 << 6),
  FORM_TIMEOUT_CONF   = (0x1 << 7),
  LOOKUP_TIMEOUT_CONF = (0x1 << 8),
  ALL_TIMEOUT_CONF    = 0x1FF
};

static  int     TimeoutMask;

static  int     TokenTimeout;
static  int     HurryTimeout;
static  int     AliveTimeout;
static  int     JoinTimeout;
static  int     RepTimeout;
static  int     SegTimeout;
static  int     GatherTimeout;
static  int     FormTimeout;
static  int     LookupTimeout;

static  bool    WideAreaNetworkFlag = FALSE;
static  bool    WideAreaNetwork     = FALSE;

static  bool    Conf_Reload_State = FALSE;
static  bool    Conf_Reload_Singleton_State = FALSE;
static  configuration *Config_Previous;
static  char    Conf_FileName[256];
static  char    Conf_MyName_buf[256];
static  char   *Conf_MyName;

#if AF_INET == -1 || AF_INET6 == -1
#  error "Conf_Active_IP_Version assumes that neither AF_INET nor AF_INET6 is -1 but one of them is!"
#endif

static  int     Conf_Active_IP_Version = -1;

char Conf_id_to_str_buf[MAX_ID_SIZE];

void	Conf_init( char *file_name, char *my_name )
{
  proc *config_procs;
        
  strncpy(Conf_FileName, file_name, sizeof(Conf_FileName));
  Conf_FileName[sizeof(Conf_FileName) - 1] = 0;

  if (strcmp(Conf_FileName, file_name))
    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_init: configuration filename was too long!\n");
        
  if (my_name != NULL)
  {
    strncpy(Conf_MyName_buf, my_name, sizeof(Conf_MyName_buf));
    Conf_MyName_buf[sizeof(Conf_MyName_buf) - 1] = 0;

    if (strcmp(Conf_MyName_buf, my_name))
      Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_init: daemon's name was too long!\n");
            
    Conf_MyName = &Conf_MyName_buf[0];
  }
  else
    Conf_MyName = NULL;

  if (NULL == (Config = Mem_alloc(sizeof(configuration))))
    Alarmp( SPLOG_FATAL, CONF_SYS, "Conf_init: Failed to allocate memory for configuration structure\n");
        
  if (NULL == (config_procs = Mem_alloc( MAX_PROCS_RING * sizeof(proc))))
    Alarmp( SPLOG_FATAL, CONF_SYS, "Conf_init: Failed to allocate memory for configuration procs array\n");

  Config->allprocs = config_procs;

  Conf_load_conf_file( file_name, my_name);
}

bool    Conf_in_reload_singleton_state(void)
{
        return(Conf_Reload_Singleton_State);
}

void    Conf_reload_singleton_state_begin(void)
{
        Conf_Reload_Singleton_State = TRUE;
}

void    Conf_reload_singleton_state_end(void)
{
        Conf_Reload_Singleton_State = FALSE;
}

bool    Conf_in_reload_state(void)
{
        return(Conf_Reload_State);
}

void    Conf_reload_state_begin(void)
{
        Conf_Reload_State = TRUE;
}

void    Conf_reload_state_end(void)
{
        Conf_Reload_State = FALSE;
}

static bool Conf_daemon_changed(const proc *np, const segment *np_seg, const proc *op, const segment *op_seg, bool me)
{
  char idstr[MAX_ID_SIZE];
  int  lvl1 = (me ? SPLOG_PRINT : SPLOG_INFO);
  int  lvl2 = (me ? SPLOG_FATAL : SPLOG_INFO);
  int  i;

  if (strncmp(np->name, op->name, MAX_PROC_NAME)            ||
      spu_addr_ip_cmp(&np->proc_addr, &op->proc_addr, TRUE) ||
      np->id != op->id                                      ||
      np->num_if != op->num_if)
  {
    /* NOTE: we use several separate alarms bc we use same variables for multiple outputs (e.g. - idstr, global buffer for ntop) */

    Alarmp(lvl1, CONF_SYS, (me ?
                            "Conf_reload_initiate: My daemon parameters have changed! Exiting!\n" :
                            "Conf_reload_initiate: A daemon's parameters have changed! Partitioning down to singleton!\n"));
          
    Alarmp(lvl1, CONF_SYS, "\tOld: name '%s', addr [%s]:%u, id '%s', num_ifs %d\n",
           op->name, SPU_ADDR_NTOP_CANON(&op->proc_addr), (unsigned) spu_addr_ip_get_port(&op->proc_addr), Conf_id_to_str(op->id, idstr), op->num_if);
          
    Alarmp(lvl2, CONF_SYS, "\tNew: name '%s', addr [%s]:%u, id '%s', num_ifs %d\n",
           np->name, SPU_ADDR_NTOP_CANON(&np->proc_addr), (unsigned) spu_addr_ip_get_port(&np->proc_addr), Conf_id_to_str(np->id, idstr), np->num_if);

    return TRUE;
  }

  for (i = 0; i < np->num_if; i++)
  {
    if (np->ifc[i].iftype != op->ifc[i].iftype ||
        spu_addr_ip_cmp(&np->ifc[i].ifaddr, &op->ifc[i].ifaddr, TRUE) ||
        (me && spu_addr_family(&np->ifc[i].ifaddr) == AF_INET6 && np->ifc[i].ifaddr.ipv6.sin6_scope_id != op->ifc[i].ifaddr.ipv6.sin6_scope_id))
    {
      /* NOTE: we use several separate alarms bc we use same variables for output (e.g. - idstr, global buffer for ntop) */
      
      Alarmp(lvl1, CONF_SYS, (me ?
                              "Conf_reload_initiate: My daemon interface specs have changed! Exiting!\n" :
                              "Conf_reload_initiate: A daemon's interface specs have changed! Partitioning down to singleton!\n"));
                    
      Alarmp(lvl1, CONF_SYS, "\tOld: name '%s', iftype 0x%x, ifaddr [%s]:%u\n",
             op->name, op->ifc[i].iftype, SPU_ADDR_NTOP(&op->ifc[i].ifaddr), (unsigned) spu_addr_ip_get_port(&op->ifc[i].ifaddr));
          
      Alarmp(lvl2, CONF_SYS, "\tNew: name '%s', iftype 0x%x, ifaddr [%s]:%u\n",
             np->name, np->ifc[i].iftype, SPU_ADDR_NTOP(&np->ifc[i].ifaddr), (unsigned) spu_addr_ip_get_port(&np->ifc[i].ifaddr));

      return TRUE;
    }
  }

  if (np_seg->num_seg_addrs != op_seg->num_seg_addrs)
    i = 0;

  else
    for (i = 0; i < np_seg->num_seg_addrs && !spu_addr_ip_cmp(&np_seg->seg_addrs[i], &op_seg->seg_addrs[i], TRUE); ++i)
      if (me && spu_addr_family(&np_seg->seg_addrs[i]) == AF_INET6 && np_seg->seg_addrs[i].ipv6.sin6_scope_id != op_seg->seg_addrs[i].ipv6.sin6_scope_id)
        break;
  
  if (i != np_seg->num_seg_addrs)
  {
    Alarmp(lvl1, CONF_SYS, (me ?
                            "Conf_reload_initiate: My segment parameters have changed! Exiting!\n" :
                            "Conf_reload_initiate: A segment's parameters have changed! Partitioning down to singleton!\n"));
    Alarmp(lvl1, CONF_SYS, "\tOld: name '%s'\n", op->name);
    Alarmp(lvl2, CONF_SYS, "\tNew: name '%s'\n", np->name);

    return TRUE;
  }

  return FALSE;
}

/* Basic algorithm:
 * 1) copy Config to oldConfig
 * 2) load new spread.conf file into Config
 * 3) Check if we should exit;
 * 4) Check if this change is only add/sub reconfig or not. Return answer
 */
bool    Conf_reload_initiate(void)
{
  char idstr[MAX_ID_SIZE];
  bool need_singleton = FALSE;
  proc *np, *op, *op2;
  int  i;

  /* make a copy of current configuration then load new configuration */
  
  if ((Config_Previous = Mem_alloc(sizeof(configuration))) == NULL)
    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_reload_initiate: Failed to allocate memory for old configuration structure\n");        
        
  if ((Config_Previous->allprocs = Mem_alloc(MAX_PROCS_RING * sizeof(proc))) == NULL)
    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_reload_initiate: Failed to allocate memory for old configuration procs array\n");

  Conf_config_copy(Config, Config_Previous);

  if (Conf_proc_ref_by_id_in_conf(Config_Previous, My.id, &op) < 0)
    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_reload_initiate: BUG! My.id(%s) is not in previous config!\n", Conf_id_to_str(My.id, idstr));

  Conf_load_conf_file(Conf_FileName, Conf_MyName);  /* NOTE: updates My */

  /* check if this daemon is still in config with same identity + networking; exit if not */
        
  if (Conf_proc_ref_by_id(My.id, &np) < 0)
    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_reload_initiate: BUG! New My.id(%s) is no longer in config!\n", Conf_id_to_str(My.id, idstr));

  Conf_daemon_changed(np, &Config->segments[np->seg_index], op, &Config_Previous->segments[op->seg_index], TRUE);  /* NOTE: will print + exit if this daemon has changed */
  
  /* check that any daemons both in old and new configs still have same identity + networking */
  /* NOTE: Do we really require that all surviving daemons' networking can't change at all? Probably not, but this is safe. */
  
  for (i = 0; i < Config->num_total_procs; ++i)
  {
    np = &Config->allprocs[i];
    Conf_proc_ref_by_name_in_conf(Config_Previous, np->name, (op = NULL, &op));
    Conf_proc_ref_by_id_in_conf(Config_Previous, np->id, (op2 = NULL, &op2));
    
    /* NOTE: we need to be paranoid here bc user could change daemon names and/or vids at same time */
    
    if (op == NULL && op2 == NULL)
    {
      Alarmp(SPLOG_INFO, CONF_SYS, "Conf_reload_initiate: Added new daemon: name %s, addr [%s]:%u, id '%s'\n",
             np->name, SPU_ADDR_NTOP_CANON(&np->proc_addr), (unsigned) spu_addr_ip_get_port(&np->proc_addr), Conf_id_to_str(np->id, idstr));
      continue;
    }

    if (op != op2)
    {
      Alarmp(SPLOG_INFO, CONF_SYS, "Conf_reload_initiate: daemon identity mapped to two different old daemons: name '%s' -> %p, id '%s' -> %p! Partitioning to singleton!\n",
             np->name, op, Conf_id_to_str(np->id, idstr), op2);
            
      need_singleton = TRUE;
      break;
    }

    if ((need_singleton = Conf_daemon_changed(np, &Config->segments[np->seg_index], op, &Config_Previous->segments[op->seg_index], FALSE)))
      break;
  }

  /* free old config structs and arrays since they will never be used again */
  
  dispose(Config_Previous->allprocs);
  dispose(Config_Previous);
  Config_Previous = NULL;
        
  Alarmp(SPLOG_INFO, CONF_SYS, "Conf_reload_initiate: Return need_singleton = %d\n", need_singleton);
        
  return need_singleton;
}

void	Conf_load_conf_file(char *file_name, char *my_name)
{
        size_t    num_ips;
        spu_addr *ips                             = ip_enum_local(&num_ips);
        char      configfile_location[MAXPATHLEN] = { 0 };
        int32u    scope_id                        = 0;
	char	  idstr[MAX_ID_SIZE];
        char      name[MAX_PROC_NAME];
        int       i;
        size_t    j;
        
        if (ips == NULL || num_ips == 0)
          Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_load_conf_file: Couldn't find any local internet addresses on this machine!\n");

        /* open configuration file */
        
        strcat(configfile_location, SPREAD_ETCDIR);
        strcat(configfile_location, "/spread.conf");

	if (NULL != (yyin = fopen(file_name,"r")))
          Alarmp(SPLOG_INFO, CONF_SYS, "Conf_load_conf_file: using file: %s\n", file_name);
        
	else if (NULL != (yyin = fopen("./spread.conf", "r")))
          Alarmp(SPLOG_INFO, CONF_SYS, "Conf_load_conf_file: using file: ./spread.conf\n");
        
	else if (NULL != (yyin = fopen(configfile_location, "r")))
          Alarmp(SPLOG_INFO, CONF_SYS, "Conf_load_conf_file: using file: %s\n", configfile_location);
        
        else
          Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_load_conf_file: error opening config file %s and alternates!\n", file_name);

        /* reinitialize and run parser */
        
        parser_init();        
	yyparse();
        fclose(yyin);

        /* match my_name to an entry in configuration file */
        
	if (my_name == NULL)
        {
          for (i = 0; i < Config->num_total_procs; ++i)
            for (j = 0; j < num_ips; ++j)
              if (!spu_addr_ip_cmp(&Config->allprocs[i].proc_addr, &ips[j], FALSE))
              {
                My = Config->allprocs[i];
                goto DONE;
              }

        DONE:
          if (i == Config->num_total_procs)
            Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_load_conf_file: None of this machine's internet addresses match any daemon's address in configuration!\n");
	}
        else
        {
          strncpy(name, my_name, MAX_PROC_NAME);
          name[MAX_PROC_NAME - 1] = 0;
                
          if (Conf_proc_by_name(name, &My) < 0)
            Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_load_conf_file: My proc %s is not in configuration!\n", name);
	}

        /* make sure that the addresses we configured for this daemon are actually on this machine */
        /* set IPv6 sin6_scope_id's */

        for (j = 0; j < num_ips && spu_addr_ip_cmp(&My.proc_addr, &ips[j], FALSE); ++j);

        if (j >= num_ips)
          Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_load_conf_file: None of this machine's internet addresses match this daemon's address %s!\n", SPU_ADDR_NTOP(&My.proc_addr));

        if (spu_addr_family(&My.proc_addr) == AF_INET6)
        {
          if (My.proc_addr.ipv6.sin6_scope_id == 0)                       /* NOTE: if address contained an interface spec (e.g. - %eth0), then parser did an if_nametoindex() on it; otherwise zero */
            My.proc_addr.ipv6.sin6_scope_id = ips[j].ipv6.sin6_scope_id;  /* if no interface was spec'd or the lookup failed; try to use whatever ip_enum_local found instead (e.g. - getifaddrs) */

          if ((scope_id = My.proc_addr.ipv6.sin6_scope_id) != 0)
            Alarmp(SPLOG_INFO, CONF_SYS, "Conf_load_conf_file: my ipv6 address [%s] corresponds with interface index %u; using that (sin6_scope_id) for all unicast sends!\n",
                   SPU_ADDR_NTOP(&My.proc_addr), (unsigned) scope_id);

          else
            Alarmp(SPLOG_WARNING, CONF_SYS, "Conf_load_conf_file: couldn't find an interface index that corresponds with my address [%s]! Trying to use a scope_id of 0 for all unicast sends!\n",
                   SPU_ADDR_NTOP(&My.proc_addr));
          
          for (i = 0; i < Config->num_total_procs; ++i)
            Config->allprocs[i].proc_addr.ipv6.sin6_scope_id = scope_id;

          for (i = 0; i < Config->segments[My.seg_index].num_seg_addrs; ++i)
          {
            if (Config->segments[My.seg_index].seg_addrs[i].ipv6.sin6_scope_id == 0)                
              Config->segments[My.seg_index].seg_addrs[i].ipv6.sin6_scope_id = scope_id;

            if (Config->segments[My.seg_index].seg_addrs[i].ipv6.sin6_scope_id != 0)            
              Alarmp(SPLOG_INFO, CONF_SYS, "Conf_load_conf_file: my ipv6 segment address [%s] corresponds with interface index %u; using that (sin6_scope_id) for this segment address!\n",
                     SPU_ADDR_NTOP(&Config->segments[My.seg_index].seg_addrs[i]), (unsigned) Config->segments[My.seg_index].seg_addrs[i].ipv6.sin6_scope_id);

            else
              Alarmp(SPLOG_WARNING, CONF_SYS, "Conf_load_conf_file: couldn't find an interface index that corresponds with my segment address [%s]! Trying to use a scope_id of 0 for this segment address!\n",
                     SPU_ADDR_NTOP(&Config->segments[My.seg_index].seg_addrs[i]));
          }
        }

        for (i = 0; i < My.num_if; ++i)
        {
          if (spu_addr_ip_is_unspecified(&My.ifc[i].ifaddr) || spu_addr_ip_is_multicast(&My.ifc[i].ifaddr))
            continue;
         
          for (j = 0; j < num_ips && spu_addr_ip_cmp(&My.ifc[i].ifaddr, &ips[j], FALSE); ++j);
          
          if (j >= num_ips)
          {
            Alarmp(SPLOG_WARNING, CONF_SYS, "Conf_load_conf_file: none of this machine's addresses match configured interface %s\n", SPU_ADDR_NTOP(&My.ifc[i].ifaddr));            
            continue;
          }

          if (spu_addr_family(&My.ifc[i].ifaddr) == AF_INET6)
          {
            if (My.ifc[i].ifaddr.ipv6.sin6_scope_id == 0 &&
                (My.ifc[i].ifaddr.ipv6.sin6_scope_id = ips[j].ipv6.sin6_scope_id) == 0)
              My.ifc[i].ifaddr.ipv6.sin6_scope_id = scope_id;

            Alarmp(SPLOG_INFO, CONF_SYS, "Conf_load_conf_file: using sin6_scope_id %u for interface %s\n", (unsigned) My.ifc[i].ifaddr.ipv6.sin6_scope_id, SPU_ADDR_NTOP(&My.ifc[i].ifaddr));
          }
        }
        
	Alarmp(SPLOG_INFO, CONF_SYS, "Conf_load_conf_file: My name: %s, id: %s, addr: %s, port: %u\n",
               My.name, Conf_id_to_str(My.id, idstr), SPU_ADDR_NTOP(&My.proc_addr), (unsigned) spu_addr_ip_get_port(&My.proc_addr));
        
        free(ips);
}

configuration	Conf()
{
	return *Config;
}

configuration   *Conf_ref(void)
{
        return Config;
}

proc	Conf_my()
{
	return	My;
}

int	Conf_proc_by_id( int32u id, proc *p )
{
        return( Conf_proc_by_id_in_conf( Config, id, p ));
}

int     Conf_proc_ref_by_id( int32u id, proc **p )
{
        return( Conf_proc_ref_by_id_in_conf( Config, id, p ));
}

int 	Conf_proc_by_name( char *name, proc *p )
{
        return( Conf_proc_by_name_in_conf( Config, name, p));
}

int 	Conf_proc_ref_by_name( char *name, proc **p )
{
        return( Conf_proc_ref_by_name_in_conf( Config, name, p));
}

int	Conf_proc_by_id_in_conf( configuration *config, int32u id, proc *p )
{
        int   ret;
        proc *p2;

        if ( (ret = Conf_proc_ref_by_id_in_conf( config, id, &p2 )) >= 0 )
                *p = *p2;

	return( ret );
}

int     Conf_proc_ref_by_id_in_conf( configuration *config, int32u id, proc **p )
{
        int i;

	for ( i=0; i < config->num_total_procs; i++ )
	{
                if ( config->allprocs[i].id == id )
                {
                        *p = &config->allprocs[i];
                        return( i );
                }
	}
	return( -1 );
}

int 	Conf_proc_by_name_in_conf( configuration *config, char *name, proc *p )
{
        int   ret;
        proc *p2;

        if ( (ret = Conf_proc_ref_by_name_in_conf( config, name, &p2 )) >= 0 )
                *p = *p2;

	return( ret );
}

int 	Conf_proc_ref_by_name_in_conf( configuration *config, char *name, proc **p )
{
	int	i;

	for ( i=0; i < config->num_total_procs; i++ )
	{
                if ( !strcmp( config->allprocs[i].name, name ) )
		{
			*p = &config->allprocs[i];
			return( i );
		}
	}
	return( -1 );
}

int	Conf_id_in_seg( segment *seg, int32u id )
{
	int 	j;

	for ( j=0; j < seg->num_procs; j++ )
	{
                if ( seg->procs[j]->id == id )
                        return( j );
	}
	return( -1 );
}

void    Conf_config_copy( configuration *src_conf, configuration *dst_conf)
{
    int  i, j, p_index;
    proc p, *pp;

    pp                 = dst_conf->allprocs;
    *dst_conf          = *src_conf;
    dst_conf->allprocs = pp;
    
    for (i = 0; i < src_conf->num_total_procs; ++i)
      dst_conf->allprocs[i] = src_conf->allprocs[i];

    for (i = 0; i < src_conf->num_segments; ++i)
        for (j = 0; j < src_conf->segments[i].num_procs; ++j)
        {
            p_index = Conf_proc_by_id_in_conf(dst_conf, src_conf->segments[i].procs[j]->id, &p);
            assert(p_index >= 0);
            dst_conf->segments[i].procs[j] = &dst_conf->allprocs[p_index];
        }
    
#ifndef NDEBUG
    for (i = 0; i < dst_conf->num_segments; ++i)
        for (j = 0; j < dst_conf->segments[i].num_procs; ++j)
            assert(dst_conf->segments[i].procs[j]->id == src_conf->segments[i].procs[j]->id);
#endif
}

int     Conf_append_id_to_seg( segment *seg, int32u id)
{
        proc *p;
        
        if (Conf_proc_ref_by_id(id, &p) != -1)
        {
                seg->procs[seg->num_procs] = p;
                seg->num_procs++;
                return( 0 );
        } 
        return( -1 );
}

int	Conf_id_in_conf( configuration *config, int32u id )
{
	int 	i;

	for ( i = 0; i < config->num_segments; ++i )
                if ( Conf_id_in_seg(&(config->segments[i]), id) >= 0 )
                        return( i );
	return( -1 );
}

int	Conf_num_procs( configuration *config )
{
	int 	i, ret = 0;

	for ( i = 0; i < config->num_segments; ++i )
		ret += config->segments[i].num_procs;

	return( ret );
}

int     Conf_num_segments( configuration *config )
{
        return( config->num_segments );
}

int32u	Conf_leader( configuration *config )
{
        int i;

        for( i = 0; i < config->num_segments; ++i )
                if( config->segments[i].num_procs > 0 )
                        return( config->segments[i].procs[0]->id );
        
	Conf_print( config );
        Alarmp( SPLOG_FATAL, CONF_SYS, "Conf_leader: Empty configuration\n" );
        
	return( -1 );
}

int32u	Conf_last( configuration *config )
{
        int i;

        for( i = config->num_segments - 1; i >= 0; --i )
                if( config->segments[i].num_procs > 0 )
                        return( config->segments[i].procs[config->segments[i].num_procs - 1]->id );
        
	Conf_print( config );
        Alarmp( SPLOG_FATAL, CONF_SYS, "Conf_last: Empty configuration\n" );
        
	return( -1 );
}

int32u Conf_previous( configuration *config )
{
        int index_in_seg;
        int i;

        index_in_seg = Conf_id_in_seg( &config->segments[My.seg_index], My.id );

        if( index_in_seg > 0 )
          return config->segments[My.seg_index].procs[index_in_seg - 1]->id;           /* I am not first in my segment; previous is previous proc in segment */

        for( i = My.seg_index - 1; i >= 0; --i )
                if( config->segments[i].num_procs > 0 )  
                        return config->segments[i].procs[config->segments[i].num_procs - 1]->id;  /* There is a non-empty segment before mine; previous is last in that segment */

        for( i = config->num_segments - 1; i >= My.seg_index; --i )
                if( config->segments[i].num_procs > 0 )
                        return config->segments[i].procs[config->segments[i].num_procs - 1]->id;  /* I am first in first non-empty segment; previous is last in last segment */
        
        Alarmp( SPLOG_FATAL, CONF_SYS, "Conf_previous: No process found\n" );
        
        return( -1 );
}

int32u	Conf_seg_leader( configuration *config, int16 seg_index )
{
	if( config->segments[seg_index].num_procs > 0 )
		return( config->segments[seg_index].procs[0]->id );

	Conf_print( config );
        Alarmp( SPLOG_FATAL, CONF_SYS, "Conf_seg_leader: Empty segment %d in Conf\n", seg_index );
        
	return( -1 );
}

int32u	Conf_seg_last( configuration *config, int16 seg_index )
{
	int	j;

	if( config->segments[seg_index].num_procs > 0 )
	{
		j = config->segments[seg_index].num_procs-1;
		return( config->segments[seg_index].procs[j]->id );
	}
        
	Conf_print( config );
        Alarmp( SPLOG_FATAL, CONF_SYS, "Conf_seg_leader: Empty segment %d in Conf\n", seg_index );
        
        return(-1);
}

int	Conf_num_procs_in_seg( configuration *config, int16 seg_index )
{
	return( config->segments[seg_index].num_procs );
}

char   *Conf_id_to_str( int32u id, char *str )
{
  snprintf( str, MAX_ID_SIZE, "%lu", (unsigned long) id );

  return str;
}

int32u Conf_str_to_id(const char *str)
{
  size_t tlen = strlen(str) + 1;
  int32u ret  = (int32u) stdhcode_oaat(str, tlen);

  /* NOTE: auto-generated vids must be a cross platform (in host byte order), deterministic function of proc.name only! */
                            
  if (ret == 0 || ret == (int32u) -1)  /* protect reserved proc id's: 0 and -1, ~2 in 2^32 chance */
  {
    ret = (int32u) tlen << 16;         /* assume users will typically configure either most or least significant byte of VIDs; avoid them */

    if (ret == 0)                      /* next to impossible: requires a tlen with lower 16 bits == 0 and that hashes to zero */
      ret = (int32u) 0x1 << 16;        /* assume users will typically configure either most or least significant byte of VIDs; avoid them */
  }

  if (ret == 0 || ret == (int32u) -1)
    Alarmp(SPLOG_FATAL, CONF_SYS | EXIT, "Conf_str_to_id: BUG! A string mapped to 0 or -1 somehow?!\n");
  
  return ret;  
}

int	Conf_print(configuration *config)
{
	int 	s,p;
	char	idstr[MAX_ID_SIZE];
	proc	pr;

	Alarmp(SPLOG_PRINT, CONF_SYS, "--------------------\n" );
	Alarmp(SPLOG_PRINT, CONF_SYS, "Configuration at %s is:\n", My.name );
	Alarmp(SPLOG_PRINT, CONF_SYS, "Num Segments %d\n", config->num_segments );
        
	for ( s=0; s < config->num_segments; s++ )
	{
                Alarmp(SPLOG_PRINT, CONF_SYS, "\t%d", config->segments[s].num_procs);

                for (p = 0; p < config->segments[s].num_seg_addrs; ++p)                 
                  Alarmp(SPLOG_PRINT | SPLOG_NODATE, CONF_SYS, "\t[%s]:%u", SPU_ADDR_NTOP(&config->segments[s].seg_addrs[p]), (unsigned) spu_addr_ip_get_port(&config->segments[s].seg_addrs[p]));

                Alarmp(SPLOG_PRINT | SPLOG_NODATE, CONF_SYS, "\n");
                
		for( p=0; p < config->segments[s].num_procs; p++)
		{
			Conf_proc_by_id( config->segments[s].procs[p]->id, &pr );	
			Alarmp(SPLOG_PRINT, CONF_SYS, "\t\t%-" QQ(MAX_PROC_NAME) "s\t%s\tID: %-" QQ(MAX_ID_SIZE) "s\n",
                               pr.name, SPU_ADDR_NTOP(&pr.proc_addr), Conf_id_to_str(pr.id, idstr) );
		}
	}
        
	Alarmp(SPLOG_PRINT, CONF_SYS, "====================\n" );
        
	return( 0 );
}


int	Conf_print_procs(configuration *config)
{
	int 	i;
	char	idstr[MAX_ID_SIZE];

	Alarmp(SPLOG_PRINT, CONF_SYS, "--------------------\n" );
	Alarmp(SPLOG_PRINT, CONF_SYS, "Configured Procs at %s is:\n", My.name );
	Alarmp(SPLOG_PRINT, CONF_SYS, "Num Procs %d\n",config->num_total_procs );
        
	for ( i=0; i < config->num_total_procs; i++ )
	{
                const proc *pr = &config->allprocs[i];
          
                Alarmp(SPLOG_PRINT, CONF_SYS, "\t%-" QQ(MAX_PROC_NAME) "s\tID: %-" QQ(MAX_ID_SIZE) "s\t[%s]:%u\tNumIfs: %d\n",
                       pr->name, Conf_id_to_str(config->allprocs[i].id, idstr), 
                       SPU_ADDR_NTOP(&pr->proc_addr), (unsigned) spu_addr_ip_get_port(&pr->proc_addr), config->allprocs[i].num_if);
                       
        }
	Alarmp(SPLOG_PRINT, CONF_SYS, "====================\n" );
                
	return( 0 );
}

bool    Conf_get_dangerous_monitor_state(void)
{
        return(EnableDangerousMonitor);
}

void    Conf_set_dangerous_monitor_state(bool new_state)
{
        if (new_state)
          Alarmp(SPLOG_PRINT, CONF_SYS, "ENABLING Dangerous Monitor Commands! Make sure Spread network is secured\n");
        else
          Alarmp(SPLOG_PRINT, CONF_SYS, "Disabling Dangerous Monitor Commands!\n");

        EnableDangerousMonitor = new_state;
}

port_reuse Conf_get_port_reuse_type(void)
{
        return(SocketPortReuse);
}

void    Conf_set_port_reuse_type(port_reuse state)
{
        switch (state)
        {
        case PORT_REUSE_AUTO:
                Alarmp(SPLOG_PRINT, CONF_SYS, "Setting SO_REUSEADDR to auto\n");
                break;
        case PORT_REUSE_ON:
                Alarmp(SPLOG_PRINT, CONF_SYS, "Setting SO_REUSEADDR to always on -- make sure Spread daemon host is secured!\n");
                break;
        case PORT_REUSE_OFF:
                Alarmp(SPLOG_PRINT, CONF_SYS, "Setting SO_REUSEADDR to always off\n");
                break;
        default:
                Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_port_reuse_type: Bug! Unexpected state %d!\n", state);
                break;
        }
        SocketPortReuse = state;
}

static void set_param_if_valid(char **param, char *value, char *description, size_t max_value_len)
{
        if (value != NULL && *value != '\0')
        {
                size_t len       = strlen(value);
                char  *old_value = *param;
                char  *buf;
                
                if (len > max_value_len)
                  Alarmp(SPLOG_FATAL, CONF_SYS, "set_param_if_valid: value string too long\n");

                if (NULL == (buf = Mem_alloc(len + 1)))
                  Alarmp(SPLOG_FATAL, CONF_SYS, "set_param_if_valid: Out of memory\n");
                
                strncpy(buf, value, len);
                buf[len] = '\0';

                *param = buf;
                
                if (old_value != NULL)
                    dispose(old_value);

                Alarmp(SPLOG_INFO, CONF_SYS, "Set %s to '%s'\n", description, value);
        }
        else
          Alarmp(SPLOG_ERROR, CONF_SYS, "set_param_if_valid: Ignored invalid %s\n", description);
}

void    Conf_set_max_session_messages(int max_messages)
{
        if (max_messages < 0)
        {
            Alarmp(SPLOG_ERROR, CONF_SYS, "Conf_set_max_session_messages: Attempt to set max_message to less then zero. Resetting to default value of %d\n", DEFAULT_MAX_SESSION_MESSAGES);
            max_messages = DEFAULT_MAX_SESSION_MESSAGES;
        }
        Alarmp(SPLOG_DEBUG, CONF_SYS, "Conf_set_max_session_messages: Set Max Session Messages to %d\n", max_messages);
        MaxSessionMessages = max_messages;
}

int     Conf_get_max_session_messages(void)
{
        return (MaxSessionMessages);
}

void    Conf_set_active_ip_version(int ipver)
{
    if (Conf_Active_IP_Version != -1 && ipver != Conf_Active_IP_Version)
        Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_active_ip_version: active IP version already differently specified: %d vs. %d\n", Conf_Active_IP_Version, ipver);
    
    switch( ipver )
    {
    case AF_INET:
    case AF_INET6:
        Conf_Active_IP_Version = ipver;
        break;
    case -1:
    default:
        Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_active_ip_version: Invalid IP version specified: %d\n", ipver);
        break;
    }
}

int     Conf_get_active_ip_version(void)
{
        return (Conf_Active_IP_Version);
}

char    *Conf_get_runtime_dir(void)
{
        return (RuntimeDir != NULL ? RuntimeDir : SP_RUNTIME_DIR);
}

void    Conf_set_runtime_dir(char *dir)
{
        set_param_if_valid(&RuntimeDir, dir, "runtime directory", MAXPATHLEN);
}

char    *Conf_get_user(void)
{
        return (User != NULL ? User : SP_USER);
}

void    Conf_set_user(char *user)
{
        set_param_if_valid(&User, user, "user name", 32);
}

char    *Conf_get_group(void)
{
        return (Group != NULL ? Group : SP_GROUP);
}

void    Conf_set_group(char *group)
{
        set_param_if_valid(&Group, group, "group name", 32);
}

void    Conf_set_window(int window)
{
        if (window <= 0)
	    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_window: Attempt to set window to non-positive (%d)!\n", window);

        Alarmp(SPLOG_DEBUG, CONF_SYS, "Conf_set_window: Set Window to %d\n", window);
        Window = window;
}

int     Conf_get_window(void)
{
	return Window;
}

void    Conf_set_personal_window(int pwindow)
{
        if (pwindow <= 0)
	    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_personal_window: Attempt to set personal window to non-positive (%d)!\n", pwindow);

        Alarmp(SPLOG_DEBUG, CONF_SYS, "Conf_set_personal_window: Set Personal Window to %d\n", pwindow);
        PersonalWindow = pwindow;
}

int     Conf_get_personal_window(void)
{
	return PersonalWindow;
}

void Conf_set_accelerated_ring_flag(bool flag_is_set)
{
  AcceleratedRingFlag = flag_is_set;
}

bool Conf_get_accelerated_ring_flag(void)
{
  return AcceleratedRingFlag;
}

void Conf_set_accelerated_ring(bool prot_is_accelerated)
{
  AcceleratedRing = prot_is_accelerated;
}

bool Conf_get_accelerated_ring(void)
{
  return AcceleratedRing;
}

void Conf_set_accelerated_window(int pwindow)
{
        if (pwindow < 0)
	    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_accelerated_window: Attempt to set window to non-positive (%d)!\n", pwindow);

        Alarmp(SPLOG_DEBUG, CONF_SYS, "Conf_set_accelerated_window: Set Window to %d\n", pwindow);
	AcceleratedWindow = pwindow;
}

int Conf_get_accelerated_window(void)
{
  return AcceleratedWindow;
}

int Conf_memb_timeouts_set(void)
{
  return TimeoutMask != 0;
}

int Conf_all_memb_timeouts_set(void)
{
  return TimeoutMask == ALL_TIMEOUT_CONF;
}

void Conf_set_token_timeout(int timeout) 
{
  if (timeout <= 0)
    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_token_timeout: Non-positive timeout (%d) specified!\n", timeout);

  TimeoutMask |= TOKEN_TIMEOUT_CONF;
  TokenTimeout = timeout;
}

int Conf_get_token_timeout(void) 
{ 
  return TokenTimeout; 
}

void Conf_set_hurry_timeout(int timeout) 
{
  if (timeout <= 0)
    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_hurry_timeout: Non-positive timeout (%d) specified!\n", timeout);

  TimeoutMask |= HURRY_TIMEOUT_CONF;
  HurryTimeout = timeout;
}

int Conf_get_hurry_timeout(void) 
{ 
  return HurryTimeout; 
}

void Conf_set_alive_timeout(int timeout) 
{
  if (timeout <= 0)
    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_alive_timeout: Non-positive timeout (%d) specified!\n", timeout);

  TimeoutMask |= ALIVE_TIMEOUT_CONF;
  AliveTimeout = timeout;
}

int Conf_get_alive_timeout(void) 
{ 
  return AliveTimeout; 
}

void Conf_set_join_timeout(int timeout) 
{
  if (timeout <= 0)
    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_join_timeout: Non-positive timeout (%d) specified!\n", timeout);

  TimeoutMask |= JOIN_TIMEOUT_CONF;
  JoinTimeout = timeout;
}

int Conf_get_join_timeout(void) 
{ 
  return JoinTimeout; 
}

void Conf_set_rep_timeout(int timeout) 
{
  if (timeout <= 0)
    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_rep_timeout: Non-positive timeout (%d) specified!\n", timeout);

  TimeoutMask |= REP_TIMEOUT_CONF;
  RepTimeout = timeout;
}

int Conf_get_rep_timeout(void) 
{ 
  return RepTimeout; 
}

void Conf_set_seg_timeout(int timeout) 
{
  if (timeout <= 0)
    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_seg_timeout: Non-positive timeout (%d) specified!\n", timeout);

  TimeoutMask |= SEG_TIMEOUT_CONF;
  SegTimeout = timeout;
}

int Conf_get_seg_timeout(void) 
{ 
  return SegTimeout; 
}

void Conf_set_gather_timeout(int timeout) 
{
  if (timeout <= 0)
    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_gather_timeout: Non-positive timeout (%d) specified!\n", timeout);

  TimeoutMask |= GATHER_TIMEOUT_CONF;
  GatherTimeout = timeout;
}

int Conf_get_gather_timeout(void) 
{ 
  return GatherTimeout; 
}

void Conf_set_form_timeout(int timeout) 
{
  if (timeout <= 0)
    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_form_timeout: Non-positive timeout (%d) specified!\n", timeout);

  TimeoutMask |= FORM_TIMEOUT_CONF;
  FormTimeout = timeout;
}

int Conf_get_form_timeout(void) 
{ 
  return FormTimeout; 
}

void Conf_set_lookup_timeout(int timeout) 
{
  if (timeout <= 0)
    Alarmp(SPLOG_FATAL, CONF_SYS, "Conf_set_lookup_timeout: Non-positive timeout (%d) specified!\n", timeout);

  TimeoutMask |= LOOKUP_TIMEOUT_CONF;
  LookupTimeout = timeout;
}

int Conf_get_lookup_timeout(void) 
{ 
  return LookupTimeout; 
}

void Conf_set_wide_area_network_flag(bool flag_is_set)
{
  WideAreaNetworkFlag = flag_is_set;
}

bool Conf_get_wide_area_network_flag(void)
{
  return WideAreaNetworkFlag;
}

void Conf_set_wide_area_network(bool isWan)
{
  WideAreaNetwork = isWan;
}

bool Conf_get_wide_area_network(void)
{
  return WideAreaNetwork;
}
