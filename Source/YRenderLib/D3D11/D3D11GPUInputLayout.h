#pragma once
#include "YRenderLib/D3D11/D3D11Common.h"
#include "YRenderLib/Renderer.h"

class D3D11GPUInputLayout : public GPUInputLayout
{
public:
    D3D11GPUInputLayout(GPU_VERTEX_ELEMENT_DESC* pElements, uint32 nElements, ID3D11InputLayout* pD3DInputLayout);
    ~D3D11GPUInputLayout();

    ID3D11InputLayout* GetD3DInputLayout() const { return m_pD3DInputLayout; }

    virtual void GetMemoryUsage(uint32 *cpuMemoryUsage, uint32 *gpuMemoryUsage) const override;
    virtual void SetDebugName(const char *name) override;

protected:
    ID3D11InputLayout* m_pD3DInputLayout;
};

