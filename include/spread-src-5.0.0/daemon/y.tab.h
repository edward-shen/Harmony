/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

