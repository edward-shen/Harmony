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
#include <stdio.h>

#ifndef	ARCH_PC_WIN95

#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <netinet/tcp.h> 
#include <sys/un.h>
#include <sys/param.h>
#include <netdb.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>

#  ifdef HAVE_SYS_SELECT_H
#    include <sys/select.h>
#  endif

#  ifdef HAVE_POLL
#    include <poll.h>
#  endif

#else	/* ARCH_PC_WIN95 */

#include <winsock2.h>
#define ioctl   ioctlsocket

WSADATA	WSAData;

#endif	/* ARCH_PC_WIN95 */

#include "mutex.h"
#include "spu_events.h"
#include "spread_params.h"
#include "sess_types.h"
#include "spu_scatter.h"
#include "spu_alarm.h"
#include "spu_addr.h"
#include "acm.h"

/* SP functions need these types, but internal headers do not */
typedef struct dummy_vs_set_info {
        unsigned int num_members;
        unsigned int members_offset;  /* offset from beginning of msg body*/
} vs_set_info;

typedef struct dummy_membership_info {
        group_id     gid;
        char         changed_member[MAX_GROUP_NAME]; /* filled in on CAUSED_BY_JOIN, CAUSED_BY_LEAVE or CAUSED_BY_DISCONNECT only */
        unsigned int num_vs_sets;
        vs_set_info  my_vs_set;
} membership_info;

#include "sp_func.h"

#ifndef ARCH_PC_WIN95
#  define GAI_STRERROR(e) gai_strerror(e)
#  ifndef INVALID_SOCKET
#    define INVALID_SOCKET (-1)
#  elif INVALID_SOCKET != -1
#  endif
#else
#  define GAI_STRERROR(e) sock_strerror(e)  /* NOTE: gai_strerror is not thread safe on win32 */
#endif

enum sp_sess_state {
    SESS_UNUSED,
    SESS_ACTIVE,
    SESS_ERROR,
};

typedef	struct	dummy_sp_session {
        mutex_type recv_mutex;
        mutex_type send_mutex;
	mailbox	mbox;
        enum sp_sess_state state;
	char	private_group_name[MAX_GROUP_NAME];
        message_header  recv_saved_head;
        int     recv_message_saved;
} sp_session;

struct auth_method_info {
        char    name[MAX_AUTH_NAME];
        int     (*authenticate) (int, void *);
        void    *auth_data;
};

/* length of spread_name connect field is limited to 5 digit port # + '@' + hostname */
#define SPREAD_MAXCONNECT_NAMELEN       (MAXHOSTNAMELEN + 6)

/* default spread connection method depends on whether unix sockets are available */
#ifndef ARCH_PC_WIN95
#  define DEFAULT_SPREAD_CONNECTION      QQ(DEFAULT_SPREAD_PORT)
#else
#  define DEFAULT_SPREAD_CONNECTION      QQ(DEFAULT_SPREAD_PORT) "@localhost"
#endif	/* ARCH_PC_WIN95 */

static  int     sp_null_authenticate(int, void *);
static  struct auth_method_info Auth_Methods[MAX_AUTH_METHODS] = { {"NULL", sp_null_authenticate, NULL} };
static  int     Num_Reg_Auth_Methods = 1;

static once_type Init_once = ONCE_STATIC_INIT;

#ifdef _REENTRANT
static	mutex_type	Struct_mutex;

#endif /* def _REENTRANT */

#define MAX_LIB_SESSIONS       (0x1 << 10)                        /* must remain a power of 2 */
#define MBOX_TO_BASE_SES(mbox) ((mbox) & (MAX_LIB_SESSIONS - 1))  /* so this bit masking works */

static	int		Num_sessions = 0;
static	sp_session	Sessions[MAX_LIB_SESSIONS];

static	void    Flip_mess( message_header *head_ptr );
static	int	SP_get_session( mailbox mbox );

static  int     SP_connect_timeout_low( const char *spread_name, const char *private_name,
                                        int priority, int group_membership, mailbox *mbox,
                                        char *private_group, sp_time *time_out );

static	int	SP_internal_multicast( mailbox mbox, service service_type, 
				       int num_groups,
				       const char groups[][MAX_GROUP_NAME],
				       int16 mess_type,
				       const scatter *scat_mess );

/* wait_nointr waits for read or write readiness on one socket while
 * ignoring signal interruptions with an optional absolute timeout.
 *
 * Returns 0 on success when the socket is ready in time and non-zero
 * otherwise.
 *
 * Returns 1 on timeout, otherwise returns -1 with the specific error
 * indicated in sock_errno.
 */

static int wait_nointr( int sock, int readable, const sp_time *abs_time_out )
#ifdef HAVE_POLL
{
    int ret;
    int tmp;

    do
    {
        struct pollfd fd    = { 0 };
        int           to_ms = -1;
        
        fd.fd     = sock;
        fd.events = ( readable ? POLLIN : POLLOUT );

        if ( abs_time_out )
        {
            sp_time t = E_sub_time( *abs_time_out, E_get_time_monotonic() );

            if ( t.sec < 0 || t.usec < 0 )
            {
                ret = 1;
                break;
            }
            else if ( t.sec >= INT_MAX / 1000 )
                to_ms = INT_MAX;

            else
                to_ms = ( int ) ( t.sec * 1000 + ( t.usec + 999 ) / 1000 );
        }
    
        tmp = poll( &fd, 1, to_ms );

        switch ( tmp )
        {
        case 1:  ret =  0; break;
        case 0:  ret =  1; break;
        default: ret = -1; break;
        }
    }
    while ( tmp == -1 && sock_errno == EINTR );
    
    return ret;
}
#else
{
    int ret;
    int tmp;

    do
    {
        struct timeval sel_time_ptr = NULL;
        struct timeval sel_time;
        fd_set         sel_set;

        FD_ZERO( &sel_set );
        FD_SET( sock, &sel_set );

        if ( abs_time_out )
        {
            sp_time t = E_sub_time( *abs_time_out, E_get_time_monotonic() );

            if ( t.sec < 0 || t.usec < 0 )
            {
                ret = 1;
                break;
            }

            sel_time.tv_sec  = t.sec;
            sel_time.tv_usec = t.usec;
            sel_time_ptr     = &sel_time;
        }

        tmp = select( s + 1, ( readable ? &sel_set : NULL ), ( readable ? NULL : &sel_set ), NULL, sel_time_ptr );

        switch ( tmp )
        {
        case 1:  ret =  0; break;
        case 0:  ret =  1; break;
        default: ret = -1; break;
        }
    }
    while ( tmp == -1 && sock_errno == EINTR );
    
    return ret;
}
#endif

/* connect_nointr calls connect on a socket while ignoring signal
 * interruptions with an optional absolute timeout.
 *
 * Returns 0 on success when the socket is connected in time and
 * non-zero otherwise.
 * 
 * Returns 1 on timeout, otherwise returns -1 with the specific error
 * indicated in sock_errno.
 */

static int connect_nointr( int s, struct sockaddr *addr, socklen_t addr_len, sp_time *abs_time_out )
{
    int       ret;
    int       err;
    socklen_t err_len;

    if ( ( ret = connect( s, addr, addr_len ) ) != 0 && ( sock_errno == EINPROGRESS || sock_errno == EINTR || sock_errno == EAGAIN || sock_errno == EWOULDBLOCK ) )
    {
        ret = wait_nointr( s, 0, abs_time_out );

        switch ( ret )
        {
        case 0:   /* writeable */
            if ( ( ret = getsockopt( s, SOL_SOCKET, SO_ERROR, ( err = 0, &err ), ( err_len = sizeof( err ), &err_len ) ) ) == 0 && err != 0 )
            {
                sock_set_errno( err );
                ret = -1;
            }            
            break;
            
        case 1:   /* timed out */
            break;
            
        default:  /* error */
            break;
        }
    }

    return ret;
}

#if 0
/* recv_n_nointr calls recv repeatedly until either a total of exactly
 * n bytes have been received or an error occurs, all while ignoring
 * signal interruptions.
 * 
 * recv_n_nointr returns the remaining number of bytes left to recv.
 * On success, it returns 0.  On failure, the non-zero number of bytes
 * left to recv is returned and sock_errno indicates the error.
 *
 * If n is less than zero, then the call will immediately return n and
 * sock_errno will be set to EINVAL.  If n is zero, then the call will
 * immediately succeed and return 0.
 *
 * This function is meant to be used with blocking sockets.  If called
 * on a non-blocking socket, then a short read and a blocking error
 * can be returned.
 *
 * NOTE: We don't use MSG_WAITALL as there are reports of large
 * receives improperly blocking forever with this option on some
 * implementations when OS buffers are full.
 */

static int recv_n_nointr( int s, char *b, int n )
{
    int tmp;

    for ( ; n > 0; n -= tmp, b += tmp )
    {
        tmp = ( int ) recv( s, b, n, 0 );
    
        if ( tmp <= 0 )
        {
            if ( tmp == -1 && sock_errno == EINTR )
                tmp = 0;

            else
            {
                if ( tmp == 0 )
                    sock_set_errno( ECONNRESET );
                    
                break;
            }
        }
    }
  
    if ( n < 0 )
        sock_set_errno( EINVAL );  /* handle cases where n was initially negative or tmp > n */
  
    return n;
}

/* send_n_nointr calls send repeatedly until either a total of
 * exactly n bytes have been sent or an error occurs, all while
 * ignoring signal interruptions.
 * 
 * send_n_nointr returns the remaining number of bytes left to send.
 * On success, it returns 0.  On failure, the non-zero number of bytes
 * left to send is returned with sock_errno indicating the error.
 *
 * This function is meant to be used with blocking sockets.  If called
 * on a non-blocking socket, then a short send and a blocking error
 * can be returned.
 */

static int send_n_nointr( int s, char *b, int n )
{
    int tmp;

    for ( ; n > 0; n -= tmp, b += tmp )
    {
        tmp = ( int ) send( s, b, n, 0 );
      
        if ( tmp <= 0 )
        {
            if ( tmp == -1 && sock_errno == EINTR )
                tmp = 0;

            else
            {
                if ( tmp == 0 )
                    sock_set_errno( ECONNRESET );

                break;
            }
        }
    }
    
    if ( n < 0 )
        sock_set_errno( EINVAL );  /* handle cases where n was initially negative or tmp > n */
  
    return n;
}
#endif

/* recv_n_nointr_nb calls recv repeatedly until either a total of
 * exactly n bytes have been received, an error occurs, or an optional
 * absolute timeout expires, all while ignoring signal interruptions.
 *
 * recv_n_nointr_nb returns the remaining number of bytes left to
 * recv.  On success, it returns 0.  On failure, the non-zero number
 * of bytes left to recv is returned and sock_errno indicates the
 * error.  If the timeout expires before all n bytes could be
 * received, then the non-zero number of bytes left to recv is
 * returned and sock_errno will be set to EAGAIN.
 *
 * If n is less than zero, then the call will immediately return n and
 * sock_errno will be set to EINVAL.  If n is zero, then the call will
 * immediately succeed and return 0.
 *
 * abs_time_out can be NULL, in which case the timeout is infinite.
 *
 * This function is meant to be used with nonblocking sockets.  If
 * called on a blocking socket, then any timeout may not be respected
 * and the function can block for arbitrary periods of time.
 *
 * NOTE: We don't use MSG_WAITALL as there are reports of large
 * receives improperly blocking forever with this option on some
 * implementations when OS buffers are full.
 */

