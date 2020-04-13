
FString ENGINE_API GetCDKeyHash();
FString ENGINE_API GetCDKeyResponse(const TCHAR* Challenge);


class ENGINE_API CDKeyResponseInterface{
public:
	CDKeyResponseInterface();
	CDKeyResponseInterface(const CDKeyResponseInterface& Other);
	~CDKeyResponseInterface();
	CDKeyResponseInterface& operator=(const CDKeyResponseInterface& Other);

	static FString GetCDKey();
	static CDKeyResponseInterface* GetCDKeyInterface();

	virtual void GetResponse(char* Challenge, char* CDKey, char* Response);

protected:
	static CDKeyResponseInterface* mpCDKeyInterface;
	static FString msCDKeyString;
};


