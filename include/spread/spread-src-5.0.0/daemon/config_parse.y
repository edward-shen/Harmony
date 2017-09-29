%{
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
 *    Ryan Caudy           rcaudy@gmail.com - contributions to process groups.
 *    Claudiu Danilov      claudiu@acm.org - scalable wide area support.
 *    Cristina Nita-Rotaru crisn@cs.purdue.edu - group communication security.
 *    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.
 *    Dan Schoenblum       dansch@cnds.jhu.edu - Java interface.
 *
 */

#include "arch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdutil/stdutil.h>

#ifndef ARCH_PC_WIN95
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/param.h>

#else /* ARCH_PC_WIN95 */
#include <winsock2.h>
#endif /* ARCH_PC_WIN95 */

#include "spu_alarm.h"
#include "configuration.h"
#include "spu_memory.h"
#include "spu_objects.h"
#include "conf_body.h"
#include "acm.h"

#define PARSE_ERROR_STR     "----------- Parse Error -------------------\nError on or before line %d\nOffending token: %s\nError type: "
#define PARSE_WARNING_STR   "----------- Parse Warning------------------\nWarning on or before line %d\nOffending token: %s\nWarning type: "
  
/********************************************************************************
 ********************************************************************************/

       int     line_num, semantic_errors;
extern char   *yytext;
extern void    yyerror(const char *str);
extern void    yywarn(const char *str);
extern int     yylex();

static int     num_procs = 0;
static int     segment_procs = 0;
static int     segment_addrs = 0;
static int     segments = 0;
static int     procs_interfaces = 0;

static int     authentication_configured = 0;

#define MAX_ALARM_FORMAT 40
static char    alarm_format[MAX_ALARM_FORMAT];
static int     alarm_precise = 0;
static int     alarm_custom_format = 0;
 
#define MAX_CONF_STRING 100000

static  char   ConfStringRep[MAX_CONF_STRING + 1];
static  int    ConfStringLen;

/********************************************************************************
 ********************************************************************************/

void parser_init()
{
    num_procs        = 0;
    segment_procs    = 0;
    segment_addrs    = 0;
    segments         = 0;
    procs_interfaces = 0;    
    ConfStringRep[0] = 0;
    ConfStringLen    = 0;
}

/********************************************************************************
 ********************************************************************************/

static void expand_filename(char *out_string, int str_size, const char *in_string)
{
  const char *in_loc;
  char       *out_loc;
  char        hostn[MAXHOSTNAMELEN + 1];
  
  for (in_loc = in_string, out_loc = out_string; out_loc - out_string < str_size; ++in_loc)
  {
    if (*in_loc == '%')
    {
      switch (in_loc[1])
      {
      case 'h':
      case 'H':
        gethostname(hostn, sizeof(hostn));
        out_loc += snprintf(out_loc, str_size - (out_loc - out_string), "%s", hostn); 
        in_loc++;
        continue;
        
      default:
        break;
      }
    }
    
    *out_loc = *in_loc;
    out_loc++;
    
    if (*in_loc == '\0')
      break;
  }
  
  out_string[str_size - 1] = '\0';
}

/********************************************************************************
 ********************************************************************************/

static void set_daemon_name_and_addr(const char *name, const conf_ipaddr *conf_addr)
{
  const spu_addr *addr   = NULL;
  int             family = Conf_get_active_ip_version();
  int             ret;
  char           *tmp;
  int             i;

  /* check for zero length daemon name: can happen with a quoted string */
  /* warn if name ends with a semicolon: user likely intended it to separate two daemon specs but stuck it on the end of the name without any intervening white space */

  if (name != NULL)
  {
    size_t len = strlen(name);

    if (len == 0)
      yyerror("Zero length daemon name!");
    
    if (name[len - 1] == ';')
      yywarn("Daemon name ended with a semicolon! If you intended for it to demark two different daemon specifications then it needs white space around it!");
  }
  
  /* figure out daemon's address (.proc_addr) first */
  
  if (conf_addr != NULL)
  {
    addr = &conf_addr->ipaddr;

    if (spu_addr_ip_is_unspecified(addr))
      yyerror("A daemon's address must not be the unspecified address!");
    
    if (spu_addr_ip_get_port(addr) != 0)
      yyerror("A daemon's address must not specify a port!");

    Config->allprocs[num_procs].proc_addr = *addr;
  }
  else if (name != NULL)  /* do a DNS lookup for address */
  {
    struct addrinfo hint = { 0 }, *rslt, *curr;

    hint.ai_family   = PF_UNSPEC;
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_protocol = IPPROTO_UDP;
    hint.ai_flags    = 0;
    
    if ((ret = getaddrinfo(name, NULL, &hint, &rslt)))
      yyerror(ret != EAI_SYSTEM ? gai_strerror(ret) : strerror(errno));

    /* find first address of correct type */
    
    for (curr = rslt;
         curr != NULL && (curr->ai_addr == NULL || (curr->ai_addr->sa_family != AF_INET && curr->ai_addr->sa_family != AF_INET6) || (family != -1 && curr->ai_addr->sa_family != family));
         curr = curr->ai_next);

    if (curr == NULL)
      yyerror("Couldn't find an appropriate address based on daemon name! Check that your ActiveIPVersion and DNS entries match or explicitly configure the address to use!");

    if (spu_addr_from_sockaddr_known(&Config->allprocs[num_procs].proc_addr, curr->ai_addr))
      yyerror("Bug! Setting proc_addr from getaddrinfo failed?!");
    
    freeaddrinfo(rslt);
  }
  else
    yyerror("A daemon must have a name, an address or both!");

  /* sanity check and default ActiveIPVersion */

  if (family == -1)
    Conf_set_active_ip_version(spu_addr_family(&Config->allprocs[num_procs].proc_addr));

  else if (family != spu_addr_family(&Config->allprocs[num_procs].proc_addr))
    yyerror("A daemon's address was of the wrong type!  It disagreed with ActiveIPVersion!");

  if (spu_addr_ip_is_multicast(&Config->allprocs[num_procs].proc_addr))
    yyerror("A daemon's address can't be a multicast address!");
  
  /* figure out a daemon's .name */

  if (name == NULL)
    name = conf_addr->ipstr;

  strncpy(Config->allprocs[num_procs].name, name, MAX_PROC_NAME);
  Config->allprocs[num_procs].name[MAX_PROC_NAME - 1] = 0;

  /* NOTE: remove any trailing IPv6 interface specification, which are node-local, if we derived the name from the daemon address */
  
  if (conf_addr != NULL && name == conf_addr->ipstr && (tmp = strchr(Config->allprocs[num_procs].name, '%')) != NULL)
    *tmp = 0;
  
  if (Config->allprocs[num_procs].name[0] == 0)
    Alarmp(SPLOG_FATAL, CONF_SYS, "set_daemon_name_and_addr: BUG! Zero length proc name somehow?!\n");

  Alarmp(SPLOG_INFO, CONF_SYS, "Configured daemon '%s' with IP '%s'\n", Config->allprocs[num_procs].name, SPU_ADDR_NTOP(&Config->allprocs[num_procs].proc_addr));
  
  /* ensure .name uniqueness */

  for (i = 0; i < num_procs; ++i)
    if (!strncmp(Config->allprocs[num_procs].name, Config->allprocs[i].name, MAX_PROC_NAME))
      Alarmp(SPLOG_FATAL, CONF_SYS, PARSE_ERROR_STR "Daemon name (%s) not unique! Already in use by daemon #%d!\n", line_num, yytext, Config->allprocs[num_procs].name, i);

  /* NOTE: we ensure .proc_addr uniqueness when a segment is fully configured so we know the port # too */
}

/********************************************************************************
 ********************************************************************************/

static size_t convert_segment_to_string(char * const segstr, const int segstr_size, const segment *seg)
{
  char *c = segstr;
  char *e = segstr + segstr_size;
  char  idstr[MAX_ID_SIZE];
  int   i;

  /* NOTE: we use CANON so that there can be no confusion between implementations about the particular form of the addresses and that any interface specification is always left off */
  /* NOTE: we still include proc addr's in conf hash string to ensure all daemons are using same addresses in a config */

  c += snprintf(c, SPCLAMP(c, e), "Segment: port = %u;", (unsigned) spu_addr_ip_get_port(&seg->seg_addrs[0]));

  for (i = 0; i < seg->num_seg_addrs; ++i)  
    c += snprintf(c, SPCLAMP(c, e), " [%s]", SPU_ADDR_NTOP_CANON(&seg->seg_addrs[i]));

  c += snprintf(c, SPCLAMP(c, e), "\n");
  
  for (i = 0; i < seg->num_procs; ++i)
    c += snprintf(c, SPCLAMP(c, e), "\t'%s' [%s] [%s]\n", seg->procs[i]->name, SPU_ADDR_NTOP_CANON(&seg->procs[i]->proc_addr), Conf_id_to_str(seg->procs[i]->id, idstr));
  
  c += snprintf(c, SPCLAMP(c, e), "\n");

  Alarmp(SPLOG_DEBUG, CONF_SYS, "config_parse.y:convert_segment_to_string: The segment string is %d characters long:\n'%s'\n", (int) (c - segstr), segstr);
  
  if (c >= e)
    yyerror("Configuration string too long! (1)");
  
  return (size_t) (c - segstr);
}

/********************************************************************************
 ********************************************************************************/

static size_t convert_version_to_string(char * const segstr, const int segstr_size)
{
  char *c = segstr;
  char *e = segstr + segstr_size;

  c += snprintf(segstr, SPCLAMP(c, e), "Version: %d\nProtocol: %s\n",
                SP_MAJOR_VERSION, (Conf_get_accelerated_ring() ? "Accelerated Ring" : "Original Ring"));
  
  Alarmp(SPLOG_DEBUG, CONF_SYS, "config_parse.y:convert_version_to_string: The version string is %d characters long:\n'%s'\n", (int) (c - segstr), segstr);
  
  if (c >= e)
    yyerror("Configuration string too long! (2)");

  return (size_t) (c - segstr);
}

/********************************************************************************
 ********************************************************************************/

%}
%start Config
%token SEGMENT EVENTLOGFILE EVENTTIMESTAMP EVENTPRECISETIMESTAMP EVENTPRIORITY IPADDR IPPORT NUMBER COLON SEMICOLON
%token PDEBUG PINFO PWARNING PERROR PCRITICAL PFATAL
%token OPENBRACE CLOSEBRACE EQUALS STRING
%token DEBUGFLAGS BANG
%token DDEBUG DEXIT DPRINT DDATA_LINK DNETWORK DPROTOCOL DSESSION
%token DCONF DMEMB DFLOW_CONTROL DSTATUS DEVENTS DGROUPS DMEMORY
%token DSKIPLIST DACM DSECURITY DALL DNONE
%token DANGEROUSMONITOR SOCKETPORTREUSE RUNTIMEDIR SPUSER SPGROUP ALLOWEDAUTHMETHODS REQUIREDAUTHMETHODS ACCESSCONTROLPOLICY
%token MAXSESSIONMESSAGES ACTIVEIPVERSION
%token WINDOW PERSONALWINDOW ACCELERATEDRING ACCELERATEDWINDOW
%token TOKENTIMEOUT HURRYTIMEOUT ALIVETIMEOUT JOINTIMEOUT REPTIMEOUT SEGTIMEOUT GATHERTIMEOUT FORMTIMEOUT LOOKUPTIMEOUT
%token SP_BOOL SP_TRIVAL
%token IMONITOR ICLIENT IDAEMON
%token VIRTUALID
%token WIDEAREANETWORK
%%
Config		:	ConfigStructs
                        {
                          if (segments == 0)
                            yyerror("No segments specified!");

                          if (num_procs == 0)
                            yyerror("No daemons specified!");

                          if (Conf_get_personal_window() > Conf_get_window())
                            yyerror("PersonalWindow > Window!");

                          if (Conf_get_accelerated_ring() && Conf_get_accelerated_window() > Conf_get_personal_window())
                            yyerror("AcceleratedWindow > PersonalWindow!");

			  Config->num_segments    = segments;
			  Config->num_total_procs = num_procs;

                          /* add Spread daemon version number and algorithm to hash string and calculate hash */
                          
                          ConfStringLen    += convert_version_to_string(&ConfStringRep[ConfStringLen], MAX_CONF_STRING - ConfStringLen);
                          Config->hash_code = stdhcode_oaat(ConfStringRep, ConfStringLen + 1);

                          Alarmp(SPLOG_DEBUG, CONF_SYS, "Full hash string is %d %d characters long:\n'%s'\n", ConfStringLen, (int) strlen(ConfStringRep), ConfStringRep);
                          Alarmp(SPLOG_INFO,  CONF_SYS, "Hash value for this configuration is: %u\n", (unsigned) Config->hash_code);
			  Alarmp(SPLOG_INFO,  CONF_SYS, "Finished configuration file.\n");
			}