static int recv_n_nointr_nb( int s, char *b, int n, const sp_time *abs_time_out )
{
    int tmp;

    for ( ; n > 0; n -= tmp, b += tmp )
    {
        /* check if optional timeout has expired */
          
        if ( abs_time_out && E_compare_time( E_get_time_monotonic(), *abs_time_out ) >= 0 )
        {
            sock_set_errno( EAGAIN );
            break;
        }
          
        /* recv and handle error cases; fatal errors break loop, non-fatal errors set tmp = 0 and continue */
          
        tmp = ( int ) recv( s, b, n, 0 );

        if ( tmp <= 0 )
        {
            if ( tmp == -1 && ( sock_errno == EAGAIN || sock_errno == EWOULDBLOCK ) )
            {
                if ( ( tmp = wait_nointr( s, 1, abs_time_out ) ) != 0 )
                {
                    if ( tmp == 1 )
                        sock_set_errno( EAGAIN );
                        
                    break;
                }
            }
            else if ( tmp != -1 || sock_errno != EINTR )
            {
                if ( tmp == 0 )
                    sock_set_errno( ECONNRESET );

                break;
            }

            tmp = 0;
        }
    }

    if ( n < 0 )
        sock_set_errno( EINVAL );  /* handle cases where n was initially negative or tmp > n */

    return n;
}  

/* send_n_nointr_nb calls send repeatedly until either a total of
 * exactly n bytes have been sent, an error occurs, or an optional
 * absolute timeout expires, all while ignoring signal interruptions.
 *
 * send_n_nointr_nb returns the remaining number of bytes left to
 * send.  On success, it returns 0.  On failure, the non-zero number
 * of bytes left to send is returned and sock_errno indicates the
 * error.  If the timeout expires before all n bytes could be sent,
 * then the non-zero number of bytes left to send is returned and
 * sock_errno will be set to EAGAIN.  
 *
 * If n is less than zero, then the call will immediately return n and
 * sock_errno will be set to EINVAL.  If n is zero, then the call will
 * immediately succeed and return 0.
 *
 * abs_time_out can be NULL, in which case the timeout is infinite.
 *
 * This function is meant to be used with nonblocking sockets.  If
 * called on a blocking socket, then any timeout may not be respected
 * and the function can block for arbitrary periods of time.
 */

static int send_n_nointr_nb( int s, char *b, int n, const sp_time *abs_time_out )
{
    int tmp;

    for ( ; n > 0; n -= tmp, b += tmp )
    {
        /* check if optional timeout has expired */
          
        if ( abs_time_out && E_compare_time( E_get_time_monotonic(), *abs_time_out ) >= 0 )
        {
            sock_set_errno( EAGAIN );
            break;
        }
          
        /* send and handle error cases; fatal errors break loop, non-fatal errors set tmp = 0 and continue */
          
        tmp = (int) send( s, b, n, 0 );

        if ( tmp <= 0 )
        {
            if ( tmp == -1 && ( sock_errno == EAGAIN || sock_errno == EWOULDBLOCK ) )
            {
                if ( ( tmp = wait_nointr( s, 0, abs_time_out ) ) != 0 )
                {
                    if ( tmp == 1 )
                        sock_set_errno( EAGAIN );
                    
                    break;
                }
            }
            else if ( tmp != -1 || sock_errno != EINTR )
            {
                if ( tmp == 0 )
                    sock_set_errno( ECONNRESET );

                break;
            }

            tmp = 0;
        }
    }

    if ( n < 0 )
        sock_set_errno( EINVAL );  /* handle cases where n was initially negative or tmp > n */

    return n;
}  

/* This is a null authenticate method that does nothing */

static  int     sp_null_authenticate(int fd, void * auth_data)
{
        /* return success */
        return(1);
}
static  int     valid_auth_method(char *auth_method, char *auth_list, int auth_list_len)
{
        char *cur_p, *next_p;
        char list_str[MAX_AUTH_NAME * MAX_AUTH_METHODS];
        memcpy(list_str, auth_list, auth_list_len);

        list_str[auth_list_len] = '\0';
        cur_p = list_str;
        do {
                next_p = strchr(cur_p, ' ');
                if (next_p != NULL)
                        *next_p = '\0';
                if (!strcmp(auth_method, cur_p) )
                        return(1);
                if (next_p != NULL)
                        cur_p = next_p + 1;
                else
                        cur_p = NULL;
        } while (NULL != cur_p );
        /* didn't find the method in the list */
        return(0);
}

/* Lock and threading related functions */

/* reset all mutexes in child process.
 * This is strictly speaking, unsafe, because a parent thread may have 
 * a Mutex lock on the mailbox becaue it is reading from the socket
 * and once the locks in the child are reset, the child process could 
 * also attempt a read -- resulting in garbage in both processe.
 * However -- We document and require that spread mailboxes cannot be
 * read from different processes at the same time, so the application
 * is responsible for not using them this way. This restriction is true 
 * even without the mutex reset because the pthread mutexes' are not 
 * enforced accross processes, but only within the threads of one process.
 *
 * What can be done is spread connections can be passed by fork from
 * a parent process to a child and only used in one of the processes. 
 * that will now work correctly, even if a parent is also multithreaded.
 */
/* We take the Struct_mutex in prepare to prevent inconsistent state 
 * in Session[] structs in child
 */
/* NOTE: this needs work to be 100% correct (need to grab all locks or use semaphores or something) */
#ifdef _REENTRANT

#ifdef  HAVE_PTHREAD_ATFORK

static  void    sp_atfork_prepare(void)
{
        Mutex_lock( &Struct_mutex );
}

/* parent unlocks Struct_mutex when fork is complete */
static  void    sp_atfork_parent(void)
{
        Mutex_unlock( &Struct_mutex );
}

/* Child unlocks Struct_mutex, and also unlocks all session locks so
 * it can acces all of the connections.
 */
static  void    sp_atfork_child(void)
{
        int ses;

        Mutex_unlock( &Struct_mutex );
        for( ses=0; ses < MAX_LIB_SESSIONS; ++ses )
        {
            Mutex_unlock( &Sessions[ses].recv_mutex );
	    Mutex_unlock( &Sessions[ses].send_mutex );
        }
}

#endif  /* HAVE_PTHREAD_ATFORK */
#endif /* _REENTRANT */

static  void    sp_initialize(void)
{
        int ses;

#ifdef ENABLEDEBUG
        Alarm_set_types(SESSION | DEBUG);
        Alarm_set_priority(SPLOG_DEBUG);
#endif

	Mutex_init( &Struct_mutex );
	for( ses=0; ses < MAX_LIB_SESSIONS; ++ses )
	{
	        Mutex_init( &Sessions[ses].recv_mutex );
		Mutex_init( &Sessions[ses].send_mutex );
		Sessions[ses].mbox  = -1;
		Sessions[ses].state = SESS_UNUSED;
	}

	Mutex_atfork( sp_atfork_prepare, sp_atfork_parent, sp_atfork_child );

#ifndef ARCH_PC_WIN95
	signal( SIGPIPE, SIG_IGN );
#else	/* ARCH_PC_WIN95 */
	{
	        int ret = WSAStartup( MAKEWORD(2,0), &WSAData );

		if( ret != 0 ) Alarm( EXIT, "sp_initialize: winsock initialization error %d\n", ret );
	}
#endif	/* ARCH_PC_WIN95 */

        return;
}

/* Increase socket buffer size to 200Kb if possible.
 * Used in SP_connect family when connection is established.
 */
static void set_large_socket_buffers(int s)
{
    int i, on, ret;
    sockopt_len_t onlen;

    for( i=10; i <= 200; i+=5 )
    {
        on = 1024*i;

        ret = setsockopt( s, SOL_SOCKET, SO_SNDBUF, (void *)&on, 4);
        if (ret < 0 ) break;
        
        ret = setsockopt( s, SOL_SOCKET, SO_RCVBUF, (void *)&on, 4);
        if (ret < 0 ) break;
	
        onlen = sizeof(on);
        ret= getsockopt( s, SOL_SOCKET, SO_SNDBUF, (void *)&on, &onlen );
        if( on < i*1024 ) break;
        Alarmp( SPLOG_INFO, SESSION, "set_large_socket_buffers: set sndbuf %d, ret is %d\n", on, ret );
        
        onlen = sizeof(on);
        ret= getsockopt( s, SOL_SOCKET, SO_RCVBUF, (void *)&on, &onlen );
        if( on < i*1024 ) break;
        Alarmp( SPLOG_INFO, SESSION, "set_large_socket_buffers: set rcvbuf %d, ret is %d\n", on, ret );
    }
    Alarmp( SPLOG_INFO, SESSION, "set_large_socket_buffers: set sndbuf/rcvbuf to %d\n", 1024*(i-5) );
}

/* API break 3.15.0. version is no longer a float. return 0 on error, 1 if set version number */
int	SP_version(int *major_version, int *minor_version, int *patch_version)
{
        if ( (major_version == NULL ) || 
             (minor_version == NULL ) ||
             (patch_version == NULL ) )
                return( 0 );

        *major_version = SP_MAJOR_VERSION;
        *minor_version = SP_MINOR_VERSION;
        *patch_version = SP_PATCH_VERSION;
        return( 1 );
}
/* Addition to API 3.16.0
 * Returns 0 on error, 1 if successful 
 * Registers a single authentication handler.
 */
int     SP_set_auth_method( const char *auth_name, int (*auth_function) (int, void *), void * auth_data)
{
        Once_execute( &Init_once, sp_initialize );

        if (strlen(auth_name) >= MAX_AUTH_NAME)
        {
            Alarm( SESSION, "SP_set_auth_method: Name of auth method too long\n");
            return(0);
        }
        if ( NULL == auth_function )
        {
            Alarm( SESSION, "SP_set_auth_method: auth method is NULL\n");
            return(0);
        }
        Mutex_lock( &Struct_mutex );

        strncpy(Auth_Methods[0].name, auth_name, MAX_AUTH_NAME);
        Auth_Methods[0].authenticate = auth_function;
        Auth_Methods[0].auth_data = auth_data;
        Num_Reg_Auth_Methods = 1;

        Mutex_unlock( &Struct_mutex );
        return(1);
}
/* Addition to API 3.16.0
 * Returns 0 on error, 1 if successful
 * Registers the set of authentication handlers.
 */
