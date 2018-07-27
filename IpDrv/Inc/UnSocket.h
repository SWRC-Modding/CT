/*============================================================================
	UnSocket.h: Common interface for WinSock and BSD sockets.

	Revision history:
		* Created by Mike Danylchuk
============================================================================*/

/*-----------------------------------------------------------------------------
	Definitions.
-----------------------------------------------------------------------------*/

#if __WINSOCK__
	typedef INT					__SIZE_T__;
	typedef INT					SOCKLEN;
	#define GCC_OPT_INT_CAST
	#define MSG_NOSIGNAL		0
    #define UDP_ERR_PORT_UNREACH WSAECONNRESET
#endif

// Provide WinSock definitions for BSD sockets.
#if __BSD_SOCKETS__
	typedef int					SOCKET;
	typedef struct hostent		HOSTENT;
	typedef in_addr				IN_ADDR;
	typedef struct sockaddr		SOCKADDR;
	typedef struct sockaddr_in	SOCKADDR_IN;
	typedef struct linger		LINGER;
	typedef struct timeval		TIMEVAL;
	typedef TCHAR*				LPSTR;
	typedef SIZE_T				__SIZE_T__;
	typedef socklen_t			SOCKLEN;


	#define INVALID_SOCKET		-1
	#define SOCKET_ERROR		-1

    #define UDP_ERR_PORT_UNREACH ECONNREFUSED

	//#define WSAEWOULDBLOCK		EWOULDBLOCK
	#define WSAEWOULDBLOCK		EAGAIN

	#define WSAENOTSOCK			ENOTSOCK
	#define WSATRY_AGAIN		TRY_AGAIN
	#define WSAHOST_NOT_FOUND	HOST_NOT_FOUND
	#define WSANO_DATA			NO_ADDRESS
	#define LPSOCKADDR			sockaddr*
    #define WSAECONNRESET		ECONNRESET

	#define closesocket			close
	#define ioctlsocket			ioctl
	#define WSAGetLastError()	errno

	#define GCC_OPT_INT_CAST	(DWORD*)
#endif

// IP address macros.
#if __WINSOCK__
	#define IP(sin_addr,n) sin_addr.S_un.S_un_b.s_b##n
#elif __BSD_SOCKETS__
	#define IP(sin_addr,n) ((BYTE*)&sin_addr.s_addr)[n-1]
#endif

/*-----------------------------------------------------------------------------
	Definitions.
-----------------------------------------------------------------------------*/

// An IP address (host byte order).
struct FIpAddr
{
	DWORD Addr;			// Host byte order
	DWORD Port;			// Host byte order

	// tors
	FIpAddr() {}
	FIpAddr( const TCHAR* IPString, INT InPort )
	{
		Addr = ntohl(inet_addr(TCHAR_TO_ANSI(IPString)));  // FIpAddr is in host byte order.
		Port = InPort;
	}
	FIpAddr( SOCKADDR_IN SockAddr );

	// equality
	UBOOL operator==(FIpAddr& Other)
	{
		return Addr==Other.Addr && Port==Other.Port;
	}

	// helpers
	FString GetString( UBOOL ShowPort );
	SOCKADDR_IN GetSockAddr();

	// serialization
    friend FArchive& operator<<( FArchive& Ar, FIpAddr& IP )
    {
        return Ar << IP.Addr << IP.Port;
    }
};

// SocketData.
struct FSocketData
{
	SOCKADDR_IN Addr;	// Hetwork byte order.
	INT Port;			// Host byte order.
	SOCKET Socket;

	void UpdateFromSocket()
	{
		SOCKLEN size = sizeof(Addr);
		getsockname ( Socket, (SOCKADDR*)(&Addr), &size);
		Port = ntohs(Addr.sin_port);
	}
	FString GetString( UBOOL ShowPort );
};

/*----------------------------------------------------------------------------
	Functions.
----------------------------------------------------------------------------*/

UBOOL InitSockets( FString& Error );
UBOOL SetNonBlocking( INT Socket );
UBOOL SetSocketReuseAddr( INT Socket, UBOOL ReUse=1 );
UBOOL SetSocketLinger( INT Socket );
UBOOL SetSocketRecvErr( INT Socket );
TCHAR* SocketError( INT Code=-1 );
UBOOL IpMatches( sockaddr_in& A, sockaddr_in& B );
void IpGetBytes( in_addr Addr, BYTE& Ip1, BYTE& Ip2, BYTE& Ip3, BYTE& Ip4 );
void IpSetBytes( in_addr& Addr, BYTE Ip1, BYTE Ip2, BYTE Ip3, BYTE Ip4 );
void IpGetInt( in_addr Addr, DWORD& Ip );
void IpSetInt( in_addr& Addr, DWORD Ip );
FString IpString( in_addr Addr, INT Port=0 );

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/