ConfigStructs	:	ConfigStructs SegmentStruct 
		|	ConfigStructs ParamStruct 
		|
		;

AlarmBit	:	DDEBUG { $$ = $1; }
		|	DEXIT { $$ = $1; }
		|	DPRINT { $$ = $1; }
		|	DDATA_LINK { $$ = $1; }
		|	DNETWORK { $$ = $1; }
		|	DPROTOCOL { $$ = $1; }
		|	DSESSION { $$ = $1; }
		|	DCONF { $$ = $1; }
		|	DMEMB { $$ = $1; }
		|	DFLOW_CONTROL { $$ = $1; }
		|	DSTATUS { $$ = $1; }
		|	DEVENTS { $$ = $1; }
		|	DGROUPS { $$ = $1; }
		|	DMEMORY { $$ = $1; }
		|	DSKIPLIST { $$ = $1; }
		|	DACM { $$ = $1; }
		|	DSECURITY { $$ = $1; }
		|	DALL { $$ = $1; }
		|	DNONE { $$ = $1; }
		;

AlarmBitComp	:	AlarmBitComp AlarmBit
			{
			  $$.mask = ($1.mask | $2.mask);
			}
		|	AlarmBitComp BANG AlarmBit
			{
			  $$.mask = ($1.mask & ~($3.mask));
			}
		|	{ $$.mask = NONE; }
		;
