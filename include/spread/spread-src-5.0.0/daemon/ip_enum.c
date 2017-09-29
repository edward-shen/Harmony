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

#include <string.h>

#ifndef ARCH_PC_WIN95
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <net/if.h>
#  if defined(HAVE_IFADDRS_H)
#    include <ifaddrs.h>     /* NOTE: this has to come after <net/if.h> on some systems! */
#  endif
#  include <netdb.h>
#  include <sys/ioctl.h>
#  include <netinet/in.h>
#  ifdef sun
#    include <sys/sockio.h>  /* for SIOCGIFCONF */
#  endif
#else
#  include <winsock2.h>
#  include <iphlpapi.h>
#endif

#include "spu_alarm.h"
#include "spu_addr.h"
#include "ip_enum.h"

#define MAX_IF 1024     /* max interfaces to look up */
#define MIN_IF 16       /* must be able to look up at least this many */

#ifdef ARCH_PC_WIN95
typedef INTERFACE_INFO If_info;
#else
typedef struct ifreq If_info;
#  define closesocket(s) close(s)
#endif

#if !defined(_SIZEOF_ADDR_IFREQ)
#  if defined(HAVE_SA_LEN_IN_SOCKADDR)
#    define _SIZEOF_ADDR_IFREQ(ifr) ((ifr).ifr_addr.sa_len > sizeof(struct sockaddr) ? (sizeof(struct ifreq) + (ifr).ifr_addr.sa_len - sizeof(struct sockaddr)) : sizeof(struct ifreq))
#  else
#    define _SIZEOF_ADDR_IFREQ(ifr) sizeof(ifr)
#  endif
#endif

#define NEXT_IF(ifp) ((struct ifreq*) ((char*) (ifp) + _SIZEOF_ADDR_IFREQ(*(ifp))))

typedef struct 
{
  spu_addr *ips;
  size_t    size;
  size_t    next;
  
} ip_array;

/********************************************************************************
 ********************************************************************************/

static void add_ip(ip_array *array, const spu_addr *ip)
{
  if (array->next >= array->size)
  {
    size_t    new_size = (array->next < MIN_IF ? MIN_IF : 4 * array->next);
    spu_addr *new_ips  = realloc(array->ips, new_size * sizeof(spu_addr));

    if (new_ips == NULL)
    {
      Alarmp(SPLOG_ERROR, CONF_SYS, SPLOC ":add_ip: realloc failed!\n");
      return;
    }

    array->ips  = new_ips;
    array->size = new_size;
  }

  Alarmp(SPLOG_DEBUG, CONF_SYS, SPLOC ":add_ip: adding '%s'\n", SPU_ADDR_NTOP(ip));
  array->ips[array->next++] = *ip;
}

/********************************************************************************
 ********************************************************************************/

static void ip_enum_hostname(ip_array *array)
{
  char             name[256] = { 0 };
  struct addrinfo  hint      = { 0 };
  struct addrinfo *rslt;
  struct addrinfo *curr;
  spu_addr         addr;
  int              tmp;
  
  if (gethostname(name, sizeof(name)) || name[0] == 0)
  {
    Alarmp(SPLOG_ERROR, CONF_SYS, SPLOC ":ip_enum_hostname: gethostname() failed?! %d '%s'\n", sock_errno, sock_strerror(sock_errno));
    return;
  }

  if ((tmp = getaddrinfo(name, NULL, &hint, &rslt)))
  {
    Alarmp(SPLOG_ERROR, CONF_SYS, SPLOC ":ip_enum_hostname: getaddrinfo(%s) failed! %d '%s'\n", name, tmp, GAI_STRERROR(tmp));
    return;
  }
  
  for (curr = rslt; curr != NULL; curr = curr->ai_next)
  {
    if (curr->ai_addr == NULL || (curr->ai_addr->sa_family != AF_INET && curr->ai_addr->sa_family != AF_INET6))
      continue;

    if (spu_addr_from_sockaddr_known(&addr, curr->ai_addr))
      Alarmp(SPLOG_FATAL, CONF_SYS, SPLOC ": ip_enum_getifaddrs: BUG! spu_addr_from_sockaddr_known failed?!");
      
    add_ip(array, &addr);
  }

  if (rslt != NULL)
    freeaddrinfo(rslt);
}