int     SP_set_auth_methods( int num_methods, const char *auth_name[], int (*auth_function[]) (int, void *), void * auth_data[])
{
        int i;

        Once_execute(&Init_once, sp_initialize );

        if (num_methods < 0 || num_methods > MAX_AUTH_METHODS)
        {
            Alarm( SESSION, "SP_set_auth_methods: Too many methods trying to be registered\n");
            return(0);
        }

        /* check validity of handlers */
        for (i=0; i< num_methods; i++)
        {
            if (strlen(auth_name[i]) >= MAX_AUTH_NAME)
            {
                Alarm( SESSION, "SP_set_auth_method: Name of auth method too long\n");
                return(0);
            }
            if ( NULL == auth_function[i] )
            {
                Alarm( SESSION, "SP_set_auth_method: auth method is NULL\n");
                return(0);
            }                
        }

        /* insert set of handlers as atomic action */
        Mutex_lock( &Struct_mutex );
        for (i=0; i< num_methods; i++)
        {

            strncpy(Auth_Methods[i].name, auth_name[i], MAX_AUTH_NAME);
            Auth_Methods[i].authenticate = auth_function[i];
            Auth_Methods[i].auth_data = auth_data[i];
        }
        Num_Reg_Auth_Methods = num_methods;
        Mutex_unlock( &Struct_mutex );
        return(1);
}

int	SP_connect( const char *spread_name, const char *private_name,
		    int priority, int group_membership, mailbox *mbox,
		    char *private_group )
{
    return SP_connect_timeout_low( spread_name, private_name, priority, group_membership, mbox, private_group, NULL );
}

int	SP_connect_timeout( const char *spread_name, const char *private_name,
		    int priority, int group_membership, mailbox *mbox,
		    char *private_group, sp_time time_out )
{
    sp_time *t = NULL;

    if ( ( time_out.sec < 0 && time_out.usec > 0 ) ||
         ( time_out.sec > 0 && time_out.usec < 0 ) ||
         ( time_out.usec <= -1000000 || time_out.usec >= 1000000 ) )
        return ILLEGAL_TIME;
    
    if ( time_out.sec != 0 || time_out.usec != 0 )  /* NOTE: maintain existing API: timeout of (0, 0) means infinite timeout */
    {
        time_out = E_add_time( E_get_time_monotonic(), time_out );
        t        = &time_out;
    }
    
    return SP_connect_timeout_low( spread_name, private_name, priority, group_membership, mbox, private_group, t );
}

static int SP_connect_timeout_low( const char *spread_name, const char *private_name,
                                   int priority, int group_membership, mailbox *mbox,
                                   char *private_group, sp_time *abs_time_out )
{
	int16u			port;
	char			host_name[SPREAD_MAXCONNECT_NAMELEN + 1];
	char                    conn[ 5 + MAX_PRIVATE_NAME ];
        char                    auth_list_len;
        char                    auth_list[MAX_AUTH_NAME * MAX_AUTH_METHODS];
        char                    auth_choice[MAX_AUTH_NAME * MAX_AUTH_METHODS];
        bool                    failed;
        int                     num_auth_methods;
        struct auth_method_info auth_methods[MAX_AUTH_METHODS];
	int			s;
	int                     ses;
	int                     base_ses;
	int			ret, i;
        int                     len;
	int			sp_v1, sp_v2, sp_v3;
	int32			on;
        int                     tmp;

        spu_addr                addr      = { 0 };
	struct sockaddr        *sock_addr;
	socklen_t               sock_len;

        struct addrinfo         hint = { 0 };
        struct addrinfo        *rslt = NULL;
        struct addrinfo        *curr;

#ifndef	ARCH_PC_WIN95
	struct	sockaddr_un	unix_addr;
#endif	/* ARCH_PC_WIN95 */

        Once_execute( &Init_once, sp_initialize );

	/* 
	 * There are 4 options for a spread daemon name:
	 *      NULL
	 *      ""
	 * 	<port_num>
	 *	<port_num>@<host_name>
	 */

	/* options NULL and "" */
	if( spread_name == NULL || !strcmp( spread_name, "" ) )
          spread_name = DEFAULT_SPREAD_CONNECTION;

	/* ensure sscanf is safe */
	if( strlen( spread_name ) >= sizeof( host_name ) )
          return( ILLEGAL_SPREAD );

	switch ( sscanf( spread_name, "%hu%s", &port, host_name ) )
	{
	case 1:  /* option <port_num> */

#ifndef	ARCH_PC_WIN95
		memset( &unix_addr, 0, sizeof( unix_addr ) );
		unix_addr.sun_family = AF_UNIX;
		sprintf( unix_addr.sun_path, "%s/%hu", SP_UNIX_SOCKET, port );
		sock_addr = (struct sockaddr *) &unix_addr;
		sock_len  = sizeof( unix_addr );

		s = socket( AF_UNIX, SOCK_STREAM, 0 );
		if( s == INVALID_SOCKET )
		{
			Alarm( DEBUG, "SP_connect: unable to create mailbox %d\n", s );
			return( COULD_NOT_CONNECT );
		}

		break;

#else	/* ARCH_PC_WIN95 */
		/* NOTE: win32: intentional fall through this case to option <port_num>@<host_name> where <host_name> = localhost */
		strcpy( host_name, "@localhost" );
#endif	/* ARCH_PC_WIN95 */

	case 2:  /* option <port_num>@<host_name> */

		if( host_name[0] != '@' ) return( ILLEGAL_SPREAD );

                hint.ai_family   = PF_UNSPEC;
                hint.ai_socktype = SOCK_STREAM;
                hint.ai_protocol = IPPROTO_TCP;
                hint.ai_flags    = 0;

                if ((ret = getaddrinfo(host_name + 1, NULL, &hint, &rslt)) || rslt == NULL)
                {
                  Alarm(DEBUG, "SP_connect: getaddrinfo(%s) failed with %d '%s'\n", host_name + 1, ret, GAI_STRERROR(ret));
                 
                  if (ret == EAI_NONAME)
                    return ILLEGAL_SPREAD;

                  return COULD_NOT_CONNECT;
                }

                for (curr = rslt; curr != NULL; curr = curr->ai_next)
                {
                  if (curr->ai_addr == NULL)
                    continue;
                  
                  switch (curr->ai_addr->sa_family)
                  {
                  case AF_INET:
                    addr.ipv4           = *(struct sockaddr_in*) curr->ai_addr;
                    addr.ipv4.sin_port  = htons(port);
                    sock_len            = sizeof(struct sockaddr_in);
                    break;
                    
                  case AF_INET6:
                    addr.ipv6           = *(struct sockaddr_in6*) curr->ai_addr;
                    addr.ipv6.sin6_port = htons(port);
                    sock_len            = sizeof(struct sockaddr_in6);
                    break;

                  default:
                    continue;
                  }
                    
                  break;
                }
                
                freeaddrinfo(rslt);
                
                if (curr == NULL)
                {
                  Alarm(DEBUG, "SP_connect: getaddrinfo(%s) didn't return any usable internet addresses!\n");
                  return COULD_NOT_CONNECT;
                }

                sock_addr = &addr.addr;
                
		s = socket( sock_addr->sa_family, SOCK_STREAM, 0 );
                
		if( s == INVALID_SOCKET )
		{
                        Alarm( DEBUG, "SP_connect: unable to create mailbox %d %d '%s'\n", s, sock_errno, sock_strerror(sock_errno) );
			return( COULD_NOT_CONNECT );
		}

                on = 1;
                ret = setsockopt( s, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on) );
                if( ret < 0 ) Alarm( PRINT,   "SP_connect: setting TCP_NODELAY failed with error: %s\n", sock_strerror( sock_errno ) );
                else          Alarm( SESSION, "SP_connect: set TCP_NODELAY for socket %d\n", s );

                on = 1;
                ret = setsockopt( s, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on) );
                if( ret < 0 ) Alarm( PRINT,   "SP_connect: setting SO_KEEPALIVE failed with error: %s\n", sock_strerror( sock_errno ) );
                else          Alarm( SESSION, "SP_connect: set SO_KEEPALIVE for socket %d\n", s );

		break;

	default:  /* didn't start with a legal port # */
		return( ILLEGAL_SPREAD );
	}

	set_large_socket_buffers(s);

        if ( ioctl( s, FIONBIO, ( tmp = 1, &tmp ) ) == -1 )
        {
		Alarm( SESSION, "SP_connect: unable to set non-blocking mode on mailbox %d: %s\n", s, sock_strerror( sock_errno ) );
		close( s );
		return( COULD_NOT_CONNECT );
        }

	ret = connect_nointr( s, sock_addr, sock_len, abs_time_out );
        
	if( ret < 0 )
	{
		Alarm( SESSION, "SP_connect: unable to connect mailbox %d: %s\n", s, sock_strerror(sock_errno));
		close( s );
		return( COULD_NOT_CONNECT );
	}
        
        /* client connect message looks like:
	 *
	 * byte - version of lib
	 * byte - subversion of lib
         * byte - patch version of lib
	 * byte - lower half byte 1/0 with or without groups, upper half byte: priority (0/1).
	 * byte - len of name
	 * len bytes - name
	 */

        len = 0;

        if ( private_name != NULL &&
             ( len = strlen( private_name ) ) > MAX_PRIVATE_NAME )
                len = MAX_PRIVATE_NAME;
        
	conn[0] = SP_MAJOR_VERSION;
	conn[1] = SP_MINOR_VERSION;
	conn[2] = SP_PATCH_VERSION;
        conn[3] = ( ( ( priority >= 1 ) << 4 ) | ( group_membership != 0 ) );
	conn[4] = len;
        memcpy( &conn[5], private_name, len );

        ret = send_n_nointr_nb( s, conn, 5+len, abs_time_out );

        if ( ret )
        {
		Alarm( SESSION, "SP_connect: unable to send connect handshake %d %d: %s\n", ret, 5+len, sock_strerror(sock_errno));
		close( s );
		return( CONNECTION_CLOSED );
        }
        
        /* insert access control and authentication checks here */

        ret = recv_n_nointr_nb( s, &auth_list_len, sizeof( auth_list_len ), abs_time_out );
        
        if ( ret )
        {
		Alarm( SESSION, "SP_connect: unable to read auth_list_len %d: %s\n", ret, sock_strerror(sock_errno));
		close( s );
		return( CONNECTION_CLOSED );
        }
        
        if ( auth_list_len < 0 ) 
        {
                Alarm( SESSION, "SP_connect: connection invalid with code %d while reading auth_list_len\n", auth_list_len );
                close( s );
                return( auth_list_len );
        }
        
        if ( auth_list_len > MAX_AUTH_NAME * MAX_AUTH_METHODS )
        {
		Alarm( SESSION, "SP_connect: illegal value in auth_list_len %d: %s\n", auth_list_len, sock_strerror(sock_errno) );
		close( s );
		return( CONNECTION_CLOSED );
        }

        auth_list[0] = 0;
        
        ret = recv_n_nointr_nb( s, auth_list, auth_list_len, abs_time_out);
                
        if ( ret )
        {
                Alarm( SESSION, "SP_connect: unable to read auth_list %d: %s\n", ret, sock_strerror(sock_errno));
                close( s );
                return( CONNECTION_CLOSED );
        }

        Alarm( SESSION, "SP_connect: DEBUG: Auth list is: '%s'\n", auth_list );

        /* Here is where we check the list of available methods of authentication and pick one. 
         * For right now we just ignore the list and use the method the app set in SP_set_auth_method.
         * If no method was set we use the NULL method.
         * The global Auth_Methods struct needs to be protected by the Struct_mutex.
         */
        
        memset(auth_choice, 0, MAX_AUTH_NAME * MAX_AUTH_METHODS);
        
        Mutex_lock( &Struct_mutex );
        for (i = 0; i < Num_Reg_Auth_Methods; i++)
        {
                auth_methods[i] = Auth_Methods[i];
                memcpy(&auth_choice[i * MAX_AUTH_NAME], Auth_Methods[i].name, MAX_AUTH_NAME);
        }
        num_auth_methods = Num_Reg_Auth_Methods;
        Mutex_unlock( &Struct_mutex );

        for ( i = 0; i < num_auth_methods; i++ )
        {
                if ( !valid_auth_method(auth_methods[i].name, auth_list, auth_list_len) )
                {
                        Alarm( SESSION, "SP_connect: chosen authentication method is not permitted by daemon\n");
                        close( s );
                        return( REJECT_AUTH );
                }
        }

        ret = send_n_nointr_nb( s, auth_choice, MAX_AUTH_NAME * MAX_AUTH_METHODS, abs_time_out );
        
	if ( ret )
	{
		Alarm( SESSION, "SP_connect: unable to send auth_name %d %d: %s\n", ret, MAX_AUTH_NAME * MAX_AUTH_METHODS, sock_strerror(sock_errno));
		close( s );
		return( CONNECTION_CLOSED );
	}

        /* Here is where the authentication work will happen. 
         * This will be specific to the method chosen, and should be done in an
         * 'authenticate' function that is called and only returns when the process
         * is finished with either an authenticated connection or a failure. 
         * If failure (return 0) the SP_connect returns with an error REJECT_AUTH.
         * If authenticated (return 1 ), SP_connect continues with the rest of the connect protocol.
         */
        
        failed = FALSE;
        
        for ( i = 0; i < num_auth_methods; i++ )
        {
                ret = auth_methods[i].authenticate( s, auth_methods[i].auth_data );
                
                if ( !ret )
                {
                        Alarm( SESSION, "SP_connect: authentication of connection failed in method %s\n", auth_methods[i].name );
                        failed = TRUE;
                }
        }
        
        if ( failed )
        {
                close ( s );
                return( REJECT_AUTH );
        }

        /* server connect message looks like:
	 *
         * byte - accept / reject
	 * byte - version of daemon
	 * byte - subversion of daemon
         * byte - patch version of daemon
	 * byte - len of name
	 * len bytes - name
	 */

        ret = recv_n_nointr_nb( s, conn, 5, abs_time_out );
        
	if ( ret )
	{
                close( s );

                if ( ret < 5 && conn[0] != ACCEPT_SESSION )
                {
                        Alarm( SESSION, "SP_connect: session rejected %d\n", conn[0] );
                        return( conn[0] );
                }
                
		Alarm( SESSION, "SP_connect: unable to read answer %d: %s\n", ret, sock_strerror(sock_errno) );
		return( CONNECTION_CLOSED );
	}
        
	if ( conn[0] != ACCEPT_SESSION )
	{
		close( s );
		return( conn[0] );
	}

        sp_v1 = conn[1];
        sp_v2 = conn[2];
        sp_v3 = conn[3];
        len   = conn[4];
        
	/* checking daemon version */

        if ( sp_v1 < 0 || sp_v2 < 0 || sp_v3 < 0 || len < 0 || len >= MAX_GROUP_NAME )
        {
                Alarm( PRINT  , "SP_connect: illegal values received! %d.%d.%d; %d\n",
                       sp_v1, sp_v2, sp_v3, len );
		close( s );
		return( CONNECTION_CLOSED );
        }
        
	if ( sp_v1 < 3 )
	{
		Alarm( PRINT  , "SP_connect: old daemon version %d.%d.%d not supported\n", 
			sp_v1, sp_v2, sp_v3 );
		close( s );
		return( REJECT_VERSION );
	}
        
	if ( priority > 0 && sp_v1 == 3 && sp_v2 < 8 )
	{
		Alarm( PRINT, "SP_connect: old spread version %d.%d.%d does not support priority other than 0\n", 
			sp_v1, sp_v2, sp_v3 );
		close( s );
		return( REJECT_VERSION );
	}

        ret = recv_n_nointr_nb( s, private_group, len, abs_time_out);
        
	if ( ret )
	{
		Alarm( SESSION, "SP_connect: unable to read private group %d: %s\n", ret, sock_strerror(sock_errno));
		close( s );
		return( CONNECTION_CLOSED );
	}
        
	private_group[len] = 0;
        
	Alarm( DEBUG, "SP_connect: connected with private group(%d bytes): %s\n", 
		ret, private_group );
        
	*mbox = s;

        if ( ioctl( s, FIONBIO, ( tmp = 0, &tmp ) ) == -1 )
        {
		Alarm( SESSION, "SP_connect: unable to set blocking mode on mailbox %d: %s\n", s, sock_strerror( sock_errno ) );
		close( s );
		return( CONNECTION_CLOSED );
        }

	Mutex_lock( &Struct_mutex );

	if (Num_sessions >= MAX_LIB_SESSIONS)
        {
	        Alarm( SESSION, "SP_connect: too many sessions in local process!\n");
		close( s );
		Mutex_unlock( &Struct_mutex );
		return( REJECT_QUOTA );
	}

	++Num_sessions;

	/* find an unused session structure */

	ses = base_ses = MBOX_TO_BASE_SES(s);

	while( Sessions[ses].state != SESS_UNUSED )
        {
	        if( ++ses == MAX_LIB_SESSIONS)
	                ses = 0;

		if( ses == base_ses )
                {
		        Alarm( SESSION, "SP_connect: BUG! No unused sessions when there should be!\n");
			close( s );
			Mutex_unlock( &Struct_mutex );
			return( SP_BUG );
		}
	}

	Sessions[ses].mbox = s;
        Sessions[ses].state = SESS_ACTIVE;
	strcpy( Sessions[ses].private_group_name, private_group );
        Sessions[ses].recv_message_saved = 0;

	Mutex_unlock( &Struct_mutex );

	return( ACCEPT_SESSION );
}