PriorityLevel   :       PDEBUG { $$ = $1; }
                |       PINFO { $$ = $1; }
                |       PWARNING { $$ = $1; }
                |       PERROR { $$ = $1; }
                |       PCRITICAL { $$ = $1; }
                |       PFATAL { $$ = $1; }
                ;

ParamStruct	:	DEBUGFLAGS EQUALS OPENBRACE AlarmBitComp CLOSEBRACE
			{
			  if (! Alarm_get_interactive() ) {
                            Alarm_clear_types(ALL);
			    Alarm_set_types($4.mask);
			    Alarm(CONF_SYS, "Set Alarm mask to: %x\n", Alarm_get_types());
                          }
			}
                |       EVENTPRIORITY EQUALS PriorityLevel
                        {
                            if (! Alarm_get_interactive() ) {
                                Alarm_set_priority($3.number);
                            }
                        }

		|	EVENTLOGFILE EQUALS STRING
			{
			  if (! Alarm_get_interactive() ) {
                            char file_buf[MAXPATHLEN];
                            expand_filename(file_buf, MAXPATHLEN, $3.string);
                            Alarm_set_output(file_buf);
                          }
                          free($3.string);
			}
		|	EVENTTIMESTAMP EQUALS STRING
			{
			  if (! Alarm_get_interactive() ) {
                              strncpy(alarm_format, $3.string, MAX_ALARM_FORMAT);
                              alarm_format[MAX_ALARM_FORMAT - 1] = 0;
                              alarm_custom_format = 1;
                              if (alarm_precise) {
                                  Alarm_enable_timestamp_high_res(alarm_format);
                              } else {
                                  Alarm_enable_timestamp(alarm_format);
                              }
                          }
                          free($3.string);
			}
		|	EVENTTIMESTAMP
			{
			  if (! Alarm_get_interactive() ) {
                              if (alarm_precise) {
                                  Alarm_enable_timestamp_high_res(NULL);
                              } else {
                                  Alarm_enable_timestamp(NULL);
                              }
                          }
			}
		|	EVENTPRECISETIMESTAMP
			{
			  if (! Alarm_get_interactive() ) {
                              alarm_precise = 1;
                              if (alarm_custom_format) {
                                  Alarm_enable_timestamp_high_res(alarm_format);
                              } else {
                                  Alarm_enable_timestamp_high_res(NULL);
                              }
                          }
			}
		|	DANGEROUSMONITOR EQUALS SP_BOOL
			{
			  if (! Alarm_get_interactive() ) {
                            Conf_set_dangerous_monitor_state($3.boolean);
                          }
			}
                |       SOCKETPORTREUSE EQUALS SP_TRIVAL
                        {
                            port_reuse state;
                            
                            if ($3.number == 1)
                              state = PORT_REUSE_ON;

                            else if ($3.number == 0)
                              state = PORT_REUSE_OFF;

                            else
                              state = PORT_REUSE_AUTO;

                            Conf_set_port_reuse_type(state);
                        }
                |       RUNTIMEDIR EQUALS STRING
                        {
                            Conf_set_runtime_dir($3.string);
                            free($3.string);
                        }
                |       SPUSER EQUALS STRING
                        {
                            Conf_set_user($3.string);
                            free($3.string);
                        }
                |       SPGROUP EQUALS STRING
                        {
                            Conf_set_group($3.string);
                            free($3.string);
                        }
                |       ALLOWEDAUTHMETHODS EQUALS STRING
                        {
                            char auth_list[MAX_AUTH_LIST_LEN];
                            int i, len;
                            char *c_ptr;
                            if (!authentication_configured) {
                                Acm_auth_set_disabled("NULL");
                            }
                            authentication_configured = 1;

                            strncpy(auth_list, $3.string, MAX_AUTH_LIST_LEN);
                            auth_list[MAX_AUTH_LIST_LEN - 1] = 0;
                            len = strlen(auth_list); 
                            for (i=0; i < len; )
                            {
                                c_ptr = strchr(&auth_list[i], ' ');
                                if (c_ptr != NULL)
                                {
                                    *c_ptr = '\0';
                                }
                                Acm_auth_set_enabled(&auth_list[i]);    
                                i += strlen(&auth_list[i]);
                                i++; /* for null */
                            }
                            free($3.string);
                        }
                |       REQUIREDAUTHMETHODS EQUALS STRING
                        {
                            char auth_list[MAX_AUTH_LIST_LEN];
                            int i, len;
                            char *c_ptr;
                            if (!authentication_configured) {
                                Acm_auth_set_disabled("NULL");
                            }
                            authentication_configured = 1;

                            strncpy(auth_list, $3.string, MAX_AUTH_LIST_LEN);
                            auth_list[MAX_AUTH_LIST_LEN - 1] = 0;
                            len = strlen(auth_list); 
                            for (i=0; i < len; )
                            {
                                c_ptr = strchr(&auth_list[i], ' ');
                                if (c_ptr != NULL)
                                {
                                    *c_ptr = '\0';
                                } 
                                Acm_auth_set_required(&auth_list[i]);    
                                i += strlen(&auth_list[i]);
                                i++; /* for null */
                            }
                            free($3.string);
                        }
                |       ACCESSCONTROLPOLICY EQUALS STRING
                        {
                            int ret;
                            ret = Acm_acp_set_policy($3.string);
                            if (!ret)
                            {
                                    yyerror("Invalid Access Control Policy name. Make sure it is spelled right and any needed mocdules are loaded");
                            }
                            free($3.string);
                        }
		|	MAXSESSIONMESSAGES EQUALS NUMBER
			{
                            Conf_set_max_session_messages($3.number);
			}
                |       ACTIVEIPVERSION EQUALS STRING
                        {
                          int ipver = 0;
                          
                          if (!strcmp("IPv4", $3.string))
                            ipver = AF_INET;
                          
                          else if (!strcmp("IPv6", $3.string))
                            ipver = AF_INET6;
                          
                          else
                            yyerror("Invalid ActiveIPVersion specified.  Value must be 'IPv4' or 'IPv6'");
                          
                          Alarmp(SPLOG_INFO, CONF_SYS, "Setting active IP version to %d\n", ipver);
                          
                          Conf_set_active_ip_version(ipver);
                          free($3.string);
                        }
 		|	WINDOW EQUALS NUMBER
			{
			    Conf_set_window($3.number);
			}
		|	PERSONALWINDOW EQUALS NUMBER
			{
			    Conf_set_personal_window($3.number);
			}
		|	ACCELERATEDRING EQUALS SP_BOOL
			{
			    Conf_set_accelerated_ring_flag(TRUE);
			    Conf_set_accelerated_ring($3.boolean);
			}
		|	ACCELERATEDWINDOW EQUALS NUMBER
			{
			    Conf_set_accelerated_window($3.number);
			}
		|	TOKENTIMEOUT EQUALS NUMBER
			{
			    Conf_set_token_timeout($3.number);
			}
		|	HURRYTIMEOUT EQUALS NUMBER
			{
			    Conf_set_hurry_timeout($3.number);
			}
		|	ALIVETIMEOUT EQUALS NUMBER
			{
			    Conf_set_alive_timeout($3.number);
			}
		|	JOINTIMEOUT EQUALS NUMBER
			{
			    Conf_set_join_timeout($3.number);
			}
		|	REPTIMEOUT EQUALS NUMBER
			{
			    Conf_set_rep_timeout($3.number);
			}
		|	SEGTIMEOUT EQUALS NUMBER
			{
			    Conf_set_seg_timeout($3.number);
			}
		|	GATHERTIMEOUT EQUALS NUMBER
			{
			    Conf_set_gather_timeout($3.number);
			}
		|	FORMTIMEOUT EQUALS NUMBER
			{
			    Conf_set_form_timeout($3.number);
			}
		|	LOOKUPTIMEOUT EQUALS NUMBER
			{
			    Conf_set_lookup_timeout($3.number);
			}
		|	WIDEAREANETWORK EQUALS SP_BOOL
			{
                            Conf_set_wide_area_network_flag(TRUE);
			    Conf_set_wide_area_network($3.boolean);
			}
                

