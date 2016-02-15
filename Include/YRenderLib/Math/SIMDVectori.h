#pragma once
#include "YRenderLib/Math/Common.h"

// Integer vector classes only available with SSE2.
#if Y_CPU_SSE_LEVEL >= 2
    #include "YRenderLib/Math/SIMDVectori_sse.h"
#else
    #include "YRenderLib/Math/SIMDVectori_scalar.h"
#endif
