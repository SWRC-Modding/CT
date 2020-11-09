struct FProfile{
	FString Name;
	FString UserIni;
	FString Directory;
	INT     Idk4; // Probably a UBOOL
};

// Name is misleading as this does not inherit from UObject
class ENGINE_API UProfileManager{
public:
	UProfileManager();
	~UProfileManager();

	static UProfileManager* GetManager();

	FString GetCurrentPlayerName();
	void ReloadProfile(int);

	virtual void Init(class UClient*, FString);
	virtual void Tick(FLOAT DeltaTime);
	virtual int Exec(const TCHAR* Cmd, FOutputDevice& Ar);
	virtual void Serialize(FArchive& Ar);
	virtual void Destroy();
	virtual FString GetProfileDirectory(const FString& Name);

protected:
	INT Idk; // Seems to be uninitialized
	TArray<FProfile> Profiles;
	INT CurrentProfile;
	FString DirectoryPrefix;

	static UProfileManager* msProfileMgr;

	virtual int VerifyProfile(const FString&);
	virtual int NewProfile(const FString&, int);
	virtual int LoadProfile(int, int, bool);
	virtual int UpdateProfile(int);
	virtual int DeleteProfile(int, int, int);
	virtual int AddProfileInfo(const FString&, int);

	int FindProfileIndex(const FString& Name);
	FString FindProfiles(int);
	FString FormatProfileList(TArray<FString>&);
	FString GetProfileFilename(const FString&, int);
	FString GetProfilesRoot();
	FString GetSystemDirectory();
	int LoadProfileConfig(int, int);
	int ProfileInUse(int);
	void UpdateURL(class FURL&, const FString&, const FString&);
};