SegmentStruct	:	SEGMENT SegmentAddress SegmentAddresses_opt OPENBRACE Segmentparams CLOSEBRACE
			{
                          int16u port = spu_addr_ip_get_port(&$2.addr.ipaddr);
                          int    i, j;

                          if (port == 0)
                            port = DEFAULT_SPREAD_PORT;
                            
                          if (segments >= MAX_SEGMENTS)                            
                            yyerror("Too many segments configured!");

                          /* NOTE: we left seg_addrs[0] open in SegmentAddresses_opt rule below */

                          assert(segment_addrs < MAX_ADDRS_SEGMENT);
                          Config->segments[segments].seg_addrs[0]  = $2.addr.ipaddr;
                          Config->segments[segments].num_seg_addrs = ++segment_addrs;
                          
                          for (i = 0; i < segment_addrs;++i)
                          {
                            if (spu_addr_family(&Config->segments[segments].seg_addrs[i]) != Conf_get_active_ip_version())
                              Alarmp(SPLOG_FATAL, CONF_SYS, PARSE_ERROR_STR "Segment address %s incompatible with ActiveIPVersion!",
                                     line_num, yytext, SPU_ADDR_NTOP(&Config->segments[segments].seg_addrs[i]));
                            
                            if (spu_addr_ip_is_unspecified(&Config->segments[segments].seg_addrs[i]))
                              Alarmp(SPLOG_FATAL, CONF_SYS, PARSE_ERROR_STR "Segment configured with an unspecified segment address %s!",
                                     line_num, yytext, SPU_ADDR_NTOP(&Config->segments[segments].seg_addrs[i]));
                                     
                            if (spu_addr_family(&Config->segments[segments].seg_addrs[i]) == AF_INET6 &&
                                !spu_addr_ip_is_multicast(&Config->segments[segments].seg_addrs[i]))
                              Alarmp(SPLOG_WARNING, CONF_SYS, PARSE_WARNING_STR "IPv6 segment configured with a non-multicast segment address %s!\n",
                                     line_num, yytext, SPU_ADDR_NTOP(&Config->segments[segments].seg_addrs[i]));
                            
                            if (spu_addr_ip_is_loopback(&Config->segments[segments].seg_addrs[i]))
                              Alarmp(SPLOG_WARNING, CONF_SYS, PARSE_WARNING_STR "Segment configured with a loopback address %s!",
                                     line_num, yytext, SPU_ADDR_NTOP(&Config->segments[segments].seg_addrs[i]));
                            
                            /* NOTE: we don't try to validate IPv4 broadcast addresses here because we don't know the umask */

                            spu_addr_ip_set_port(&Config->segments[segments].seg_addrs[i], port);  /* NOTE: set all segment addresses to use same port */
                          }

			  Config->segments[segments].num_procs = segment_procs;

			  Alarmp(SPLOG_INFO, CONF_SYS, "Successfully configured Segment %d [%s]:%u with %d procs:\n",
                                 segments, SPU_ADDR_NTOP(&$2.addr.ipaddr), (unsigned) port, segment_procs);

                          /* set ports for daemon communications */
                          
			  for (i = num_procs - segment_procs; i < num_procs; ++i)
                          {
                            spu_addr_ip_set_port(&Config->allprocs[i].proc_addr, port);

                            for (j = 0; j < Config->allprocs[i].num_if; ++j)
                              spu_addr_ip_set_port(&Config->allprocs[i].ifc[j].ifaddr, port);

                            Alarmp(SPLOG_INFO, CONF_SYS, "\t%" QQ(MAX_PROC_NAME) "s: %s\n", Config->allprocs[i].name, SPU_ADDR_NTOP(&Config->allprocs[i].proc_addr));

                            /* ensure .proc_addr uniqueness */
                            
                            for (j = 0; j < i; ++j)
                              if (!spu_addr_ip_cmp(&Config->allprocs[i].proc_addr, &Config->allprocs[j].proc_addr, TRUE))
                                Alarmp(SPLOG_FATAL, CONF_SYS, PARSE_ERROR_STR "Daemon's (%s) address (%s) not unique! Already in use by daemon #%d (%s)!\n",
                                       line_num, yytext, Config->allprocs[i].name, SPU_ADDR_NTOP(&Config->allprocs[i].proc_addr), j, Config->allprocs[j].name);
                          }

                          ConfStringLen += convert_segment_to_string(&ConfStringRep[ConfStringLen], MAX_CONF_STRING - ConfStringLen, &Config->segments[segments]);

			  ++segments;
			  segment_procs = 0;
                          segment_addrs = 0;

                          free($2.addr.ipstr);
			}
		;

