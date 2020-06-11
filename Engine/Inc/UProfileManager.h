struct FProfile{
	FString Idk1;
	FString Idk2;
	FString Idk3;
	INT     Idk4;
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
	virtual void Tick(float);
	virtual int Exec(const char*, FOutputDevice&);
	virtual void Serialize(FArchive&);
	virtual void Destroy();
	virtual FString GetProfileDirectory(FString const&);

protected:
	INT CurrentProfile;
	TArray<FProfile> Profiles;
	INT Idk1;
	FString Idk2;

	static UProfileManager* msProfileMgr;

	virtual int VerifyProfile(const FString&);
	virtual int NewProfile(const FString&, int);
	virtual int LoadProfile(int, int, bool);
	virtual int UpdateProfile(int);
	virtual int DeleteProfile(int, int, int);
	virtual int AddProfileInfo(const FString&, int);

	int FindProfileIndex(const FString&);
	FString FindProfiles(int);
	FString FormatProfileList(TArray<FString>&);
	FString GetProfileFilename(const FString&, int);
	FString GetProfilesRoot();
	FString GetSystemDirectory();
	int LoadProfileConfig(int, int);
	int ProfileInUse(int);
	void UpdateURL(class FURL&, const FString&, const FString&);
};