int	SP_disconnect( mailbox mbox )
{
	int		ses;
	int		ret;
	char		send_group[MAX_GROUP_NAME];
	scatter		send_scat;

	Mutex_lock( &Struct_mutex );

	ses = SP_get_session( mbox );

	if( ses < 0 )
	{
		Mutex_unlock( &Struct_mutex );
		return( ILLEGAL_SESSION );
	}
	strcpy(send_group, Sessions[ses].private_group_name );

	Mutex_unlock( &Struct_mutex );

	send_scat.num_elements = 0;

	ret = SP_internal_multicast( mbox, KILL_MESS, 1, (const char (*)[MAX_GROUP_NAME])send_group, 0, &send_scat );

	SP_kill( mbox );

	ret = 0;

	return( ret );
}

int	SP_join( mailbox mbox, const char *group )
{
	int		ret;
	char		send_group[MAX_GROUP_NAME];
	scatter		send_scat;
	unsigned int	len;
	int		i;

	len = strlen( group );
	if ( len == 0 ) return( ILLEGAL_GROUP );
        if ( len >= MAX_GROUP_NAME ) return( ILLEGAL_GROUP );
	for( i=0; i < (int) len; i++ )
		if( group[i] < 36 || group[i] > 126 ) return( ILLEGAL_GROUP );

	send_group[MAX_GROUP_NAME-1]=0;
	strncpy(send_group, group, MAX_GROUP_NAME-1);
	send_scat.num_elements = 0;

	ret = SP_internal_multicast( mbox, JOIN_MESS, 1, (const char (*)[MAX_GROUP_NAME])send_group, 0, &send_scat );
	return( ret );
}

int	SP_leave( mailbox mbox, const char *group )
{
	int		ret;
	char		send_group[MAX_GROUP_NAME];
	scatter		send_scat;
	unsigned int	len;
	int		i;

	len = strlen( group );
	if ( len == 0 ) return( ILLEGAL_GROUP );
	if ( (int) len >= MAX_GROUP_NAME ) return( ILLEGAL_GROUP );
	for( i=0; i < (int) len; i++ )
		if( group[i] < 36 || group[i] > 126 ) return( ILLEGAL_GROUP );

	send_group[MAX_GROUP_NAME-1]=0;
	strncpy(send_group, group, MAX_GROUP_NAME-1);
	send_scat.num_elements = 0;

	ret = SP_internal_multicast( mbox, LEAVE_MESS, 1, (const char (*)[MAX_GROUP_NAME])send_group, 0, &send_scat );
	return( ret );
}

int	SP_multicast( mailbox mbox, service service_type, 
		      const char *group,
		      int16 mess_type, int mess_len, const char *mess )
{
	int		ret;
	char		send_group[MAX_GROUP_NAME];
	scatter		send_scat;

	send_group[MAX_GROUP_NAME-1]=0;
	strncpy(send_group, group, MAX_GROUP_NAME-1);

	send_scat.num_elements = 1;
	send_scat.elements[0].len = mess_len;
	/* might be good to create a const_scatter type */
	send_scat.elements[0].buf = (char *)mess;

	ret = SP_multigroup_scat_multicast( mbox, service_type, 1, (const char (*)[MAX_GROUP_NAME])send_group, mess_type, &send_scat );
	return( ret );
}

int	SP_scat_multicast( mailbox mbox, service service_type, 
			   const char *group,
			   int16 mess_type, const scatter *scat_mess )
{
	int		ret;
	char		send_group[MAX_GROUP_NAME];

	send_group[MAX_GROUP_NAME-1]=0;
	strncpy(send_group, group, MAX_GROUP_NAME-1);

	ret = SP_multigroup_scat_multicast( mbox, service_type, 1, (const char (*)[MAX_GROUP_NAME])send_group, mess_type, scat_mess );
	return( ret );

}

int	SP_multigroup_multicast( mailbox mbox, service service_type, 
			   	 int num_groups,
				 const char groups[][MAX_GROUP_NAME],
		      		 int16 mess_type, int mess_len,
				 const char *mess )
{
	int		ret;
	scatter		send_scat;

	send_scat.num_elements = 1;
	send_scat.elements[0].len = mess_len;
	send_scat.elements[0].buf = (char *)mess;

	ret = SP_multigroup_scat_multicast( mbox, service_type, num_groups, groups, mess_type, &send_scat );
	return( ret );
}

