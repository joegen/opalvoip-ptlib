#ifndef _BDEFS_h
#define _BDEFS_h
//
//	BeOS port of PWLib by Yuri Kiryanov 
//  mailto:netvideo@xcarrier.net
//
//	This file is created is to cheat PWLib's excessive demands and 
//	make a build! Failure is no option.
//	Later I'll do proper stuff

// Added BeOS stuff to pwlib/include/ptlib/unix/ptlib/pmachdep.h 
// /pwlib/src/ptlib/unix/udll.cxx - all commented
// pwlib/src/ptlib/unix/tlib.cxx - wait3 tweaked!

// Added to unix.mak
//
//ifeq ($(OSTYPE),beos)

//# BeOS R4, using gcc from Cygnus
//STDCCFLAGS	:= $(STDCCFLAGS) -DP_HAS_INT64

//endif # beos
//

// Sockets

// Fix to: PICMPSocket::OpenSocket(...)

#define SOCK_RAW SOCK_DGRAM // raw sockets not supported by current release of Be, R4

// The getprotoent(), getprotobyname(), and getprotobynumber() functions 
// each return a pointer to an object with the following structure contain╜ 
// ing the broken-out fields of a line in the network protocol data base, 
// /etc/protocols. 

struct  protoent { 
        char    *p_name;        /* official name of protocol */ 
        char    **p_aliases;    /* alias list */ 
        int     p_proto;        /* protocol number */ 
}; 

struct protoent * getprotobyname(const char *name); // Am I to write the function?

#define PF_INET AF_INET // True

// Fix to: PSocket::GetOption(...)

int	getsockopt (int, int, int, void *, int *); // And this one?

// Fix to: PSocket::GetNameByProtocol(...)
struct protoent * getprotobynumber(int proto);
          
// Fix to: PSocket::GetServiceByPort(...)
struct servent * getservbyport(int port, const char *proto);

// #warning Platform requires implemetation of GetRouteTable()! Am I ...

// From sys/ioccom.h
#define	IOCPARM_MASK	0x1fff		/* parameter length, at most 13 bits */
#define	IOC_VOID	0x20000000	/* no parameters */
#define	IOC_OUT		0x40000000	/* copy out parameters */
#define	IOC_IN		0x80000000	/* copy in parameters */
#define	IOC_INOUT	(IOC_IN|IOC_OUT)
#define	_IOC(inout,group,num,len) \
	((unsigned long)(inout | ((len & IOCPARM_MASK) << 16) | ((group) << 8) | (num)))
#define	_IO(g,n)	_IOC(IOC_VOID,	(g), (n), 0)
#define	_IOR(g,n,t)	_IOC(IOC_OUT,	(g), (n), sizeof(t))
#define	_IOW(g,n,t)	_IOC(IOC_IN,	(g), (n), sizeof(t))
#define	_IOWR(g,n,t)	_IOC(IOC_INOUT,	(g), (n), sizeof(t))

// Fix to: int PSocket::os_connect(int af, int type, int protocol)
#define	FIONBIO		_IOW('f', 126, int)	/* set/clear non-blocking i/o */
#define	FIONREAD	_IOR('f', 127, int)	/* get # bytes to read */

// Fix to: int PSocket::os_connect(struct sockaddr * addr, PINDEX size)
#define SO_SNDBUF	0x1001		/* send buffer size */
#define SO_RCVBUF	0x1002		/* receive buffer size */
#define	SO_ERROR	0x1007		/* get error status and clear */

// Fix to: PIPSocket::IsLocalHost()
#define	SIOCGIFCONF	_IOWR('i', 36, struct ifconf)	/* get ifnet list */

// From net/if.h
struct	ifreq {
#define	IFNAMSIZ	16
	char	ifr_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	union {
		struct	sockaddr ifru_addr;
		struct	sockaddr ifru_dstaddr;
		struct	sockaddr ifru_broadaddr;
		short	ifru_flags;
		int	ifru_metric;
		int	ifru_mtu;
		int	ifru_phys;
		int	ifru_media;
		caddr_t	ifru_data;
	} ifr_ifru;
#define	ifr_addr	ifr_ifru.ifru_addr	/* address */
#define	ifr_dstaddr	ifr_ifru.ifru_dstaddr	/* other end of p-to-p link */
#define	ifr_broadaddr	ifr_ifru.ifru_broadaddr	/* broadcast address */
#define	ifr_flags	ifr_ifru.ifru_flags	/* flags */
#define	ifr_metric	ifr_ifru.ifru_metric	/* metric */
#define	ifr_mtu		ifr_ifru.ifru_mtu	/* mtu */
#define ifr_phys	ifr_ifru.ifru_phys	/* physical wire */
#define ifr_media	ifr_ifru.ifru_media	/* physical media */
#define	ifr_data	ifr_ifru.ifru_data	/* for use by interface */
};

struct	ifconf {
	int	ifc_len;		/* size of associated buffer */
	union {
		caddr_t	ifcu_buf;
		struct	ifreq *ifcu_req;
	} ifc_ifcu;
#define	ifc_buf	ifc_ifcu.ifcu_buf	/* buffer address */
#define	ifc_req	ifc_ifcu.ifcu_req	/* array of structures returned */
};
#define    IFF_UP          0x1             /* interface is up */ 

// From sys/sockio.h
#define	SIOCGIFFLAGS	_IOWR('i', 17, struct ifreq)	/* get ifnet flags */
#define	SIOCGIFADDR	_IOWR('i', 33, struct ifreq)	/* get ifnet address */

// Fix to: PEthSocket::EnumIpAddress(...)
#define	SIOCGIFNETMASK	_IOWR('i', 37, struct ifreq)	/* get net addr mask */
#define IFF_PROMISC     0x100           /* receive all packets */

// Fix to: PEthSocket::SetFilter(...)
#define	SIOCSIFFLAGS	 _IOW('i', 16, struct ifreq)	/* set ifnet flags */

// Fix to: PChannel::ConvertOSError(...)
#define	ETXTBSY		26		/* Text file busy */

// Fix to: PFile::GetInfo()
//
// Note: BeOS sockets are NOT files!
#define	S_ISSOCK(m)	(((m) & 0170000) == 0140000)	/* socket */

#endif // _BDEFS_h