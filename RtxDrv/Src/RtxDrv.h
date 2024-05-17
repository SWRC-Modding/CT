#ifndef RTXDRV_NATIVE_DEFS
#define RTXDRV_NATIVE_DEFS

#include "../../D3DDrv/Inc/D3DDrv.h"

#if SUPPORTS_PRAGMA_PACK
#pragma pack (push,4)
#endif

#ifndef RTXDRV_API
#define RTXDRV_API DLL_IMPORT
#endif

class RTXDRV_API URtxRenderDevice : public UD3DRenderDevice{
	DECLARE_CLASS(URtxRenderDevice,UD3DRenderDevice,CLASS_Config,RtxDrv)
public:
};

#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif

#endif // RTXDRV_NATIVE_DEFS