SegmentAddress	:	IPADDR { $$ = $1; }
		|	IPPORT { $$ = $1; }
		;

SegmentAddresses_opt	:	SegmentAddresses_opt IPADDR
				{
                                  if (++segment_addrs >= MAX_ADDRS_SEGMENT)            /* NOTE: pre-increment here leaves index 0 open for primary segment address above */
                                    yyerror("Too many segment addresses specified!");

                                  Config->segments[segments].seg_addrs[segment_addrs] = $2.addr.ipaddr;
                                  free($2.addr.ipstr);
                                }
			|
			;

Segmentparams	:	Segmentparams Daemonparam 
		|
		;

Daemonparam	:	Daemonname Daemonvid_opt Daemoninterfaces_opt Semicolon_opt
			{
                          char idstr[MAX_ID_SIZE];
                          int i;

                          if (num_procs >= MAX_PROCS_RING)
                            yyerror("Too many daemons configured!");

                          if (segment_procs >= MAX_PROCS_SEGMENT)
                            yyerror("Too many daemons configured in segment!");

                          /* auto-generate a VID if not configured */
                          
                          if (Config->allprocs[num_procs].id == 0)
                          {
                            Config->allprocs[num_procs].id = Conf_str_to_id(Config->allprocs[num_procs].name);
                            
                            Alarmp(SPLOG_INFO, CONF_SYS, "Auto-generated virtual ID = '%s' for daemon '%s'\n",
                                   Conf_id_to_str(Config->allprocs[num_procs].id, idstr), Config->allprocs[num_procs].name);
                          }

                          /* ensure VID uniqueness */
                          
                          for (i = 0; i < num_procs; ++i)
                            if (Config->allprocs[i].id == Config->allprocs[num_procs].id)
                              Alarmp(SPLOG_FATAL, CONF_SYS, "The virtual ID '%s' of '%s' is already in use by '%s'!  "
                                     "You will probably need to explicitly reconfigure the daemons' virtual IDs so that they don't conflict.\n",
                                     Conf_id_to_str(Config->allprocs[num_procs].id, idstr), Config->allprocs[num_procs].name, Config->allprocs[i].name);
                          
                          if (procs_interfaces == 0)
                          {
                            procs_interfaces                          = 1;
                            Config->allprocs[num_procs].ifc[0].iftype = (IFTYPE_ALL | IFTYPE_ANY);
                            Config->allprocs[num_procs].ifc[0].ifaddr = Config->allprocs[num_procs].proc_addr;
                          }
                          
                          for (i = 0; i < procs_interfaces; ++i)
                            if ((!spu_addr_ip_cmp(&Config->allprocs[num_procs].proc_addr, &Config->allprocs[num_procs].ifc[i].ifaddr, FALSE) ||
                                 (spu_addr_family(&Config->allprocs[num_procs].proc_addr) == spu_addr_family(&Config->allprocs[num_procs].ifc[i].ifaddr) &&
                                   spu_addr_ip_is_unspecified(&Config->allprocs[num_procs].ifc[i].ifaddr)))
                                &&
                                (Is_IfType_Daemon(Config->allprocs[num_procs].ifc[i].iftype) ||
                                 Is_IfType_Any(Config->allprocs[num_procs].ifc[i].iftype)))
                              break;

                          if (i == procs_interfaces)
                            yyerror("Interface spec doesn't include a daemon entry for the daemon's primary address!");

                          Config->allprocs[num_procs].seg_index           = segments;
			  Config->allprocs[num_procs].index_in_seg        = segment_procs;
                          Config->allprocs[num_procs].num_if              = procs_interfaces;
			  Config->segments[segments].procs[segment_procs] = &Config->allprocs[num_procs];

                          ++num_procs;
                          ++segment_procs;
                          procs_interfaces = 0;
                        }
		;