int	SP_multigroup_scat_multicast( mailbox mbox, service service_type, 
			   	      int num_groups,
				      const char groups[][MAX_GROUP_NAME],
			   	      int16 mess_type,
				      const scatter *scat_mess )
{
	int		ret;

	if( !Is_regular_mess( service_type ) ) return( ILLEGAL_SERVICE );

	ret = SP_internal_multicast( mbox, service_type, num_groups, groups, mess_type, scat_mess );
	return( ret );
}

static	int	SP_internal_multicast( mailbox mbox, service service_type, 
				       int num_groups,
				       const char groups[][MAX_GROUP_NAME],
				       int16 mess_type,
				       const scatter *scat_mess )
{

	char		head_buf[10000]; 
	message_header	*head_ptr;
	char		*group_ptr;
	int		mess_len, len;
	int		ses;
	int		i;
        int             buf_len;
	int		ret;

        /* zero head_buf to avoid information leakage */
        memset( head_buf, 0, sizeof(message_header) + MAX_GROUP_NAME*num_groups );

	Mutex_lock( &Struct_mutex );

	ses = SP_get_session( mbox );
	if( ses < 0 ){
		Mutex_unlock( &Struct_mutex );
		return( ILLEGAL_SESSION );
	}

        if( Sessions[ses].state != SESS_ACTIVE ) {
		Mutex_unlock( &Struct_mutex );
		return( NET_ERROR_ON_SESSION );
	}

	head_ptr = (message_header *)head_buf;
	group_ptr = &head_buf[ sizeof(message_header) ];

	/* enter the private_group_name of this mbox */
	strcpy( head_ptr->private_group_name, Sessions[ses].private_group_name );

	Mutex_unlock( &Struct_mutex );

	for( i=0, mess_len=0; i < (int) scat_mess->num_elements; i++ )
	{
		mess_len += scat_mess->elements[i].len;
	}

        if ( (mess_len + num_groups * MAX_GROUP_NAME) > MAX_MESSAGE_BODY_LEN )
        {
                /* Message contents + groups is too large */
                return( MESSAGE_TOO_LONG );
        }

	head_ptr->type = service_type;
	head_ptr->type = Set_endian( head_ptr->type );

	head_ptr->hint = (mess_type << 8) & 0x00ffff00;
	head_ptr->hint = Set_endian( head_ptr->hint );

	head_ptr->num_groups = num_groups;
	head_ptr->data_len = mess_len;
	memcpy( group_ptr, groups, MAX_GROUP_NAME * num_groups );

	Mutex_lock( &Sessions[ses].send_mutex );
        for ( buf_len = 0; buf_len < (int) sizeof(message_header)+MAX_GROUP_NAME*num_groups; buf_len += ret) 
        {
            while(((ret=send( mbox, &head_buf[buf_len], sizeof(message_header)+MAX_GROUP_NAME*num_groups - buf_len, 0 )) == -1) 
                  && ((sock_errno == EINTR) || (sock_errno == EAGAIN) || (sock_errno == EWOULDBLOCK)) )
                ;
            if( ret < 0 )
            {
		Alarm( SESSION, "SP_internal_multicast: error %d sending header and groups on mailbox %d: %s \n", ret, mbox, sock_strerror(sock_errno));
                Mutex_lock( &Struct_mutex );
                if( ses != SP_get_session( mbox ) ){
                    Alarmp( SPLOG_INFO, SESSION, "SP_internal_multicast: Session disappeared on us, possible in threaded apps that "
			    "don't externally synchronize use and destruction of mailboxes properly (user race condition)!!!\n");
                    Mutex_unlock( &Struct_mutex );
                    Mutex_unlock( &Sessions[ses].send_mutex );
                    return( CONNECTION_CLOSED );
                }
                Sessions[ses].state = SESS_ERROR;
                Mutex_unlock( &Struct_mutex );

		Mutex_unlock( &Sessions[ses].send_mutex );
		return( CONNECTION_CLOSED );
            }
        }
	for( len=0, i=0; i < (int) scat_mess->num_elements; len+=buf_len, i++ )
	{
            for ( buf_len = 0; buf_len < (int) scat_mess->elements[i].len; buf_len += ret) 
            {
		while(((ret=send( mbox, &scat_mess->elements[i].buf[buf_len], scat_mess->elements[i].len - buf_len, 0 )) == -1)
                      && ((sock_errno == EINTR) || (sock_errno == EAGAIN) || (sock_errno == EWOULDBLOCK)) )
                        ;
		if( ret < 0 )
		{
			Alarm( SESSION, "SP_internal_multicast: error %d sending message data on mailbox %d: %s \n", ret, mbox, sock_strerror(sock_errno));
                        Mutex_lock( &Struct_mutex );
                        if( ses != SP_get_session( mbox ) ){
			    Alarmp( SPLOG_INFO, SESSION, "SP_internal_multicast: Session disappeared on us, possible in threaded apps that "
				    "don't externally synchronize use and destruction of mailboxes properly (user race condition)\n");
                            Mutex_unlock( &Struct_mutex );
                            Mutex_unlock( &Sessions[ses].send_mutex );
                            return( CONNECTION_CLOSED );
                        }
                        Sessions[ses].state = SESS_ERROR;
                        Mutex_unlock( &Struct_mutex );

			Mutex_unlock( &Sessions[ses].send_mutex );
			return( CONNECTION_CLOSED );
		}
            }
	}
	Mutex_unlock( &Sessions[ses].send_mutex );
	return( len );
}

int	SP_receive( mailbox mbox, service *service_type, char sender[MAX_GROUP_NAME],
		    int max_groups, int *num_groups, char groups[][MAX_GROUP_NAME],
		    int16 *mess_type, int *endian_mismatch,
		    int max_mess_len, char *mess )
{
	int		ret;
	scatter		recv_scat;

	recv_scat.num_elements = 1;
	recv_scat.elements[0].len = max_mess_len;
	recv_scat.elements[0].buf = mess;

	ret = SP_scat_receive( mbox, service_type, sender, max_groups, num_groups, groups, 
				mess_type, endian_mismatch, &recv_scat );
	return( ret );
}

