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

/* 
 *
 * $Id: conf_body.h 873 2016-11-22 19:56:33Z jschultz $
 */

#ifndef INC_CONF_BODY
#define INC_CONF_BODY

#include "arch.h"
#include "configuration.h"
#include "spread_params.h"

int		yyparse();
void            parser_init();

#undef  ext
#ifndef ext_conf_body
#define ext extern
#else
#define ext
#endif

ext     configuration	*Config;
ext     FILE		*yyin;

#define YYSTYPE YYSTYPE

typedef struct
{
  spu_addr    ipaddr;
  char       *ipstr;
  
} conf_ipaddr;

typedef union
{
  bool        boolean;
  int32       mask;
  int         number;
  conf_ipaddr addr;  
  char       *string;
} YYSTYPE;

extern YYSTYPE yylval;
extern int yysemanticerr;

#endif /* INC_CONF_BODY */
