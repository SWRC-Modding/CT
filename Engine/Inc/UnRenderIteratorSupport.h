/*----------------------------------------------------------------------------
	ActorBuffer.
----------------------------------------------------------------------------*/

struct ActorBuffer
{
	BYTE Buffer[ sizeof(AActor) ];
};

/*----------------------------------------------------------------------------
	FActorNode.
----------------------------------------------------------------------------*/

class ENGINE_API FActorNode
{
public:
	ActorBuffer ActorProxy;
	FActorNode* NextNode;

	// Constructors.
	FActorNode(): NextNode( NULL ) {}
	~FActorNode() 
	{
		if( NextNode != NULL )
			delete NextNode;
	}
};