int	SP_scat_receive( mailbox mbox, service *service_type, char sender[MAX_GROUP_NAME],
			 int max_groups, int *num_groups, char groups[][MAX_GROUP_NAME],
			 int16 *mess_type, int *endian_mismatch,
			 scatter *scat_mess )
{

static	char		dummy_buf[10240];
        int             This_session_message_saved;
        int             drop_semantics;
	message_header	mess_head;
	message_header	*head_ptr;
	char		*buf_ptr;
        int32           temp_mess_type;
	int		len, remain, ret;
	int		max_mess_len;
	int		short_buffer;
	int		short_groups;
	int		to_read;
	int		scat_index, byte_index;
	int		ses;
	char		This_session_private_group[MAX_GROUP_NAME];
	int		i;
        int32           old_type;

	/* lookup and validate the session */

	Mutex_lock( &Struct_mutex );

	ses = SP_get_session( mbox );

	if( ses < 0 ) {
	  Mutex_unlock( &Struct_mutex );
	  return( ILLEGAL_SESSION );
	}

	if( Sessions[ses].state != SESS_ACTIVE ) {
	  Mutex_unlock( &Struct_mutex );
	  return( NET_ERROR_ON_SESSION );
	}

	Mutex_unlock( &Struct_mutex );

        /* I must acquire the recv lock for this mbox before the
         * Struct_mutex lock because I must be sure ONLY one thread is
         * truly in recv for this mbox. Otherwise, one thread enters
         * this and gets the state and sees no saved message then
         * grabs the recv lock and discovers buffer too short and
         * so regrabs the Struct_mutex and adds the saved header, but
         * during this time another thread has entered recv for the
         * same mbox and already grabbed the Struct_mutex and also
         * seen that no saved mesage exists and is now waiting for the
         * recv mutex.  When the first thread returns and releases the
         * recv mutex, the second thread will grab it and enter--but
         * it will think there is NO saved messaage when in reality
         * there IS one. This will cause MANY PROBLEMS :-)
         *
         * NOTE: locking and unlocking the Struct_mutex multiple times
         * during this is OK BECAUSE Struct_mutex only locks
         * non-blocking operations that are guaranteed to complete
         * quickly and never take additional locks.
         */

	Mutex_lock( &Sessions[ses].recv_mutex );

	Mutex_lock( &Struct_mutex );

	if( ses != SP_get_session( mbox ) ){
		Mutex_unlock( &Struct_mutex );
                Mutex_unlock( &Sessions[ses].recv_mutex );
		return( ILLEGAL_SESSION );
	}

        if( Sessions[ses].state != SESS_ACTIVE ) {
		Mutex_unlock( &Struct_mutex );
                Mutex_unlock( &Sessions[ses].recv_mutex );
		return( NET_ERROR_ON_SESSION );
	}

	strcpy( This_session_private_group, Sessions[ses].private_group_name );

        if (Sessions[ses].recv_message_saved) {
                memcpy(&mess_head, &(Sessions[ses].recv_saved_head), sizeof(message_header) );
                This_session_message_saved = 1;
        } else {
                This_session_message_saved = 0;
        }

	Mutex_unlock( &Struct_mutex );
        
	head_ptr = (message_header *)&mess_head;
	buf_ptr = (char *)&mess_head;

        drop_semantics = Is_drop_recv(*service_type);

        if (!This_session_message_saved) {
                /* read up to size of message_header */
                for( len=0, remain = sizeof(message_header); remain > 0;  len += ret, remain -= ret )
                {
                        while(((ret = recv( mbox, &buf_ptr[len], remain, 0 )) == -1 )
                              && ((sock_errno == EINTR) || (sock_errno == EAGAIN) || (sock_errno == EWOULDBLOCK)) )
                                ;
                        if( ret <=0 )
                        {
                                Alarm( SESSION, "SP_scat_receive: failed receiving header on session %d (ret: %d len: %d): %s\n", mbox, ret, len, sock_strerror(sock_errno) );
                                Mutex_lock( &Struct_mutex );
                                if( ses != SP_get_session( mbox ) ){
				    Alarmp( SPLOG_INFO, SESSION, "SP_scat_receive: Session disappeared on us, possible in threaded apps that "
					    "don't externally synchronize use and destruction of mailboxes properly (user race condition)!!!\n");
                                    Mutex_unlock( &Struct_mutex );
				    Mutex_unlock( &Sessions[ses].recv_mutex );
                                    return( CONNECTION_CLOSED );
                                }
                                Sessions[ses].state = SESS_ERROR;
                                Mutex_unlock( &Struct_mutex );

				Mutex_unlock( &Sessions[ses].recv_mutex );
                                return( CONNECTION_CLOSED );
                        }
                }

                /* Fliping message header to my form if needed */
                if( !Same_endian( head_ptr->type ) ) 
                {
                        Flip_mess( head_ptr );
                }
        }

	for( max_mess_len = 0, i=0; i < (int) scat_mess->num_elements; i++ ) {
		max_mess_len += scat_mess->elements[i].len;
        }

        /* Validate num_groups and data_len */
        if (head_ptr->num_groups < 0) {
            /* reject this message since it has an impossible (negative) num_groups
             * This is likely to be caused by a malicious attack or memory corruption
             */
	    Mutex_unlock( &Sessions[ses].recv_mutex );
            return( ILLEGAL_MESSAGE );
        }
        if (head_ptr->data_len < 0) {
            /* reject this message since it has an impossible (negative) data_len
             * This is likely to be caused by a malicious attack or memory corruption
             */
	    Mutex_unlock( &Sessions[ses].recv_mutex );
            return( ILLEGAL_MESSAGE );
        }

        /* Check if sufficient buffer space for groups and data */
        if (!drop_semantics) {
                if ( (head_ptr->num_groups > max_groups) || (head_ptr->data_len > max_mess_len) ) {
                        if (!This_session_message_saved) {
                                Mutex_lock( &Struct_mutex );
                                if( ses != SP_get_session( mbox ) ){
                                        Mutex_unlock( &Struct_mutex );
					Mutex_unlock( &Sessions[ses].recv_mutex );
                                        return( ILLEGAL_SESSION );
                                }
                                memcpy(&(Sessions[ses].recv_saved_head), &mess_head, sizeof(message_header) );
                                Sessions[ses].recv_message_saved = 1;
                                Mutex_unlock( &Struct_mutex );
                        }
                        /* When *_TOO_SHORT error will be returned, provide caller with all available information:
                         * service_type
                         * sender
                         * mess_type
                         * 
                         * The num_groups field and endian_mismatch field are used to specify the required
                         * size of the groups array and message body array in order to fit the current message
                         * so, they do NOT have their usual meaning. 
                         * If number of groups in the message is > max_groups then the number of required groups 
                         *   is returned as a negative value in the num_groups field.
                         * If the size of the message is > max_mess_len, then the required size in bytes is 
                         *   returned as a negative value in the endian_mismatch field.
                         */
                        if ( Is_regular_mess( head_ptr->type ) || Is_reject_mess( head_ptr->type ) )
                        {
                                temp_mess_type = head_ptr->hint;
                                if ( !Same_endian( head_ptr->hint ) ) {
                                        temp_mess_type = Flip_int32( temp_mess_type );
                                }
                                temp_mess_type = Clear_endian( temp_mess_type );
                                *mess_type = ( temp_mess_type >> 8 ) & 0x0000ffff;
                        }
                        else 
                                *mess_type = 0;
                        *service_type = Clear_endian( head_ptr->type );
                        if (head_ptr->num_groups > max_groups)
                                *num_groups = -(head_ptr->num_groups);
                        else    
                                *num_groups = 0;
                        if (head_ptr->data_len > max_mess_len)
                                *endian_mismatch = -(head_ptr->data_len);
                        else
                                *endian_mismatch = 0;

                        /* Return sender field to caller */
                        strncpy( sender, head_ptr->private_group_name, MAX_GROUP_NAME );

			Mutex_unlock( &Sessions[ses].recv_mutex );
                        if (*num_groups)
                                return( GROUPS_TOO_SHORT );
                        else
                                return( BUFFER_TOO_SHORT );
                }
        }
	/* Compute mess_type and endian_mismatch from hint */
	if( Is_regular_mess( head_ptr->type ) || Is_reject_mess( head_ptr->type)  )
	{
		if( !Same_endian( head_ptr->hint ) )
		{
			head_ptr->hint = Flip_int32( head_ptr->hint );
			*endian_mismatch = 1;
		}else{
			*endian_mismatch = 0;
		}
                head_ptr->hint = Clear_endian( head_ptr->hint );
                head_ptr->hint = ( head_ptr->hint >> 8 ) & 0x0000ffff;
                *mess_type = head_ptr->hint;
	}else{
		*mess_type = -1; /* marks the index (0..n-1) of the member in the group */
		*endian_mismatch = 0;
	}

	strncpy( sender, head_ptr->private_group_name, MAX_GROUP_NAME );
        
        /* if a reject message read the extra old_type field first, and merge with head_ptr->type */
        if ( Is_reject_mess( head_ptr->type ) )
        {
                remain = 4;
                buf_ptr = (char *)&old_type;
                for( len=0; remain > 0; len += ret, remain -= ret )
                {
                        while(((ret = recv( mbox, &buf_ptr[len], remain, 0 )) == -1 ) && ((sock_errno == EINTR) || (sock_errno == EAGAIN) || (sock_errno == EWOULDBLOCK)) )
                                ;
                        if( ret <=0 )
                        {
                                Alarm( SESSION, "SP_scat_receive: failed receiving old_type for reject on session %d, ret is %d: %s\n", mbox, ret, sock_strerror(sock_errno));

                                Mutex_lock( &Struct_mutex );
                                if( ses != SP_get_session( mbox ) ){
				    Alarmp( SPLOG_INFO, SESSION, "SP_scat_receive: Session disappeared on us, possible in threaded apps that "
					    "don't externally synchronize use and destruction of mailboxes properly (user race condition)!!!\n");
                                    Mutex_unlock( &Struct_mutex );
				    Mutex_unlock( &Sessions[ses].recv_mutex );
                                    return( CONNECTION_CLOSED );
                                }
                                Sessions[ses].state = SESS_ERROR;
                                Mutex_unlock( &Struct_mutex );

				Mutex_unlock( &Sessions[ses].recv_mutex );
                                return( CONNECTION_CLOSED );
                        }
                }
                /* endian flip it */
                if ( !Same_endian( head_ptr->type ) )
                        old_type = Flip_int32(old_type);
        }

	/* read the destination groups */
	buf_ptr = (char *)groups;

	remain = head_ptr->num_groups * MAX_GROUP_NAME;
	short_groups=0;
	if( head_ptr->num_groups > max_groups )
	{
		/* groups too short */
		remain = max_groups * MAX_GROUP_NAME;
		short_groups = 1;
	}

	for( len=0; remain > 0; len += ret, remain -= ret )
	{
		while(((ret = recv( mbox, &buf_ptr[len], remain, 0 )) == -1 ) && ((sock_errno == EINTR) || (sock_errno == EAGAIN) || (sock_errno == EWOULDBLOCK)) )
                        ;
		if( ret <=0 )
		{
			Alarm( SESSION, "SP_scat_receive: failed receiving groups on session %d, ret is %d: %s\n", mbox, ret, sock_strerror(sock_errno));

                        Mutex_lock( &Struct_mutex );
                        if( ses != SP_get_session( mbox ) ){
				    Alarmp( SPLOG_INFO, SESSION, "SP_scat_receive: Session disappeared on us, possible in threaded apps that "
					    "don't externally synchronize use and destruction of mailboxes properly (user race condition)!!!\n");
                            Mutex_unlock( &Struct_mutex );
			    Mutex_unlock( &Sessions[ses].recv_mutex );
                            return( CONNECTION_CLOSED );
                        }
                        Sessions[ses].state = SESS_ERROR;
                        Mutex_unlock( &Struct_mutex );

			Mutex_unlock( &Sessions[ses].recv_mutex );
			return( CONNECTION_CLOSED );
		}
	}

	if( short_groups )
	{
		for( remain = (head_ptr->num_groups - max_groups) * MAX_GROUP_NAME; 
		     remain > 0; remain -= ret ) 
		{
			to_read = remain;
			if( to_read > sizeof( dummy_buf ) ) to_read = sizeof( dummy_buf );
			while(((ret = recv( mbox, dummy_buf, to_read, 0 )) == -1 ) && ((sock_errno == EINTR) || (sock_errno == EAGAIN) || (sock_errno == EWOULDBLOCK)) )
                                ;
			if( ret <=0 )
			{
				Alarm( SESSION, "SP_scat_receive: failed receiving groups overflow on session %d, ret is %d: %s\n", 
                                       mbox, ret, sock_strerror(sock_errno) );

                                Mutex_lock( &Struct_mutex );
                                if( ses != SP_get_session( mbox ) ){
				    Alarmp( SPLOG_INFO, SESSION, "SP_scat_receive: Session disappeared on us, possible in threaded apps that "
					    "don't externally synchronize use and destruction of mailboxes properly (user race condition)!!!\n");
                                    Mutex_unlock( &Struct_mutex );
				    Mutex_unlock( &Sessions[ses].recv_mutex );
                                    return( CONNECTION_CLOSED );
                                }
                                Sessions[ses].state = SESS_ERROR;
                                Mutex_unlock( &Struct_mutex );

				Mutex_unlock( &Sessions[ses].recv_mutex );
				return( CONNECTION_CLOSED );
			}
		}
		*num_groups = -head_ptr->num_groups; /* !!!! */
	}else	*num_groups = head_ptr->num_groups;

	/* read the rest of the message */
	remain = head_ptr->data_len;
	short_buffer=0;
	if( head_ptr->data_len > max_mess_len )
	{
		/* buffer too short */
		remain = max_mess_len;
		short_buffer = 1;
	}

	ret = 0;
	/* 
	 * pay attention that if head_ptr->data_len is smaller than max_mess_len we need to
	 * change scat, do recvmsg, and restore scat, and then check ret.
         * ret = recvmsg( mbox, &msg, 0 ); 
         * if( ret <=0 )
         * {
         *      Alarm( SESSION, "SP_scat_receive: failed receiving message on session %d\n", mbox );
	 *
	 *      Mutex_unlock( &Sessions[ses].recv_mutex );
	 *
         *      SP_kill( mbox );
         *      return;
         * }
	 */

	/* calculate scat_index and byte_index based on ret and scat_mess */
	for( byte_index=ret, scat_index=0; scat_index < (int) scat_mess->num_elements; scat_index++ )
	{
		if( (int) scat_mess->elements[scat_index].len > byte_index ) break;
		byte_index -= scat_mess->elements[scat_index].len;
	}

	remain -= ret;
	for( len=ret; remain > 0; len += ret, remain -= ret )
	{
		to_read = scat_mess->elements[scat_index].len - byte_index;
		if( to_read > remain ) to_read = remain;
		while(((ret = recv( mbox, &scat_mess->elements[scat_index].buf[byte_index], to_read, 0 )) == -1 )
                      && ((sock_errno == EINTR) || (sock_errno == EAGAIN) || (sock_errno == EWOULDBLOCK)) )
                        ;
		if( ret <=0 )
		{
			Alarm( SESSION, "SP_scat_receive: failed receiving message on session %d, ret is %d: %s\n", 
                               mbox, ret, sock_strerror(sock_errno) );

                        Mutex_lock( &Struct_mutex );
                        if( ses != SP_get_session( mbox ) ){
				    Alarmp( SPLOG_INFO, SESSION, "SP_scat_receive: Session disappeared on us, possible in threaded apps that "
					    "don't externally synchronize use and destruction of mailboxes properly (user race condition)!!!\n");
                            Mutex_unlock( &Struct_mutex );
			    Mutex_unlock( &Sessions[ses].recv_mutex );
                            return( CONNECTION_CLOSED );
                        }
                        Sessions[ses].state = SESS_ERROR;
                        Mutex_unlock( &Struct_mutex );

			Mutex_unlock( &Sessions[ses].recv_mutex );
			return( CONNECTION_CLOSED );
		}else if( ret == to_read ){
			byte_index = 0;
			scat_index++;
		}else{
			byte_index += ret;
		}
	}

	if( Is_reg_memb_mess( head_ptr->type ) && !short_groups )
	{
		/* calculate my index in group */
		for( i=0; i < head_ptr->num_groups; i++ )
		{
			if( !strcmp( groups[i], This_session_private_group ) )
			{
				*mess_type = i;
				break;
			}
		}
	}

 	if( Is_reg_memb_mess( head_ptr->type ) && !Same_endian( head_ptr->type ) )
	{
		int	 	flip_size;
		group_id	*gid_ptr;
                int32u          *num_vs_sets_ptr;
                int32u           num_vs_sets;
                int32u          *local_vs_set_offset_ptr;
		int32u		*num_vs_ptr;
		int		bytes_to_copy, bytes_index;
		char		groups_buf[10240];
                int             num_bytes;
                int             j;
                int             total_index; /* Index into the scatters viewed as a contiguous byte array */
                int             target_index;/* Goal for total index */
                int             scat;        /* The current scatter element */
                int             scat_index=0;/* Index into the current scatter element */
                int             first_scat;       /* The first scatter element used for a given num_vs */
                int             first_scat_index; /* Index into the first scatter element for a given num_vs */

		/* 
		 * flip membership message:
		 * group_id, number of vs_sets, offset to local,
                 * and number of members in vs_sets (for each set)
		 * so - acctually 5+n int32.
		 */
		flip_size = sizeof( group_id ) + 2*sizeof( int32u );
		if( flip_size > max_mess_len ) flip_size = max_mess_len;
		for( bytes_index = 0, scat = 0 ; bytes_index < flip_size ; bytes_index += bytes_to_copy )
		{
			bytes_to_copy = flip_size - bytes_index;
			if( bytes_to_copy > (int) scat_mess->elements[scat].len )
				bytes_to_copy = scat_mess->elements[scat].len;
			memcpy( &groups_buf[bytes_index], scat_mess->elements[scat].buf, bytes_to_copy );
                        if( bytes_to_copy == scat_mess->elements[scat].len )
                        {
                                scat_index = 0;
                                ++scat;
                        } else {
                                scat_index = bytes_to_copy;
                        }
		}
                total_index  = flip_size;
                target_index = total_index;

                num_bytes               = 0;
		gid_ptr                 = (group_id *)&groups_buf[num_bytes];
                num_bytes              += sizeof(group_id);
                num_vs_sets_ptr         = (int32u *)&groups_buf[num_bytes];
                num_bytes              += sizeof(int32u);
                local_vs_set_offset_ptr = (int32u *)&groups_buf[num_bytes];
                num_bytes              += sizeof(int32u);
                
		gid_ptr->memb_id.proc_id = Flip_int32( gid_ptr->memb_id.proc_id );
		gid_ptr->memb_id.time    = Flip_int32( gid_ptr->memb_id.time );
		gid_ptr->index           = Flip_int32( gid_ptr->index );
                *num_vs_sets_ptr         = Flip_int32( *num_vs_sets_ptr );
                num_vs_sets              = *num_vs_sets_ptr;
                *local_vs_set_offset_ptr = Flip_int32( *local_vs_set_offset_ptr );

		for( bytes_index = 0, j = 0 ; bytes_index < flip_size ; j++, bytes_index += bytes_to_copy )
		{
			bytes_to_copy = flip_size - bytes_index;
			if( bytes_to_copy > (int) scat_mess->elements[j].len )
				bytes_to_copy = scat_mess->elements[j].len;
			memcpy( scat_mess->elements[j].buf, &groups_buf[bytes_index], bytes_to_copy );
		}
                for( i = 0; i < (int) num_vs_sets; ++i )
                {
                        while( total_index < target_index )
                        {
                                if( target_index - total_index < (int) scat_mess->elements[scat].len - scat_index )
                                {
                                        scat_index  += target_index - total_index;
                                        total_index  = target_index;
                                } else {
                                        total_index += scat_mess->elements[scat].len - scat_index;
                                        scat_index   = 0;
                                        ++scat;
                                }
                        }
                        first_scat_index = scat_index;
                        first_scat       = scat;

                        flip_size = sizeof( int32u );
                        if( flip_size + total_index > max_mess_len ) flip_size = max_mess_len - total_index;
                        for( bytes_index = 0 ; bytes_index < flip_size ; bytes_index += bytes_to_copy )
                        {
                                bytes_to_copy = flip_size - bytes_index;
                                if( bytes_to_copy > (int) scat_mess->elements[scat].len - scat_index )
                                        bytes_to_copy = scat_mess->elements[scat].len - scat_index;
                                memcpy( &groups_buf[bytes_index], &(scat_mess->elements[scat].buf[scat_index]),
                                        bytes_to_copy );
                                if( bytes_to_copy == scat_mess->elements[scat].len - scat_index )
                                {
                                        scat_index = 0;
                                        ++scat;
                                } else {
                                        scat_index += bytes_to_copy;
                                }
                        }
                        total_index += flip_size;
                        target_index = total_index;

                        num_vs_ptr  = (int32u *)&groups_buf[0];
                        *num_vs_ptr = Flip_int32( *num_vs_ptr );

                        for( bytes_index = 0, j = first_scat ; bytes_index < flip_size ;
                             j++, bytes_index += bytes_to_copy )
                        {
                                bytes_to_copy = flip_size - bytes_index;
                                if( bytes_to_copy > (int) scat_mess->elements[j].len - first_scat_index )
                                        bytes_to_copy = scat_mess->elements[j].len - first_scat_index;
                                memcpy( &(scat_mess->elements[j].buf[first_scat_index]),
                                        &groups_buf[bytes_index], bytes_to_copy );
                                first_scat_index = 0;
                        }
                        target_index += *num_vs_ptr * MAX_GROUP_NAME;
                }
	}
        if ( Is_reject_mess( head_ptr->type ) )
        {
                /* set type to be old type + reject */
                head_ptr->type = old_type | REJECT_MESS;
        }
	*service_type = Clear_endian( head_ptr->type );

	if( short_buffer )
	{
		for( remain = head_ptr->data_len - max_mess_len; remain > 0; remain -= ret ) 
		{
			to_read = remain;
			if( to_read > sizeof( dummy_buf ) ) to_read = sizeof( dummy_buf );
			while(((ret = recv( mbox, dummy_buf, to_read, 0 )) == -1 ) && ((sock_errno == EINTR) || (sock_errno == EAGAIN) || (sock_errno == EWOULDBLOCK)) )
                                ;
			if( ret <=0 )
			{
				Alarm( SESSION, "SP_scat_receive: failed receiving overflow on session %d, ret is %d: %s\n", 
                                       mbox, ret, sock_strerror(sock_errno) );

                                Mutex_lock( &Struct_mutex );
                                if( ses != SP_get_session( mbox ) ){
				    Alarmp( SPLOG_INFO, SESSION, "SP_scat_receive: Session disappeared on us, possible in threaded apps that "
					    "don't externally synchronize use and destruction of mailboxes properly (user race condition)!!!\n");
                                    Mutex_unlock( &Struct_mutex );
				    Mutex_unlock( &Sessions[ses].recv_mutex );
                                    return( CONNECTION_CLOSED );
                                }
                                Sessions[ses].state = SESS_ERROR;
                                Mutex_unlock( &Struct_mutex );

				Mutex_unlock( &Sessions[ses].recv_mutex );
				return( CONNECTION_CLOSED );
			}
		}
		Mutex_unlock( &Sessions[ses].recv_mutex );
		return( BUFFER_TOO_SHORT );
	}
        /* Successful receive so clear saved_message info if any */
        if (This_session_message_saved) {
                Mutex_lock( &Struct_mutex );
                if( ses != SP_get_session( mbox ) ){
                        Mutex_unlock( &Struct_mutex );
			Mutex_unlock( &Sessions[ses].recv_mutex );
                        return( ILLEGAL_SESSION );
                }
                memset(&(Sessions[ses].recv_saved_head), 0, sizeof(message_header) );
                Sessions[ses].recv_message_saved = 0;
                Mutex_unlock( &Struct_mutex );
        }

	Mutex_unlock( &Sessions[ses].recv_mutex );
	return( head_ptr->data_len );
}

