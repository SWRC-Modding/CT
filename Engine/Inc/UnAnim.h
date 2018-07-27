/*=============================================================================
	UnAnim.h: 

	Revision history:
		* Created by Erik de Neve
		* Moved FMeshAnimNotify/FMeshAnimSeq from UnMesh.h to UnAnim.h
=============================================================================*/





/*-----------------------------------------------------------------------------
	FMeshAnimNotify.
-----------------------------------------------------------------------------*/

// An actor notification event associated with an animation sequence.
struct FMeshAnimNotify
{
	FLOAT	Time;			// Time to occur, 0.0-1.0.
	FName	Function;		// Name of the actor function to call.
	friend FArchive &operator<<( FArchive& Ar, FMeshAnimNotify& N )
		{return Ar << N.Time << N.Function;}
	FMeshAnimNotify()
		: Time(0.0), Function(NAME_None) {}
};

/*-----------------------------------------------------------------------------
	FMeshAnimSeq.
-----------------------------------------------------------------------------*/

// Information about one animation sequence associated with a mesh,
// a group of contiguous frames.
struct FMeshAnimSeq
{
	FName					Name;		// Sequence's name.
	FName					Group;		// Group.
	INT						StartFrame;	// Starting frame number.
	INT						NumFrames;	// Number of frames in sequence.
	FLOAT					Rate;		// Playback rate in frames per second.
	TArray<FMeshAnimNotify> Notifys;	// Notifications.
	friend FArchive &operator<<( FArchive& Ar, FMeshAnimSeq& A );
		//{return Ar << A.Name << A.Group << A.StartFrame << A.NumFrames << A.Notifys << A.Rate;}
	FMeshAnimSeq()
		: Name(NAME_None), Group(NAME_None), StartFrame(0), NumFrames(0), Rate(30.0), Notifys() {}
};



/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
