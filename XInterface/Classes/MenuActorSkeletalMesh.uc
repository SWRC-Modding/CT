class MenuActorSkeletalMesh extends Actor
    NotPlaceable;

simulated event AnimEnd( int Channel )
{
    local MenuBase M;
    
    M = MenuBase(Owner);
    
    if( M != None )
        M.ChildAnimEnd( self, Channel );
}


defaultproperties
{
     DrawType=DT_Mesh
     bHidden=True
     RemoteRole=ROLE_None
     LODBias=100
}

