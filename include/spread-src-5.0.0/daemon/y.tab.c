/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SEGMENT = 258,
     EVENTLOGFILE = 259,
     EVENTTIMESTAMP = 260,
     EVENTPRECISETIMESTAMP = 261,
     EVENTPRIORITY = 262,
     IPADDR = 263,
     IPPORT = 264,
     NUMBER = 265,
     COLON = 266,
     SEMICOLON = 267,
     PDEBUG = 268,
     PINFO = 269,
     PWARNING = 270,
     PERROR = 271,
     PCRITICAL = 272,
     PFATAL = 273,
     OPENBRACE = 274,
     CLOSEBRACE = 275,
     EQUALS = 276,
     STRING = 277,
     DEBUGFLAGS = 278,
     BANG = 279,
     DDEBUG = 280,
     DEXIT = 281,
     DPRINT = 282,
     DDATA_LINK = 283,
     DNETWORK = 284,
     DPROTOCOL = 285,
     DSESSION = 286,
     DCONF = 287,
     DMEMB = 288,
     DFLOW_CONTROL = 289,
     DSTATUS = 290,
     DEVENTS = 291,
     DGROUPS = 292,
     DMEMORY = 293,
     DSKIPLIST = 294,
     DACM = 295,
     DSECURITY = 296,
     DALL = 297,
     DNONE = 298,
     DANGEROUSMONITOR = 299,
     SOCKETPORTREUSE = 300,
     RUNTIMEDIR = 301,
     SPUSER = 302,
     SPGROUP = 303,
     ALLOWEDAUTHMETHODS = 304,
     REQUIREDAUTHMETHODS = 305,
     ACCESSCONTROLPOLICY = 306,
     MAXSESSIONMESSAGES = 307,
     ACTIVEIPVERSION = 308,
     WINDOW = 309,
     PERSONALWINDOW = 310,
     ACCELERATEDRING = 311,
     ACCELERATEDWINDOW = 312,
     TOKENTIMEOUT = 313,
     HURRYTIMEOUT = 314,
     ALIVETIMEOUT = 315,
     JOINTIMEOUT = 316,
     REPTIMEOUT = 317,
     SEGTIMEOUT = 318,
     GATHERTIMEOUT = 319,
     FORMTIMEOUT = 320,
     LOOKUPTIMEOUT = 321,
     SP_BOOL = 322,
     SP_TRIVAL = 323,
     IMONITOR = 324,
     ICLIENT = 325,
     IDAEMON = 326,
     VIRTUALID = 327,
     WIDEAREANETWORK = 328
   };
#endif
/* Tokens.  */
#define SEGMENT 258
#define EVENTLOGFILE 259
#define EVENTTIMESTAMP 260
#define EVENTPRECISETIMESTAMP 261
#define EVENTPRIORITY 262
#define IPADDR 263
#define IPPORT 264
#define NUMBER 265
#define COLON 266
#define SEMICOLON 267
#define PDEBUG 268
#define PINFO 269
#define PWARNING 270
#define PERROR 271
#define PCRITICAL 272
#define PFATAL 273
#define OPENBRACE 274
#define CLOSEBRACE 275
#define EQUALS 276
#define STRING 277
#define DEBUGFLAGS 278
#define BANG 279
#define DDEBUG 280
#define DEXIT 281
#define DPRINT 282
#define DDATA_LINK 283
#define DNETWORK 284
#define DPROTOCOL 285
#define DSESSION 286
#define DCONF 287
#define DMEMB 288
#define DFLOW_CONTROL 289
#define DSTATUS 290
#define DEVENTS 291
#define DGROUPS 292
#define DMEMORY 293
#define DSKIPLIST 294
#define DACM 295
#define DSECURITY 296
#define DALL 297
#define DNONE 298
#define DANGEROUSMONITOR 299
#define SOCKETPORTREUSE 300
#define RUNTIMEDIR 301
#define SPUSER 302
#define SPGROUP 303
#define ALLOWEDAUTHMETHODS 304
#define REQUIREDAUTHMETHODS 305
#define ACCESSCONTROLPOLICY 306
#define MAXSESSIONMESSAGES 307
#define ACTIVEIPVERSION 308
#define WINDOW 309
#define PERSONALWINDOW 310
#define ACCELERATEDRING 311
#define ACCELERATEDWINDOW 312
#define TOKENTIMEOUT 313
#define HURRYTIMEOUT 314
#define ALIVETIMEOUT 315
#define JOINTIMEOUT 316
#define REPTIMEOUT 317
#define SEGTIMEOUT 318
#define GATHERTIMEOUT 319
#define FORMTIMEOUT 320
#define LOOKUPTIMEOUT 321
#define SP_BOOL 322
#define SP_TRIVAL 323
#define IMONITOR 324
#define ICLIENT 325
#define IDAEMON 326
#define VIRTUALID 327
#define WIDEAREANETWORK 328




