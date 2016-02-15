#pragma once
#include "YRenderLib/Math/Common.h"

// Float vector classes on SSE+
#if Y_CPU_SSE_LEVEL > 0
    #include "YRenderLib/Math/SIMDVectorf_sse.h"
#else
    #include "YRenderLib/Math/SIMDVectorf_scalar.h"
#endif
