#pragma once

#include "Core/Utils/TCSmartPtr.h"

namespace VKE
{
    class CVkEngine;
} // VKE

// Platform
namespace VKE
{
    class CWindow;
}

// Render system types
namespace VKE
{
    namespace Core
    {
        namespace Resources
        {
            class CImage;
        } // Resources
    } // Core
    class CRenderSystem;
    namespace RenderSystem
    {
        class CGraphicsContext;
        class CComputeContext;
        class CRenderTarget;
        class CPipeline;
        class CPipelineLayout;
        class CTexture;
        class CTextureView;
        class CBuffer;
        class CBufferView;
        using CVertexBuffer = CBuffer;
        using CIndexBuffer = CBuffer;
        class CShader;
        class CShaderProgram;
        class CVertexShader;
        class CPixelShader;
        class CGeometryShader;
        class CComputeShader;
        class CHullShader;
        class CDomainShader;
        class CSampler;
        class CImage;
        class CViewport;
        class CCommandBuffer;
        class CRenderQueue;
        class CRenderPass;
        class CPipelineLayout;
        class CSwapChain;
        class CFrameGraph;
        class CDeviceContext;
        class CGraphicsContext;
        class CTransferContext;
    } // Render System
} // VKE

// Scene
namespace VKE
{
    class CScene;
    class CSceneManager;
    class CScenePartition;
    class CScenePartitionOctree;
} // VKE

namespace VKE
{
    namespace Threads
    {
        class CThreadPool;
        class CThreadWorker;
    } // namespace Threads
} // VKE

namespace VKE
{
#define VKE_DECL_SMART_PTRS(_name) \
    using _name##Ptr = Utils::TCWeakPtr< C##_name >; \
    using _name##OwnPtr = Utils::TCUniquePtr< C##_name >; \
    using _name##RefPtr = Utils::TCObjectSmartPtr< C##_name >;

    namespace Core
    {
        namespace Resources
        {
            VKE_DECL_SMART_PTRS( Image );
        } // Resources
    } // Core

    VKE_DECL_SMART_PTRS(Window);
    namespace RenderSystem
    {
        VKE_DECL_SMART_PTRS( RenderTarget );
        VKE_DECL_SMART_PTRS( Pipeline );
        VKE_DECL_SMART_PTRS( Texture );
        VKE_DECL_SMART_PTRS( TextureView );
        VKE_DECL_SMART_PTRS( Buffer );
        VKE_DECL_SMART_PTRS( BufferView );
        VKE_DECL_SMART_PTRS( Shader );
        VKE_DECL_SMART_PTRS( Sampler );
        //VKE_DECL_SMART_PTRS( Image );
        VKE_DECL_SMART_PTRS( Viewport );
        VKE_DECL_SMART_PTRS( CommandBuffer );
        VKE_DECL_SMART_PTRS( RenderQueue );
        VKE_DECL_SMART_PTRS( VertexBuffer );
        VKE_DECL_SMART_PTRS( IndexBuffer );
        VKE_DECL_SMART_PTRS( RenderPass );
        VKE_DECL_SMART_PTRS( PipelineLayout );
    } // RnderSystem
} // VKE