int	SP_poll( mailbox mbox )
{
	int		num_bytes;
	int		ses;
	int		ret;

	Mutex_lock( &Struct_mutex );

	/* verify mbox */
	ses = SP_get_session( mbox );

	Mutex_unlock( &Struct_mutex );

	if( ses < 0 ) return( ILLEGAL_SESSION );

	ret = ioctl( mbox, FIONREAD, &num_bytes);
	if( ret < 0 ) return( ILLEGAL_SESSION );
	return( num_bytes );

}

int     SP_equal_group_ids( group_id g1, group_id g2 )
{
	if( g1.memb_id.proc_id == g2.memb_id.proc_id &&
            g1.memb_id.time    == g2.memb_id.time    &&
            g1.index           == g2.index              )
                return( 1 );
	else return( 0 );
}

/* get membership info from a message */

/* internal only for now -- maybe static? */
static  int     SP_get_gid_offset_memb_mess(void) 
{
        return 0;
}

static  int     SP_get_num_vs_sets_offset_memb_mess(void) 
{
        return sizeof(group_id);
}

static  int     SP_get_offset_to_local_vs_set_offset(void)
{
        return sizeof(group_id) + sizeof(int32u);
}

static  int     SP_get_first_vs_set_offset_memb_mess(void) 
{
        return sizeof(group_id) + 2*sizeof(int32u);
}

static  int     SP_get_vs_set_size_offset_vs_set(void)
{
        return 0;
}

static  int     SP_get_vs_set_members_offset_vs_set(void)
{
        return sizeof(int32u);
}

/* public functions */
int     SP_get_memb_info( const char *memb_mess, 
                          const service service_type,
                          membership_info *memb_info) 
{
        int32u my_vs_offset;

        if ( !Is_membership_mess( service_type ) )
                return(ILLEGAL_MESSAGE);

        memcpy( &memb_info->gid, &memb_mess[SP_get_gid_offset_memb_mess()], sizeof(group_id));
        
        if ( Is_reg_memb_mess( service_type ) )
        {
                if( Is_caused_join_mess( service_type ) || 
                    Is_caused_leave_mess( service_type ) ||
                    Is_caused_disconnect_mess( service_type ) )
                {
                        memcpy(memb_info->changed_member, &memb_mess[SP_get_first_vs_set_offset_memb_mess()+SP_get_vs_set_members_offset_vs_set()], MAX_GROUP_NAME);
                } else if( Is_caused_network_mess( service_type ) )
                {
                        memset(memb_info->changed_member, 0, MAX_GROUP_NAME);
                }
                memb_info->num_vs_sets = *((int32u *)&memb_mess[SP_get_num_vs_sets_offset_memb_mess()]);
                my_vs_offset = *((int32u *)&memb_mess[SP_get_offset_to_local_vs_set_offset()]);
                my_vs_offset += SP_get_first_vs_set_offset_memb_mess();
                memb_info->my_vs_set.num_members = *((int32u *)&memb_mess[my_vs_offset + SP_get_vs_set_size_offset_vs_set()]);
                memb_info->my_vs_set.members_offset  = my_vs_offset + SP_get_vs_set_members_offset_vs_set();
        } else {
                /* Is transitional */
                memb_info->num_vs_sets = 0;
                memb_info->my_vs_set.num_members = 0;
                memb_info->my_vs_set.members_offset  = 0;
        }
        return( 1 );
}


