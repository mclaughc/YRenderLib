#pragma once
#include "YRenderLib/Math/Common.h"

// Integer matrix classes only available with SSE2.
//#if Y_CPU_SSE_LEVEL >= 2
    //#include "YRenderLib/Math/SIMDMatrixi_sse.h"
//#else
    #include "YRenderLib/Math/SIMDMatrixi_scalar.h"
//#endif
