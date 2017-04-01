/*
 * tlibrtems.cxx
 *
 * Miscelaneous class implementation
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 */


extern "C" {
#include <netinet/in.h>
#include <rtems/rtems_bsdnet.h>
  

int socketpair(int d, int type, int protocol, int sv[2])
{
    static int port_count = IPPORT_USERRESERVED;
    int s;
    int addrlen;
    struct sockaddr_in addr1, addr2;
    static int network_status = 1;
    

    if (network_status>0)
    {
        printf("\"Network\" initializing!\n");
        network_status = rtems_bsdnet_initialize_network();
  if (network_status == 0)
      printf("\"Network\" initialized!\n");
  else
  {
      printf("Error: %s\n", strerror(errno));
      return -1;
  }
    }

    /* prepare sv */
    sv[0]=sv[1]=-1;

    /* make socket */
    s = socket( d, type, protocol);
    if (s<0) 
        return -1;

    memset(&addr1, 0, sizeof addr1);
    addr1.sin_family = d;
    addr1.sin_port = htons(++port_count);
    addr1.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(s, (struct sockaddr *)&addr1, sizeof addr1) < 0) 
    {
  close(s);
        return -1;
    }
    if (listen(s, 2) < 0 ) 
    {
  close(s);
        return -1;
    }
    
    sv[0] = socket(d, type, protocol);
    if (sv[0] < 0) 
    {
  close(s);
        return -1;
    }
    
    memset(&addr2, 0, sizeof addr2);
    addr2.sin_family = d;
    addr2.sin_port = htons(++port_count);
    addr2.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(sv[0], (struct sockaddr *)&addr2, sizeof addr2) < 0)
    {
  close(s);
  close(sv[0]);
  sv[0]=-1;
        return -1;
    }
    if (connect(sv[0], (struct sockaddr *)&addr1, sizeof addr1) < 0)
    {
  close(s);
  close(sv[0]);
  sv[0]=-1;
        return -1;
    }
    
    sv[1] = accept(s, (struct sockaddr *)&addr2, &addrlen);
    if (sv[1] < 0)
    {
  close(s);
  close(sv[0]);
  sv[0]=-1;
        return -1;
    }

    close(s);
    return 0;
}

/*
 * Loopback interface
 */
extern int rtems_bsdnet_loopattach(rtems_bsdnet_ifconfig *);
static struct rtems_bsdnet_ifconfig loopback_config = {
    "lo0",                          /* name */
    rtems_bsdnet_loopattach,        /* attach function */

    NULL,                           /* link to next interface */

    "127.0.0.1",                    /* IP address */
    "255.0.0.0",                    /* IP net mask */
};

#include <bsp.h>
#warning Change lines below to match Your system settings

/*
 * Default network interface
 */
static struct rtems_bsdnet_ifconfig netdriver_config = {
    RTEMS_BSP_NETWORK_DRIVER_NAME,          /* name */
    RTEMS_BSP_NETWORK_DRIVER_ATTACH,        /* attach function */

    &loopback_config,                       /* link to next interface */

    "10.0.0.2",                             /* IP address */
    "255.255.255.0",                        /* IP net mask */

    NULL,                                   /* Driver supplies hardware address */
    0                                       /* Use default driver parameters */
};

/*
 * Network configuration
 */
struct rtems_bsdnet_config rtems_bsdnet_config = {
    &netdriver_config,

    NULL,                   /* no bootp function */

    1,                      /* Default network task priority */
    0,                      /* Default mbuf capacity */
    0,                      /* Default mbuf cluster capacity */

    "computer.name",        /* Host name */
    "domain.name",          /* Domain name */
    "10.0.0.1",             /* Gateway */
    "10.0.0.1",             /* Log host */
    {"10.0.0.1" },          /* Name server(s) */
    {"10.0.0.1" },          /* NTP server(s) */

};

#define CONFIGURE_TEST_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_TEST_NEEDS_CLOCK_DRIVER
#define CONFIGURE_TEST_NEEDS_TIMER_DRIVER

#define CONFIGURE_MICROSECONDS_PER_TICK                         1000
#define CONFIGURE_TICKS_PER_TIMESLICE                             50

#define CONFIGURE_MAXIMUM_TASKS          rtems_resource_unlimited(50)
#define CONFIGURE_MAXIMUM_TIMERS         rtems_resource_unlimited(50)
#define CONFIGURE_MAXIMUM_SEMAPHORES     rtems_resource_unlimited(50)
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES rtems_resource_unlimited(50)
#define CONFIGURE_MAXIMUM_MUTEXES        rtems_resource_unlimited(50)

#define CONFIGURE_MAXIMUM_POSIX_THREADS                          500
#define CONFIGURE_MAXIMUM_POSIX_MUTEXES                          500
#define CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES              500
#define CONFIGURE_MAXIMUM_POSIX_KEYS                             500
#define CONFIGURE_MAXIMUM_POSIX_TIMERS                           500
#define CONFIGURE_MAXIMUM_POSIX_QUEUED_SIGNALS                   500
#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES                   500
#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES                       500

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS                 500
#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_POSIX_INIT_THREAD_TABLE
#define CONFIGURE_INIT_TASK_INITIAL_MODES (RTEMS_PREEMPT | RTEMS_TIMESLICE)

#ifdef _DEBUG
#define STACK_CHECKER_ON
#endif

void* POSIX_Init(void*);
#define CONFIGURE_INIT
#include <confdefs.h>
}