/* Copy the first part of user declarations.  */
#line 1 "./config_parse.y"

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



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 559 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   157

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  74
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  20
/* YYNRULES -- Number of rules.  */
#define YYNRULES  88
/* YYNRULES -- Number of states.  */
#define YYNSTATES  152

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   328

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     8,    11,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    53,    57,    58,    60,
      62,    64,    66,    68,    70,    76,    80,    84,    88,    90,
      92,    96,   100,   104,   108,   112,   116,   120,   124,   128,
     132,   136,   140,   144,   148,   152,   156,   160,   164,   168,
     172,   176,   180,   184,   188,   195,   197,   199,   202,   203,
     206,   207,   212,   215,   217,   219,   223,   224,   228,   229,
     232,   233,   236,   239,   240,   242,   244,   246,   248
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      75,     0,    -1,    76,    -1,    76,    81,    -1,    76,    80,
      -1,    -1,    25,    -1,    26,    -1,    27,    -1,    28,    -1,
      29,    -1,    30,    -1,    31,    -1,    32,    -1,    33,    -1,
      34,    -1,    35,    -1,    36,    -1,    37,    -1,    38,    -1,
      39,    -1,    40,    -1,    41,    -1,    42,    -1,    43,    -1,
      78,    77,    -1,    78,    24,    77,    -1,    -1,    13,    -1,
      14,    -1,    15,    -1,    16,    -1,    17,    -1,    18,    -1,
      23,    21,    19,    78,    20,    -1,     7,    21,    79,    -1,
       4,    21,    22,    -1,     5,    21,    22,    -1,     5,    -1,
       6,    -1,    44,    21,    67,    -1,    45,    21,    68,    -1,
      46,    21,    22,    -1,    47,    21,    22,    -1,    48,    21,
      22,    -1,    49,    21,    22,    -1,    50,    21,    22,    -1,
      51,    21,    22,    -1,    52,    21,    10,    -1,    53,    21,
      22,    -1,    54,    21,    10,    -1,    55,    21,    10,    -1,
      56,    21,    67,    -1,    57,    21,    10,    -1,    58,    21,
      10,    -1,    59,    21,    10,    -1,    60,    21,    10,    -1,
      61,    21,    10,    -1,    62,    21,    10,    -1,    63,    21,
      10,    -1,    64,    21,    10,    -1,    65,    21,    10,    -1,
      66,    21,    10,    -1,    73,    21,    67,    -1,     3,    82,
      83,    19,    84,    20,    -1,     8,    -1,     9,    -1,    83,
       8,    -1,    -1,    84,    85,    -1,    -1,    86,    87,    88,
      93,    -1,    22,     8,    -1,    22,    -1,     8,    -1,    72,
      21,    10,    -1,    -1,    19,    89,    20,    -1,    -1,    89,
      90,    -1,    -1,    91,     8,    -1,    91,    92,    -1,    -1,
      69,    -1,    70,    -1,    71,    -1,    12,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   324,   324,   351,   352,   353,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   377,   381,   385,   387,   388,
     389,   390,   391,   392,   395,   403,   410,   419,   433,   443,
     454,   460,   475,   480,   485,   490,   516,   542,   552,   556,
     574,   578,   582,   587,   591,   595,   599,   603,   607,   611,
     615,   619,   623,   627,   634,   709,   710,   713,   721,   724,
     725,   728,   792,   793,   794,   798,   813,   816,   821,   824,
     825,   828,   848,   852,   855,   856,   857,   860,   861
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SEGMENT", "EVENTLOGFILE",
  "EVENTTIMESTAMP", "EVENTPRECISETIMESTAMP", "EVENTPRIORITY", "IPADDR",
  "IPPORT", "NUMBER", "COLON", "SEMICOLON", "PDEBUG", "PINFO", "PWARNING",
  "PERROR", "PCRITICAL", "PFATAL", "OPENBRACE", "CLOSEBRACE", "EQUALS",
  "STRING", "DEBUGFLAGS", "BANG", "DDEBUG", "DEXIT", "DPRINT",
  "DDATA_LINK", "DNETWORK", "DPROTOCOL", "DSESSION", "DCONF", "DMEMB",
  "DFLOW_CONTROL", "DSTATUS", "DEVENTS", "DGROUPS", "DMEMORY", "DSKIPLIST",
  "DACM", "DSECURITY", "DALL", "DNONE", "DANGEROUSMONITOR",
  "SOCKETPORTREUSE", "RUNTIMEDIR", "SPUSER", "SPGROUP",
  "ALLOWEDAUTHMETHODS", "REQUIREDAUTHMETHODS", "ACCESSCONTROLPOLICY",
  "MAXSESSIONMESSAGES", "ACTIVEIPVERSION", "WINDOW", "PERSONALWINDOW",
  "ACCELERATEDRING", "ACCELERATEDWINDOW", "TOKENTIMEOUT", "HURRYTIMEOUT",
  "ALIVETIMEOUT", "JOINTIMEOUT", "REPTIMEOUT", "SEGTIMEOUT",
  "GATHERTIMEOUT", "FORMTIMEOUT", "LOOKUPTIMEOUT", "SP_BOOL", "SP_TRIVAL",
  "IMONITOR", "ICLIENT", "IDAEMON", "VIRTUALID", "WIDEAREANETWORK",
  "$accept", "Config", "ConfigStructs", "AlarmBit", "AlarmBitComp",
  "PriorityLevel", "ParamStruct", "SegmentStruct", "SegmentAddress",
  "SegmentAddresses_opt", "Segmentparams", "Daemonparam", "Daemonname",
  "Daemonvid_opt", "Daemoninterfaces_opt", "Interfaceparams",
  "Interfaceparam", "IfTypeComp", "IfType", "Semicolon_opt", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    74,    75,    76,    76,    76,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    77,    77,    77,    77,    77,
      77,    77,    77,    77,    77,    78,    78,    78,    79,    79,
      79,    79,    79,    79,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    81,    82,    82,    83,    83,    84,
      84,    85,    86,    86,    86,    87,    87,    88,    88,    89,
      89,    90,    91,    91,    92,    92,    92,    93,    93
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     2,     0,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     3,     0,     1,     1,
       1,     1,     1,     1,     5,     3,     3,     3,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     6,     1,     1,     2,     0,     2,
       0,     4,     2,     1,     1,     3,     0,     3,     0,     2,
       0,     2,     2,     0,     1,     1,     1,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       5,     0,     2,     1,     0,     0,    38,    39,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     4,     3,    65,    66,    68,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    37,
      28,    29,    30,    31,    32,    33,    35,    27,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    67,    70,     0,     0,    34,     0,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    74,    64,
      73,    69,    76,    26,    72,     0,    78,     0,    80,    88,
      75,    83,    87,    71,    77,    79,     0,    81,    84,    85,
      86,    82
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,   127,   104,    76,    34,    35,    38,    67,
     105,   131,   132,   136,   139,   141,   145,   146,   151,   143
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -51
static const yytype_int16 yypact[] =
{
     -51,     5,    -3,   -51,     6,    19,    43,   -51,    44,    45,
      52,    53,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   -51,   -51,   -51,   -51,   -51,    -6,
      50,    -5,     0,   -50,    40,   100,   101,   102,   103,   104,
     105,   111,   106,   119,   120,    64,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,    86,    -1,   -51,   -51,
     -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,
     -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,
     -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,
     -51,   -51,   -51,   -51,    51,    87,   -51,    -4,   -51,   -51,
     -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,
     -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,
     134,   -51,    71,   -51,   -51,   133,   132,   135,   -51,   136,
     -51,   137,   -51,   -51,   -51,   -51,    -2,   -51,   -51,   -51,
     -51,   -51
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -51,   -51,   -51,    37,   -51,   -51,   -51,   -51,   -51,   -51,
     -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51,   -51
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
       4,     5,     6,     7,     8,     3,   147,   102,    70,    71,
      72,    73,    74,    75,    36,    37,    68,    78,   103,    77,
       9,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      39,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    40,    41,    42,   148,   149,   150,
      33,   106,    69,    43,    44,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   128,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,   129,    79,   130,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    86,    80,    81,    82,    83,    84,    85,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   134,   135,   133,   140,     0,     0,   142,     0,
       0,   138,     0,   101,   137,     0,     0,   144
};

