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
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#ifndef ARCH_PC_WIN95
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <net/if.h>
#else
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  define YY_NO_UNISTD_H  /* Do not include unistd.h header in windows compiles */
#endif

#include "conf_body.h"
#include "y.tab.h"
#include "spu_alarm.h"

/********************************************************************************
 ********************************************************************************/

extern int  line_num;
extern int  semantic_errors;
extern void yyerror(const char *str);
extern void yywarn(const char *str);

/********************************************************************************
 ********************************************************************************/

static int parse_ip(YYSTYPE *val, const char *ipstr, int family, bool hasport)
{
  struct in_addr  ipv4addr;
  struct in6_addr ipv6addr;
  char           *cpystr;
  char           *addrstr;
  void           *addrptr = NULL;
  int             hasif   = FALSE;
  char           *ifstr   = NULL;
  long            port    = 0;
  char           *portstr = NULL;
  char           *endstr;
  int             tmp;

  if ((cpystr = addrstr = strdup(ipstr)) == NULL)
    yyerror("parse_ip: strdup failed!");

  switch (family)
  {
  case AF_INET:  addrptr = &ipv4addr;                                                    break;
  case AF_INET6: addrptr = &ipv6addr; hasif = ((ifstr = strchr(addrstr, '%')) != NULL);  break;
  default:       yyerror("parse_ip: Unexpected address family?!");                       break;
  }
  
  if (hasport)
  {
    if ((portstr = strrchr(cpystr, ':')) == NULL)
      yyerror("parse_ip: Missing ':' char in port spec?!");

    errno = 0;
    port  = strtol(portstr + 1, &endstr, 10);

    if (port < 0 || port > 65535 || errno == ERANGE)
      yyerror("parse_ip: Port out of valid range!");
    
    if (errno || endstr == portstr + 1 || *endstr != 0)
      yyerror("parse_ip: Failure parsing port?!");
    
    *portstr = 0;           /* cut off port specification */

    if (family == AF_INET6)
    {
      *(portstr - 1) = 0;   /* cut off enclosing bracket */
      ++addrstr;            /* skip over opening bracket */
    }
  }

  if (hasif)
    *ifstr = 0;             /* cut off interface specification */
  
  if ((tmp = inet_pton(family, addrstr, addrptr)) == 0)
  {
    yywarn("parse_ip: Potentially malformed IP address scanned!");
    free(cpystr);
    return -1;
  }
  else if (tmp != 1)
    yyerror("parse_ip: Error parsing IP address!");

  memset(val, 0, sizeof(*val));

  if (family == AF_INET)
  {
    val->addr.ipaddr.ipv4.sin_family = AF_INET;
    val->addr.ipaddr.ipv4.sin_addr   = ipv4addr;
    val->addr.ipaddr.ipv4.sin_port   = htons((int16u) port);
#ifdef HAVE_SIN_LEN_IN_STRUCT_SOCKADDR_IN
    val->addr.ipaddr.ipv4.sin_len    = sizeof(val->addr.ipaddr.ipv4);
#endif    
  }
  else
  {
    val->addr.ipaddr.ipv6.sin6_family = AF_INET6;
    val->addr.ipaddr.ipv6.sin6_addr   = ipv6addr;
    val->addr.ipaddr.ipv6.sin6_port   = htons((int16u) port);
#ifdef HAVE_SIN6_LEN_IN_STRUCT_SOCKADDR_IN6
    val->addr.ipaddr.ipv6.sin6_len    = sizeof(val->addr.ipaddr.ipv6);
#endif    

    if (hasif)
    {
      val->addr.ipaddr.ipv6.sin6_scope_id = if_nametoindex(ifstr + 1);
      *ifstr = '%';           /* restore interface specification */
    }
    
    if (hasport)
      *(portstr - 1) = ']';   /* restore enclosing bracket */
  }

  if (hasport)
    *portstr = ':';           /* restore port specification */

  val->addr.ipstr = cpystr;

  return 0;
}

/********************************************************************************
 ********************************************************************************/

/* NOTE: string pattern doesn't allow brackets '[' ']' to prevent an ipv6 interface spec from trying to consume a trailing port specification */

