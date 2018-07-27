/*=============================================================================
	UnPath.h: Path node creation and ReachSpec creations and management specific classes
	Copyright 1997-2002 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Steven Polge 3/97
=============================================================================*/

class ENGINE_API FPathBuilder{
public:
	FPathBuilder();

	bool BuildOnTheFly() const;
	void DeleteScout();
	void EditCurrentLink();
	int GetCurLinkIndex() const;
	class FColor& GetRenderColor(class ANavigationPoint*);
	class FPathingStatistics& GetStats();
	bool IsRebuilding() const;
	void NextLinkIndex();
	void Reset(class ULevel*);
	void ReviewPaths(class ULevel*);
	void SetAdjustRadii(bool);
	void SetBuildOnTheFly(bool);
	void SetRebuilding(bool);
	bool ShouldAdjustRadii() const;
	void defineChangedPaths(class ULevel*);
	void definePaths(class ULevel*);
	class AScout* getScout();
	void undefinePaths(class ULevel*);

private:
	char Padding[32];

	void GenerateAllRegions();
	void SetPathCollision(int);
};

