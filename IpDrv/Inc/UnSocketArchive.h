/*============================================================================
	UnSocketArchive.h: FArchive interface for TCP sockets.

	Revision history:
		* Created by Jack Porter
============================================================================*/

/*-----------------------------------------------------------------------------
	FArchiveSocket
-----------------------------------------------------------------------------*/

struct FArchiveSocket : public FArchive
{
	// tor.
	FArchiveSocket( INT InVersion, FInternetLink* InLink )
	:	FArchive()
	,	Link(InLink)
	{
		ArVer = InVersion;
		ArNetVer = InVersion | 0x80000000;
	}
protected:
	FInternetLink* Link;
};

/*-----------------------------------------------------------------------------
	FArchiveSocketReader - read a socket as an FArchive.
-----------------------------------------------------------------------------*/

struct FArchiveSocketReader : public FArchiveSocket
{
	// tor.
	FArchiveSocketReader( INT InVersion, FInternetLink* InLink  )
	:	FArchiveSocket( InVersion, InLink )
	{}

	// FArchiveSocketReader interface.
	virtual void ReceivedDataFromLink()=0;
};

/*-----------------------------------------------------------------------------
	FArchiveTCPReader - read from a TCP socket as an FArchive.
-----------------------------------------------------------------------------*/

struct FArchiveTCPReader : public FArchiveSocketReader
{
	// tor.
	FArchiveTCPReader( INT InVersion, FTcpLink* InLink  )
	:	FArchiveSocketReader( InVersion, InLink)
	{}

	// FArchive interface.
	virtual void Serialize( void* V, INT Length )
	{
		check( Length <= ReceiveData.Num() );
		appMemcpy( V, &ReceiveData(0), Length );
		ReceiveData.Remove( 0, Length );
	}

	// FArchiveSocketReader interface.
	virtual void ReceivedDataFromLink()
	{
		INT Length = Link->ReceivedData.Num();
		INT i = ReceiveData.Add( Length );
		Link->Recv( &ReceiveData(i), Length );
	}
private:
	TArray<BYTE> ReceiveData;
};

/*-----------------------------------------------------------------------------
	FArchiveTCPWriter - write to a TCP socket as an FArchive.
-----------------------------------------------------------------------------*/

struct FArchiveTCPWriter : public FArchiveSocket
{
	// tors.
	FArchiveTCPWriter( INT InVersion, FTcpLink* InLink  )
	:	FArchiveSocket( InVersion, InLink )
	{}
	virtual ~FArchiveTCPWriter()
	{
        Flush();
	}

	// FArchive interface.
	virtual void Serialize( void* V, INT Length )
	{
		INT i = SendData.Add(Length);
		appMemcpy( &SendData(i), V, Length );
	}
	virtual void Flush()
	{
		if( SendData.Num() )
		{
			Link->Send( &SendData(0), SendData.Num() );
			SendData.Empty();
		}
	}
private:
    TArray<BYTE> SendData;
};


/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