/* Daemonname causes Daemonparam to be ambiguous (shift/reduce conflict) because a STRING followed by an IPADDR could either mean one daemon or two. */
/* This grammaar will interpret it as one daemon spec.  We allow this ambiguity so that a configuration of only IPADDRs can be accepted. */
/* If someone wants STRING IPADDR to actually mean two daemon specs instead of one, then they can separate them with a semicolon: */
/* STRING ; IPADDR will be interpeted as two daemon specs.  A vid or interface spec will also ensure two daemon specs are parsed.  */

Daemonname	:	STRING IPADDR { set_daemon_name_and_addr($1.string, &$2.addr); free($1.string);     free($2.addr.ipstr); }
		|	STRING        { set_daemon_name_and_addr($1.string, NULL);     free($1.string);                          }
		|	IPADDR        { set_daemon_name_and_addr(NULL,      &$1.addr); free($1.addr.ipstr);                      }
		;


Daemonvid_opt	:	VIRTUALID EQUALS NUMBER
			{
                          char idstr[MAX_ID_SIZE];

                          if ($3.number <= 0)
                            yyerror("Virtual IDs must be specified as an integer greater than zero!");
                          
                          Config->allprocs[num_procs].id = $3.number;

                          if (Config->allprocs[num_procs].id <= 0)
                            yyerror("Explicity configured virtual ID mapped to a non-positive somehow?!");
                          
                          Alarmp(SPLOG_INFO, CONF_SYS, "Explicit virtual ID = '%s' for daemon '%s'\n",
                                 Conf_id_to_str(Config->allprocs[num_procs].id, idstr), Config->allprocs[num_procs].name);
			}
		|
		;