%}
qstring  \"[^\"]*\"|\'[^\']*\'
string   [^ \t\r\n\v\f#{}\[\]]+
true     [Tt][Rr][Uu][Ee]
yes      [Yy][Ee][Ss]
on       [Oo][Nn]
false    [Ff][Aa][Ll][Ss][Ee]
no       [Nn][Oo]
off      [Oo][Ff][Ff]
auto     [Aa][Uu][Tt][Oo]
ipv4addr [0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}
ipv4port {ipv4addr}:[0-9]{1,5}
quad     [0-9A-Fa-f]{1,4}
ipv6addr ({quad}(:{quad}){7}|(({quad}:){0,6}{quad})?::({quad}(:{quad}){0,6})?)(%{string})?
ipv6port "["{ipv6addr}"]":[0-9]{1,5}
%option noyywrap
%%
#.*                             {} /* Comments */
[ \t\r]                         {} /* White space */
\n                              { line_num++;}
"{"                             { return OPENBRACE; }
"}"                             { return CLOSEBRACE; }
"="                             { return EQUALS; }
":"                             { return COLON; }
";"                             { return SEMICOLON; }
!/.+                            { return BANG; }
Spread_Segment                  { return SEGMENT; }
EventLogFile                    { return EVENTLOGFILE; }
EventTimeStamp                  { return EVENTTIMESTAMP; }
EventPreciseTimeStamp           { return EVENTPRECISETIMESTAMP; }
EventPriority                   { return EVENTPRIORITY; }
DebugFlags                      { return DEBUGFLAGS; }
DangerousMonitor                { return DANGEROUSMONITOR; }
SocketPortReuse                 { return SOCKETPORTREUSE; }
RuntimeDir                      { return RUNTIMEDIR; }
DaemonUser                      { return SPUSER; }
DaemonGroup                     { return SPGROUP; }
RequiredAuthMethods             { return REQUIREDAUTHMETHODS; }
AllowedAuthMethods              { return ALLOWEDAUTHMETHODS; }
AccessControlPolicy             { return ACCESSCONTROLPOLICY; }
MaxSessionMessages              { return MAXSESSIONMESSAGES; }
ActiveIPVersion                 { return ACTIVEIPVERSION; }
Window                          { return WINDOW; }
PersonalWindow                  { return PERSONALWINDOW; }
AcceleratedRing                 { return ACCELERATEDRING; }
AcceleratedWindow               { return ACCELERATEDWINDOW; }
TokenTimeout                    { return TOKENTIMEOUT; }
HurryTimeout                    { return HURRYTIMEOUT; }
AliveTimeout                    { return ALIVETIMEOUT; }
JoinTimeout                     { return JOINTIMEOUT; }
RepTimeout                      { return REPTIMEOUT; }
SegTimeout                      { return SEGTIMEOUT; }
GatherTimeout                   { return GATHERTIMEOUT; }
FormTimeout                     { return FORMTIMEOUT; }
LookupTimeout                   { return LOOKUPTIMEOUT; }
VirtualID                       { return VIRTUALID; }
WideAreaNetwork                 { return WIDEAREANETWORK; }
{true}|{yes}                    { yylval.boolean = TRUE;        return SP_BOOL; }
{false}|{no}                    { yylval.boolean = FALSE;       return SP_BOOL; }
{on}                            { yylval.number = 1;            return SP_TRIVAL; }
{off}                           { yylval.number = 0;            return SP_TRIVAL; }
{auto}                          { yylval.number = 2;            return SP_TRIVAL; }
ALL                             { yylval.mask = ALL;            return DALL; }
EXIT                            { yylval.mask = EXIT;           return DEXIT; }
DATA_LINK                       { yylval.mask = DATA_LINK;      return DDATA_LINK; }
MEMORY                          { yylval.mask = MEMORY;         return DMEMORY; }
EVENTS                          { yylval.mask = EVENTS;         return DEVENTS; }
PRINT                           { yylval.mask = PRINT;          return DPRINT; }
NETWORK                         { yylval.mask = NETWORK;        return DNETWORK; }
PROTOCOL                        { yylval.mask = PROTOCOL;       return DPROTOCOL; }
SESSION                         { yylval.mask = SESSION;        return DSESSION; }
CONFIGURATION                   { yylval.mask = CONF_SYS;       return DCONF; }
MEMBERSHIP                      { yylval.mask = MEMB;           return DMEMB; }
FLOW_CONTROL                    { yylval.mask = FLOW_CONTROL;   return DFLOW_CONTROL; }
STATUS                          { yylval.mask = STATUS;         return DSTATUS; }
DEBUG                           { yylval.mask = DEBUG;          return DDEBUG; }
GROUPS                          { yylval.mask = GROUPS;         return DGROUPS; }
SKIPLIST                        { yylval.mask = SKIPLIST;       return DSKIPLIST; }
ACM                             { yylval.mask = ACM;            return DACM; }
SECURITY                        { yylval.mask = SECURITY;       return DSECURITY; }
NONE                            { yylval.mask = NONE;           return DNONE; }
M                               { yylval.mask = IFTYPE_MONITOR; return IMONITOR; }
C                               { yylval.mask = IFTYPE_CLIENT;  return ICLIENT; }
D                               { yylval.mask = IFTYPE_DAEMON;  return IDAEMON; }
pDEBUG                          { yylval.number = 1;            return PDEBUG; }
INFO                            { yylval.number = 2;            return PINFO; }
WARNING                         { yylval.number = 3;            return PWARNING; }
ERROR                           { yylval.number = 4;            return PERROR; }
CRITICAL                        { yylval.number = 5;            return PCRITICAL; }
FATAL                           { yylval.number = 6;            return PFATAL; }
{ipv4addr}                      { if (parse_ip(&yylval, yytext, AF_INET,  FALSE)) { REJECT; } return IPADDR; }
{ipv4port}                      { if (parse_ip(&yylval, yytext, AF_INET,  TRUE))  { REJECT; } return IPPORT; }
{ipv6addr}                      { if (parse_ip(&yylval, yytext, AF_INET6, FALSE)) { REJECT; } return IPADDR; }
{ipv6port}                      { if (parse_ip(&yylval, yytext, AF_INET6, TRUE))  { REJECT; } return IPPORT; }
[0-9]{1,9}                      { yylval.number = atoi(yytext); return NUMBER; }
{qstring}                       { if ((yylval.string = strdup(yytext + 1)) == NULL) { yyerror("strdup failed!"); } yylval.string[strlen(yylval.string) - 1] = 0; return STRING; }
{string}                        { if ((yylval.string = strdup(yytext)) == NULL) { yyerror("strdup failed!"); } return STRING; }
%%