static const yytype_int8 yycheck[] =
{
       3,     4,     5,     6,     7,     0,     8,     8,    13,    14,
      15,    16,    17,    18,     8,     9,    22,    67,    19,    19,
      23,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      21,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    21,    21,    21,    69,    70,    71,
      73,    20,    22,    21,    21,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,     8,    21,    21,    21,    21,
      21,    21,    21,    21,    21,    21,    21,    20,    68,    22,
      21,    21,    21,    21,    21,    21,    21,    21,    21,    21,
      21,    10,    22,    22,    22,    22,    22,    22,    22,    10,
      10,    67,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,     8,    72,   107,    10,    -1,    -1,    12,    -1,
      -1,    19,    -1,    67,    21,    -1,    -1,    20
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    75,    76,     0,     3,     4,     5,     6,     7,    23,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    73,    80,    81,     8,     9,    82,    21,
      21,    21,    21,    21,    21,    21,    21,    21,    21,    21,
      21,    21,    21,    21,    21,    21,    21,    21,    21,    21,
      21,    21,    21,    21,    21,    21,    21,    83,    22,    22,
      13,    14,    15,    16,    17,    18,    79,    19,    67,    68,
      22,    22,    22,    22,    22,    22,    10,    22,    10,    10,
      67,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    67,     8,    19,    78,    84,    20,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    77,     8,    20,
      22,    85,    86,    77,     8,    72,    87,    21,    19,    88,
      10,    89,    12,    93,    20,    90,    91,     8,    69,    70,
      71,    92
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 325 "./config_parse.y"
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
    break;

  case 6:
#line 356 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 7:
#line 357 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 8:
#line 358 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 9:
#line 359 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 10:
#line 360 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 11:
#line 361 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 12:
#line 362 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 13:
#line 363 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 14:
#line 364 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 15:
#line 365 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 16:
#line 366 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 17:
#line 367 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 18:
#line 368 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 19:
#line 369 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 20:
#line 370 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 21:
#line 371 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 22:
#line 372 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 23:
#line 373 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 24:
#line 374 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 25:
#line 378 "./config_parse.y"
    {
			  (yyval).mask = ((yyvsp[(1) - (2)]).mask | (yyvsp[(2) - (2)]).mask);
			}
    break;

  case 26:
#line 382 "./config_parse.y"
    {
			  (yyval).mask = ((yyvsp[(1) - (3)]).mask & ~((yyvsp[(3) - (3)]).mask));
			}
    break;

  case 27:
#line 385 "./config_parse.y"
    { (yyval).mask = NONE; }
    break;

  case 28:
#line 387 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 29:
#line 388 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 30:
#line 389 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 31:
#line 390 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 32:
#line 391 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 33:
#line 392 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 34:
#line 396 "./config_parse.y"
    {
			  if (! Alarm_get_interactive() ) {
                            Alarm_clear_types(ALL);
			    Alarm_set_types((yyvsp[(4) - (5)]).mask);
			    Alarm(CONF_SYS, "Set Alarm mask to: %x\n", Alarm_get_types());
                          }
			}
    break;

  case 35:
#line 404 "./config_parse.y"
    {
                            if (! Alarm_get_interactive() ) {
                                Alarm_set_priority((yyvsp[(3) - (3)]).number);
                            }
                        }
    break;

  case 36:
#line 411 "./config_parse.y"
    {
			  if (! Alarm_get_interactive() ) {
                            char file_buf[MAXPATHLEN];
                            expand_filename(file_buf, MAXPATHLEN, (yyvsp[(3) - (3)]).string);
                            Alarm_set_output(file_buf);
                          }
                          free((yyvsp[(3) - (3)]).string);
			}
    break;

  case 37:
#line 420 "./config_parse.y"
    {
			  if (! Alarm_get_interactive() ) {
                              strncpy(alarm_format, (yyvsp[(3) - (3)]).string, MAX_ALARM_FORMAT);
                              alarm_format[MAX_ALARM_FORMAT - 1] = 0;
                              alarm_custom_format = 1;
                              if (alarm_precise) {
                                  Alarm_enable_timestamp_high_res(alarm_format);
                              } else {
                                  Alarm_enable_timestamp(alarm_format);
                              }
                          }
                          free((yyvsp[(3) - (3)]).string);
			}
    break;

  case 38:
#line 434 "./config_parse.y"
    {
			  if (! Alarm_get_interactive() ) {
                              if (alarm_precise) {
                                  Alarm_enable_timestamp_high_res(NULL);
                              } else {
                                  Alarm_enable_timestamp(NULL);
                              }
                          }
			}
    break;

  case 39:
#line 444 "./config_parse.y"
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
    break;

  case 40:
#line 455 "./config_parse.y"
    {
			  if (! Alarm_get_interactive() ) {
                            Conf_set_dangerous_monitor_state((yyvsp[(3) - (3)]).boolean);
                          }
			}
    break;

  case 41:
#line 461 "./config_parse.y"
    {
                            port_reuse state;
                            
                            if ((yyvsp[(3) - (3)]).number == 1)
                              state = PORT_REUSE_ON;

                            else if ((yyvsp[(3) - (3)]).number == 0)
                              state = PORT_REUSE_OFF;

                            else
                              state = PORT_REUSE_AUTO;

                            Conf_set_port_reuse_type(state);
                        }
    break;

  case 42:
#line 476 "./config_parse.y"
    {
                            Conf_set_runtime_dir((yyvsp[(3) - (3)]).string);
                            free((yyvsp[(3) - (3)]).string);
                        }
    break;

  case 43:
#line 481 "./config_parse.y"
    {
                            Conf_set_user((yyvsp[(3) - (3)]).string);
                            free((yyvsp[(3) - (3)]).string);
                        }
    break;

  case 44:
#line 486 "./config_parse.y"
    {
                            Conf_set_group((yyvsp[(3) - (3)]).string);
                            free((yyvsp[(3) - (3)]).string);
                        }
    break;

  case 45:
#line 491 "./config_parse.y"
    {
                            char auth_list[MAX_AUTH_LIST_LEN];
                            int i, len;
                            char *c_ptr;
                            if (!authentication_configured) {
                                Acm_auth_set_disabled("NULL");
                            }
                            authentication_configured = 1;

                            strncpy(auth_list, (yyvsp[(3) - (3)]).string, MAX_AUTH_LIST_LEN);
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
                            free((yyvsp[(3) - (3)]).string);
                        }
    break;

  case 46:
#line 517 "./config_parse.y"
    {
                            char auth_list[MAX_AUTH_LIST_LEN];
                            int i, len;
                            char *c_ptr;
                            if (!authentication_configured) {
                                Acm_auth_set_disabled("NULL");
                            }
                            authentication_configured = 1;

                            strncpy(auth_list, (yyvsp[(3) - (3)]).string, MAX_AUTH_LIST_LEN);
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
                            free((yyvsp[(3) - (3)]).string);
                        }
    break;

  case 47:
#line 543 "./config_parse.y"
    {
                            int ret;
                            ret = Acm_acp_set_policy((yyvsp[(3) - (3)]).string);
                            if (!ret)
                            {
                                    yyerror("Invalid Access Control Policy name. Make sure it is spelled right and any needed mocdules are loaded");
                            }
                            free((yyvsp[(3) - (3)]).string);
                        }
    break;

  case 48:
#line 553 "./config_parse.y"
    {
                            Conf_set_max_session_messages((yyvsp[(3) - (3)]).number);
			}
    break;

  case 49:
#line 557 "./config_parse.y"
    {
                          int ipver = 0;
                          
                          if (!strcmp("IPv4", (yyvsp[(3) - (3)]).string))
                            ipver = AF_INET;
                          
                          else if (!strcmp("IPv6", (yyvsp[(3) - (3)]).string))
                            ipver = AF_INET6;
                          
                          else
                            yyerror("Invalid ActiveIPVersion specified.  Value must be 'IPv4' or 'IPv6'");
                          
                          Alarmp(SPLOG_INFO, CONF_SYS, "Setting active IP version to %d\n", ipver);
                          
                          Conf_set_active_ip_version(ipver);
                          free((yyvsp[(3) - (3)]).string);
                        }
    break;

  case 50:
#line 575 "./config_parse.y"
    {
			    Conf_set_window((yyvsp[(3) - (3)]).number);
			}
    break;

  case 51:
#line 579 "./config_parse.y"
    {
			    Conf_set_personal_window((yyvsp[(3) - (3)]).number);
			}
    break;

  case 52:
#line 583 "./config_parse.y"
    {
			    Conf_set_accelerated_ring_flag(TRUE);
			    Conf_set_accelerated_ring((yyvsp[(3) - (3)]).boolean);
			}
    break;

  case 53:
#line 588 "./config_parse.y"
    {
			    Conf_set_accelerated_window((yyvsp[(3) - (3)]).number);
			}
    break;

  case 54:
#line 592 "./config_parse.y"
    {
			    Conf_set_token_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 55:
#line 596 "./config_parse.y"
    {
			    Conf_set_hurry_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 56:
#line 600 "./config_parse.y"
    {
			    Conf_set_alive_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 57:
#line 604 "./config_parse.y"
    {
			    Conf_set_join_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 58:
#line 608 "./config_parse.y"
    {
			    Conf_set_rep_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 59:
#line 612 "./config_parse.y"
    {
			    Conf_set_seg_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 60:
#line 616 "./config_parse.y"
    {
			    Conf_set_gather_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 61:
#line 620 "./config_parse.y"
    {
			    Conf_set_form_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 62:
#line 624 "./config_parse.y"
    {
			    Conf_set_lookup_timeout((yyvsp[(3) - (3)]).number);
			}
    break;

  case 63:
#line 628 "./config_parse.y"
    {
                            Conf_set_wide_area_network_flag(TRUE);
			    Conf_set_wide_area_network((yyvsp[(3) - (3)]).boolean);
			}
    break;

  case 64:
#line 635 "./config_parse.y"
    {
                          int16u port = spu_addr_ip_get_port(&(yyvsp[(2) - (6)]).addr.ipaddr);
                          int    i, j;

                          if (port == 0)
                            port = DEFAULT_SPREAD_PORT;
                            
                          if (segments >= MAX_SEGMENTS)                            
                            yyerror("Too many segments configured!");

                          /* NOTE: we left seg_addrs[0] open in SegmentAddresses_opt rule below */

                          assert(segment_addrs < MAX_ADDRS_SEGMENT);
                          Config->segments[segments].seg_addrs[0]  = (yyvsp[(2) - (6)]).addr.ipaddr;
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
                                 segments, SPU_ADDR_NTOP(&(yyvsp[(2) - (6)]).addr.ipaddr), (unsigned) port, segment_procs);

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

                          free((yyvsp[(2) - (6)]).addr.ipstr);
			}
    break;

  case 65:
#line 709 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 66:
#line 710 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 67:
#line 714 "./config_parse.y"
    {
                                  if (++segment_addrs >= MAX_ADDRS_SEGMENT)            /* NOTE: pre-increment here leaves index 0 open for primary segment address above */
                                    yyerror("Too many segment addresses specified!");

                                  Config->segments[segments].seg_addrs[segment_addrs] = (yyvsp[(2) - (2)]).addr.ipaddr;
                                  free((yyvsp[(2) - (2)]).addr.ipstr);
                                }
    break;

  case 71:
#line 729 "./config_parse.y"
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
    break;

  case 72:
#line 792 "./config_parse.y"
    { set_daemon_name_and_addr((yyvsp[(1) - (2)]).string, &(yyvsp[(2) - (2)]).addr); free((yyvsp[(1) - (2)]).string);     free((yyvsp[(2) - (2)]).addr.ipstr); }
    break;

  case 73:
#line 793 "./config_parse.y"
    { set_daemon_name_and_addr((yyvsp[(1) - (1)]).string, NULL);     free((yyvsp[(1) - (1)]).string);                          }
    break;

  case 74:
#line 794 "./config_parse.y"
    { set_daemon_name_and_addr(NULL,      &(yyvsp[(1) - (1)]).addr); free((yyvsp[(1) - (1)]).addr.ipstr);                      }
    break;

  case 75:
#line 799 "./config_parse.y"
    {
                          char idstr[MAX_ID_SIZE];

                          if ((yyvsp[(3) - (3)]).number <= 0)
                            yyerror("Virtual IDs must be specified as an integer greater than zero!");
                          
                          Config->allprocs[num_procs].id = (yyvsp[(3) - (3)]).number;

                          if (Config->allprocs[num_procs].id <= 0)
                            yyerror("Explicity configured virtual ID mapped to a non-positive somehow?!");
                          
                          Alarmp(SPLOG_INFO, CONF_SYS, "Explicit virtual ID = '%s' for daemon '%s'\n",
                                 Conf_id_to_str(Config->allprocs[num_procs].id, idstr), Config->allprocs[num_procs].name);
			}
    break;

  case 77:
#line 817 "./config_parse.y"
    {
                                  if (procs_interfaces == 0)  /* NOTE: indicate error this way rather than hard parsing error to give better error message */
                                    yyerror("Interfaces section declared but no actual interfaces specified!");
                                }
    break;

  case 81:
#line 829 "./config_parse.y"
    {
                          if (procs_interfaces >= MAX_INTERFACES_PROC)
                            yyerror("Too many interfaces configured on a daemon!");

                          if (spu_addr_ip_get_port(&(yyvsp[(2) - (2)]).addr.ipaddr) != 0)
                            yyerror("Interface IPs can't be specified with a port!");
                          
                          Config->allprocs[num_procs].ifc[procs_interfaces].ifaddr = (yyvsp[(2) - (2)]).addr.ipaddr;
                          
                          if ((yyvsp[(1) - (2)]).mask == 0)
                            Config->allprocs[num_procs].ifc[procs_interfaces].iftype = IFTYPE_ALL;
                          else 
                            Config->allprocs[num_procs].ifc[procs_interfaces].iftype = (yyvsp[(1) - (2)]).mask;

                          ++procs_interfaces;
                          free((yyvsp[(2) - (2)]).addr.ipstr);
			}
    break;

  case 82:
#line 849 "./config_parse.y"
    {
			  (yyval).mask = ((yyvsp[(1) - (2)]).mask | (yyvsp[(2) - (2)]).mask);
			}
    break;

  case 83:
#line 852 "./config_parse.y"
    { (yyval).mask = 0; }
    break;

  case 84:
#line 855 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 85:
#line 856 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 86:
#line 857 "./config_parse.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;


/* Line 1267 of yacc.c.  */
#line 2648 "y.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 863 "./config_parse.y"


void yyerror(const char *str)
{
  Alarmp(SPLOG_FATAL, CONF_SYS, PARSE_ERROR_STR "%s\n", line_num, yytext, str);
}

void yywarn(const char *str)
{
  Alarmp(SPLOG_WARNING, CONF_SYS, PARSE_WARNING_STR "%s\n", line_num, yytext, str);
}

