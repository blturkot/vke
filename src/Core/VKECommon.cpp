#include "Core/VKECommon.h"
#if VKE_WINDOWS
#define NOMINMAX
#include <windows.h>
#endif // VKE_WINDOWS

#include "Core/Utils/CLogger.h"

namespace VKE
{
    void DebugBreak(cstr_t pBuff)
    {
#if VKE_WINDOWS
        //::OutputDebugStringA( pBuff );
        VKE_LOGGER_LOG_ERROR( VKE_FAIL, pBuff );
        __debugbreak();
#else

#endif
    }
}