/********************************************************************************
 ********************************************************************************/

static void ip_enum_socket(ip_array *array, int family)
{
  size_t   bufferSize = sizeof(If_info) * MAX_IF;
  void    *buffer;
  int      sock;
  spu_addr addr;

  if (family != AF_INET && family != AF_INET6)
    Alarmp(SPLOG_FATAL, CONF_SYS, SPLOC ":ip_enum_socket: BUG! Only AF_INET(%d) and AF_INET(%d) families supported! %d\n", family);
  
  if ((buffer = malloc(bufferSize)) == NULL)
  {
    Alarmp(SPLOG_ERROR, CONF_SYS, SPLOC ":ip_enum_socket: malloc failed!\n");
    goto FAIL;
  }

  if ((sock = socket(family, SOCK_DGRAM, IPPROTO_IP)) == INVALID_SOCKET)
  {
    Alarmp(SPLOG_ERROR, CONF_SYS, SPLOC ":ip_enum_socket: socket() failed: %d %d '%s'\n", sock, sock_errno, sock_strerror(sock_errno));
    goto FAIL_BUFFER;
  }
  
#ifdef ARCH_PC_WIN95
  {
    INTERFACE_INFO *ifr = (INTERFACE_INFO*) buffer;
    DWORD           n   = 0;

    /* TODO: probably want to use GetAdaptersAddresses instead as a parallel to getifaddrs (i.e. - move to ip_enum_getifaddrs) */
      
    if (WSAIoctl(sock, SIO_GET_INTERFACE_LIST, 0, 0, buffer, bufferSize, &n, 0, 0) == SOCKET_ERROR)
    {
      Alarmp(SPLOG_ERROR, CONF_SYS, SPLOC ":ip_enum_socket: WSAIoctl(SIO_GET_INTERFACE_LIST) failed: %d %s\n", sock_errno, sock_strerrno(sock_errno));
      goto FAIL_SOCK;
    }
    
    for (n /= sizeof(INTERFACE_INFO); n > 0; ++ifr, --n)
    {
      if (ifr->iiAddress.Address.sa_family != AF_INET && ifr->iiAddress.Address.sa_family != AF_INET6)
        continue;

      /* NOTE: we do this manually instead because windoze has two
       * kinds of sockaddr_in6's, sockaddr_in6 and sockaddr_in6_old,
       * and this might return the old kind
      
      if (spu_addr_from_sockaddr_known(&addr, &ifr->iiAddress.Address))
        Alarmp(SPLOG_FATAL, CONF_SYS, SPLOC ":ip_enum_socket: BUG! spu_addr_from_sockaddr_known failed?!\n");
      */
      
      memset(&addr, 0, sizeof(addr));

      switch (ifr->iiAddress.Address.sa_family)
      {
      case AF_INET:
        memcpy(&addr, &ifr->iiAddress.AddressIn,  sizeof(ifr->iiAddress.AddressIn));
        break;
        
      case AF_INET6:
        memcpy(&addr, &ifr->iiAddress.AddressIn6, sizeof(ifr->iiAddress.AddressIn6));
        break;
      }

      add_ip(array, &addr);
    }
  }
#else
  {
    struct ifconf ifc;
    struct ifreq *ifp;

    ifc.ifc_len = bufferSize;
    ifc.ifc_buf = buffer;

    while (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
    {      
      if (errno == ENOMEM && bufferSize / 2 >= MIN_IF * sizeof(struct ifreq))
      {
        bufferSize /= 2;
        
        ifc.ifc_len = bufferSize;
        ifc.ifc_buf = buffer;
        continue;
      }
      
      Alarmp(SPLOG_ERROR, CONF_SYS, SPLOC ":ip_enum_socket: ioctl(SIOCGIFCONF) error: %d = '%s'\n", sock_errno, sock_strerror(sock_errno));
      goto FAIL_SOCK;
    }

    /* NOTE: ensure ifp is always pointing at a full and valid ifr
     * (i.e. - handles partial address on last entry if buffer wasn't
     * big enough for all) */
    
    if (ifc.ifc_len > (int) sizeof(If_info) * MAX_IF)
      ifc.ifc_len = sizeof(If_info) * MAX_IF;
    
    for (ifp = ifc.ifc_req; (char*) NEXT_IF(ifp) <= ifc.ifc_buf + ifc.ifc_len; ifp = NEXT_IF(ifp))
    {
      if (ifp->ifr_addr.sa_family != AF_INET && ifp->ifr_addr.sa_family != AF_INET6)
        continue;

      if (spu_addr_from_sockaddr_known(&addr, &ifp->ifr_addr))
        Alarmp(SPLOG_FATAL, CONF_SYS, SPLOC ":ip_enum_socket: BUG! spu_addr_from_sockaddr_known failed?!\n");

      add_ip(array, &addr);
    }
  }
#endif

FAIL_SOCK:
  closesocket(sock);

FAIL_BUFFER:
  free(buffer);

FAIL:
  return;
}

/********************************************************************************
 ********************************************************************************/

static void ip_enum_getifaddrs(ip_array *array)
{
#if defined(HAVE_GETIFADDRS)
  
  struct ifaddrs *ifs;
  struct ifaddrs *curr;
  spu_addr        addr;
    
  if (getifaddrs(&ifs))
  {
    Alarmp(SPLOG_ERROR, CONF_SYS, SPLOC ": ip_enum_getifaddrs: getifaddrs failed: %d '%s'\n", errno, strerror(errno));
    return;
  }

  for (curr = ifs; curr != NULL; curr = curr->ifa_next)
  {
    if (curr->ifa_addr == NULL || (curr->ifa_addr->sa_family != AF_INET && curr->ifa_addr->sa_family != AF_INET6))
      continue;

    if (spu_addr_from_sockaddr_known(&addr, curr->ifa_addr))
      Alarmp(SPLOG_FATAL, CONF_SYS, SPLOC ": ip_enum_getifaddrs: BUG! spu_addr_from_sockaddr_known failed?!");

    /* FreeBSD fills in proper scope id, so don't override it */
    
#  if !defined(__FreeBSD__)
    if (curr->ifa_addr->sa_family == AF_INET6)
      addr.ipv6.sin6_scope_id = if_nametoindex(curr->ifa_name);
#  endif    
    
    add_ip(array, &addr);        
  }

  if (ifs)
    freeifaddrs(ifs);

#endif
}

/********************************************************************************
 ********************************************************************************/

spu_addr *ip_enum_local(size_t *num_ips)
{
  ip_array array = { 0 };

  Alarmp(SPLOG_DEBUG, CONF_SYS, SPLOC ":ip_enum_local: enumerating getifaddrs\n");
  ip_enum_getifaddrs(&array);

  Alarmp(SPLOG_DEBUG, CONF_SYS, SPLOC ":ip_enum_local: enumerating socket(AF_INET); %lu\n", (unsigned long) array.next);
  ip_enum_socket(&array, AF_INET);

  Alarmp(SPLOG_DEBUG, CONF_SYS, SPLOC ":ip_enum_local: enumerating socket(AF_INET6); %lu\n", (unsigned long) array.next);
  ip_enum_socket(&array, AF_INET6);
  
  Alarmp(SPLOG_DEBUG, CONF_SYS, SPLOC ":ip_enum_local: enumerating hostname; %lu\n", (unsigned long) array.next);
  ip_enum_hostname(&array);
  
  *num_ips = array.next;
  
  return array.ips;
}

/********************************************************************************
 ********************************************************************************/