int     SP_get_vs_sets_info( const char *memb_mess, 
                             vs_set_info *vs_sets, 
                             int num_vs_sets,
                             unsigned int *my_vs_set_index)
{
        unsigned int actual_num_vs_sets;
        unsigned int memb_offset, local_vs_set_offset;
        int i;

        actual_num_vs_sets = *((int32u *)&memb_mess[SP_get_num_vs_sets_offset_memb_mess()]);

        if (num_vs_sets < (int) actual_num_vs_sets)
                return(BUFFER_TOO_SHORT);

        local_vs_set_offset = *((int32u *)&memb_mess[SP_get_offset_to_local_vs_set_offset()]);
        local_vs_set_offset += SP_get_first_vs_set_offset_memb_mess();
        memb_offset = SP_get_first_vs_set_offset_memb_mess();

        for ( i=0; i < (int) actual_num_vs_sets; i++, vs_sets++ )
        {
                if (memb_offset == local_vs_set_offset)
                        *my_vs_set_index = i;
                vs_sets->num_members = *(int32u *)&memb_mess[memb_offset];
                memb_offset += SP_get_vs_set_members_offset_vs_set();
                vs_sets->members_offset = memb_offset;
                memb_offset += (vs_sets->num_members * MAX_GROUP_NAME) ;
        }
        return ( actual_num_vs_sets );
}

int     SP_get_vs_set_members( const char *memb_mess, 
                               const vs_set_info *vs_set, 
                               char member_names[][MAX_GROUP_NAME],
                               int member_names_count)
{
        int i;
        const char *members_ptr;

        if (member_names_count < (int) vs_set->num_members)
                return(BUFFER_TOO_SHORT);

        members_ptr = &memb_mess[vs_set->members_offset];
        for ( i=0; i < (int) vs_set->num_members; i++, members_ptr += MAX_GROUP_NAME )
        {
                memcpy(&member_names[i][0], members_ptr, MAX_GROUP_NAME);
        }
        return( vs_set->num_members );
}

static  void    scat_read(void *dest_ptr, const scatter *msg, int start_byte, int copy_size)
{
        int scat, scat_index, bytes_index, bytes_to_copy;

        scat = 0;
        scat_index = 0;
        while(start_byte > (int) msg->elements[scat].len) 
        {
                scat++;
                start_byte -= msg->elements[scat].len;
        }
        scat_index = start_byte;
        for( bytes_index = 0; bytes_index < copy_size ; bytes_index += bytes_to_copy )
        {
                bytes_to_copy = copy_size - bytes_index;
                if( bytes_to_copy > (int) msg->elements[scat].len - scat_index )
                        bytes_to_copy = msg->elements[scat].len - scat_index;
                memcpy( dest_ptr, &(msg->elements[scat].buf[scat_index]), bytes_to_copy );
                if( bytes_to_copy == msg->elements[scat].len - scat_index )
                {
                        scat_index = 0;
                        ++scat;
                } else {
                        scat_index = bytes_to_copy;
                }
        }
}

int     SP_scat_get_memb_info( const scatter *memb_mess_scat, 
                               const service service_type,
                               membership_info *memb_info)
{
        int32u my_vs_offset;

        if ( !Is_membership_mess( service_type ) )
                return(ILLEGAL_MESSAGE);

        scat_read( &memb_info->gid, memb_mess_scat, 0, sizeof( group_id ));
        
        if ( Is_reg_memb_mess( service_type ) )
        {
                if( Is_caused_join_mess( service_type ) || 
                    Is_caused_leave_mess( service_type ) ||
                    Is_caused_disconnect_mess( service_type ) )
                {
                        scat_read(memb_info->changed_member, memb_mess_scat, SP_get_first_vs_set_offset_memb_mess()+SP_get_vs_set_members_offset_vs_set(), MAX_GROUP_NAME );
                } else if( Is_caused_network_mess( service_type ) )
                {
                        memset(memb_info->changed_member, 0, MAX_GROUP_NAME);
                }
                scat_read( &(memb_info->num_vs_sets), memb_mess_scat, SP_get_num_vs_sets_offset_memb_mess(), sizeof( int32u ) );
                scat_read( &my_vs_offset, memb_mess_scat, SP_get_offset_to_local_vs_set_offset(), sizeof( int32u ));
                my_vs_offset += SP_get_first_vs_set_offset_memb_mess();
                scat_read( &(memb_info->my_vs_set.num_members), memb_mess_scat,  my_vs_offset + SP_get_vs_set_size_offset_vs_set(), sizeof( int32u ) );
                memb_info->my_vs_set.members_offset  = my_vs_offset + SP_get_vs_set_members_offset_vs_set();
        } else {
                /* Is transitional */
                memb_info->num_vs_sets = 0;
                memb_info->my_vs_set.num_members = 0;
                memb_info->my_vs_set.members_offset  = 0;
        }
        return( 1 );
}
int     SP_scat_get_vs_sets_info( const scatter *memb_mess_scat, 
                                  vs_set_info *vs_sets, 
                                  int num_vs_sets,
                                  unsigned int *my_vs_set_index)
{
        unsigned int actual_num_vs_sets;
        unsigned int memb_offset, local_vs_set_offset;
        int i;

        scat_read( &actual_num_vs_sets, memb_mess_scat, SP_get_num_vs_sets_offset_memb_mess(), sizeof( int32u ) );

        if (num_vs_sets < (int) actual_num_vs_sets)
                return(BUFFER_TOO_SHORT);

        scat_read( &local_vs_set_offset, memb_mess_scat, SP_get_offset_to_local_vs_set_offset(), sizeof( int32u ) );
        local_vs_set_offset += SP_get_first_vs_set_offset_memb_mess();
        memb_offset = SP_get_first_vs_set_offset_memb_mess();

        for ( i=0; i < num_vs_sets; i++, vs_sets++ )
        {
                if (memb_offset == local_vs_set_offset)
                        *my_vs_set_index = i;

                scat_read(&(vs_sets->num_members), memb_mess_scat, memb_offset, sizeof( int32u ) );
                memb_offset += SP_get_vs_set_members_offset_vs_set();
                vs_sets->members_offset = memb_offset;
                memb_offset += (vs_sets->num_members * MAX_GROUP_NAME) ;
        }
        return ( actual_num_vs_sets );
}
int     SP_scat_get_vs_set_members( const scatter *memb_mess_scat, 
                                    const vs_set_info *vs_set, 
                                    char member_names[][MAX_GROUP_NAME],
                                    int member_names_count)
{
        if (member_names_count < (int) vs_set->num_members)
                return(BUFFER_TOO_SHORT);

        scat_read( member_names, memb_mess_scat, vs_set->members_offset, vs_set->num_members * MAX_GROUP_NAME);
        return( vs_set->num_members );
}


int	SP_query_groups( mailbox mbox, int max_groups, char *groups[MAX_GROUP_NAME] )
{
	return( -1 );
}

int	SP_query_members( mailbox mbox, char *group, int max_members, char *members[MAX_GROUP_NAME] )
{
	return( -1 );
}

void	SP_kill( mailbox mbox )
{
	int	ses;

	Mutex_lock( &Struct_mutex );

	/* get mbox out of the data structures */

	ses = SP_get_session( mbox );

	if( ses < 0 )
        { 
		Alarm( SESSION, "SP_kill: killing a non existent session for mailbox %d (likely race condition)!!!\n", mbox );
		Mutex_unlock( &Struct_mutex );
		return;
	}

	Sessions[ses].mbox  = -1;
	Sessions[ses].state = SESS_UNUSED;
	close(mbox);

	Num_sessions--;

	Mutex_unlock( &Struct_mutex );
}

static	int	SP_get_session( mailbox mbox )
{
        int ses      = MBOX_TO_BASE_SES(mbox);
        int base_ses = ses;

	while( Sessions[ses].mbox != mbox )
        {
	        if( ++ses == MAX_LIB_SESSIONS ) {
		        ses = 0;         /* wrap around to beginning of array */
		}

		if( ses == base_ses ) {  /* searched entire array */
		        ses = -1;        /* not found */
			break;
		}
	}

	return ses;
}

const char *SP_strerror( int error )
{
        const char *ret = "Unrecognized error.";
        
	switch( error )
	{
        case ILLEGAL_SPREAD:       ret = "Illegal spread name provided.";                                 break;
        case COULD_NOT_CONNECT:    ret = "Could not connect.";                                            break;
        case REJECT_QUOTA:         ret = "Connection rejected, too many users.";                          break;
        case REJECT_NO_NAME:       ret = "Connection rejected, no name was supplied.";                    break;
        case REJECT_ILLEGAL_NAME:  ret = "Connection rejected, illegal name.";                            break;
        case REJECT_NOT_UNIQUE:    ret = "Connection rejected, name not unique.";                         break;
        case REJECT_VERSION:       ret = "Connection rejected, client library incompatible with daemon."; break;
        case CONNECTION_CLOSED:    ret = "Connection closed by Spread.";                                  break;
        case REJECT_AUTH:          ret = "Connection rejected, authentication failed.";                   break;
        case ILLEGAL_SESSION:      ret = "Illegal session was supplied.";                                 break;
        case ILLEGAL_SERVICE:      ret = "Illegal service request.";                                      break;
        case ILLEGAL_MESSAGE:      ret = "Illegal message.";                                              break;
        case ILLEGAL_GROUP:        ret = "Illegal group.";                                                break;
        case BUFFER_TOO_SHORT:     ret = "Buffer too short.";                                             break;
        case GROUPS_TOO_SHORT:     ret = "Groups list too short.";                                        break;
        case MESSAGE_TOO_LONG:     ret = "Message (body + group names) too large.";                       break;
        case NET_ERROR_ON_SESSION: ret = "Network socket error. SP_kill() the mailbox.";                  break;
        case SP_BUG:               ret = "Internal Spread bug detected.";                                 break;
        case ILLEGAL_TIME:         ret = "Illegal time.";                                                 break;
	}

        return ret;
}

void	SP_error( int error )
{
        Alarm(PRINT, "SP_error: (%d) %s\n", error, SP_strerror(error));
}

static	void    Flip_mess( message_header *head_ptr )
{
	head_ptr->type     	= Flip_int32( head_ptr->type );
	head_ptr->num_groups	= Flip_int32( head_ptr->num_groups );
	head_ptr->data_len 	= Flip_int32( head_ptr->data_len );
}