Daemoninterfaces_opt	:	OPENBRACE Interfaceparams CLOSEBRACE
                                {
                                  if (procs_interfaces == 0)  /* NOTE: indicate error this way rather than hard parsing error to give better error message */
                                    yyerror("Interfaces section declared but no actual interfaces specified!");
                                }
			|
			;

Interfaceparams	:	Interfaceparams Interfaceparam
		|	
		;

Interfaceparam	:	IfTypeComp IPADDR
			{
                          if (procs_interfaces >= MAX_INTERFACES_PROC)
                            yyerror("Too many interfaces configured on a daemon!");

                          if (spu_addr_ip_get_port(&$2.addr.ipaddr) != 0)
                            yyerror("Interface IPs can't be specified with a port!");
                          
                          Config->allprocs[num_procs].ifc[procs_interfaces].ifaddr = $2.addr.ipaddr;
                          
                          if ($1.mask == 0)
                            Config->allprocs[num_procs].ifc[procs_interfaces].iftype = IFTYPE_ALL;
                          else 
                            Config->allprocs[num_procs].ifc[procs_interfaces].iftype = $1.mask;

                          ++procs_interfaces;
                          free($2.addr.ipstr);
			}
		;

IfTypeComp	:       IfTypeComp IfType
			{
			  $$.mask = ($1.mask | $2.mask);
			}
		|	{ $$.mask = 0; }
		;

IfType  	:	IMONITOR { $$ = $1; }
		|	ICLIENT { $$ = $1; }
		|	IDAEMON { $$ = $1; }
		;

Semicolon_opt	:	SEMICOLON
		|
		;
%%

void yyerror(const char *str)
{
  Alarmp(SPLOG_FATAL, CONF_SYS, PARSE_ERROR_STR "%s\n", line_num, yytext, str);
}

void yywarn(const char *str)
{
  Alarmp(SPLOG_WARNING, CONF_SYS, PARSE_WARNING_STR "%s\n", line_num, yytext, str);
}
