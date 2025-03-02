#pragma once

#include "RenderSystem/Vulkan/Common.h"
#include "RenderSystem/CDDI.h"
#include "Core/Utils/TCDynamicRingArray.h"
#include "RenderSystem/Resources/CTexture.h"

namespace VKE
{
    namespace Vulkan
    {
        class CDeviceWrapper;
    }

    namespace RenderSystem
    {
        class CDevice;
        class CGraphicsContext;
        struct SFrameData;
        class CRenderingPipeline;

        namespace Managers
        {
            class CBackBufferManager;
        }

        struct VKE_API SBackBuffer
        {
            struct SAcquireElement
            {
                VkImageMemoryBarrier    vkBarrierAttachmentToPresent;
                VkImageMemoryBarrier    vkBarrierPresentToAttachment;
                DDITexture              hDDITexture = VK_NULL_HANDLE;
                DDITextureView          hDDITextureView = VK_NULL_HANDLE;
                TEXTURE_STATE           currentState = TextureStates::UNDEFINED;
                TEXTURE_STATE           oldState = TextureStates::UNDEFINED;
                //VkFramebuffer   vkFramebuffer = VK_NULL_HANDLE;
                VkCommandBuffer         vkCbAttachmentToPresent = VK_NULL_HANDLE;
                VkCommandBuffer         vkCbPresentToAttachment = VK_NULL_HANDLE;
                DDIFramebuffer          hDDIFramebuffer = DDI_NULL_HANDLE;
                CRenderPass*            pRenderPass = nullptr;
            };
            
            Threads::SyncObject SyncObj;
            SAcquireElement*    pAcquiredElement = nullptr;
            DDISemaphore        hDDIPresentImageReadySemaphore = DDI_NULL_HANDLE;
            DDISemaphore        hDDIQueueFinishedSemaphore = DDI_NULL_HANDLE;
            DDIFence            hDDIPresentImageReadyFence = DDI_NULL_HANDLE;
            RenderTargetHandle  hRenderTarget = INVALID_HANDLE;
            uint32_t            ddiBackBufferIdx = 0;
            bool                presentDone = true;
            bool                isReady = false;

            bool IsReady() const { return isReady; }
        };

        class VKE_API CSwapChain
        {
            friend class CGraphicsContext;
            friend class CCommandBuffer;
            struct SPrivate;

            using SAcquireElement = SBackBuffer::SAcquireElement;

            using BackBufferVec = Utils::TCDynamicRingArray< SBackBuffer >;
            using AcquireElementVec = Utils::TCDynamicArray< SAcquireElement >;
            using CPUFenceQueue = std::deque<NativeAPI::CPUFence>;
            using GPUFenceQueue = std::deque<NativeAPI::GPUFence>;
            using CBackBufferManager = Managers::CBackBufferManager;
            using UintQueue = std::queue<uint32_t>;

            public:

              struct SSwapChainBuffer
              {
                  TextureRefPtr pTexture;
              };
              struct SBackBuffer
              {
                  /// <summary>
                  /// API specified index of the buffer which is acquired.
                  /// For every frame it can be different index.
                  /// It is not related to index of this back buffer.
                  /// </summary>
                  uint32_t swapChainBufferIndex = UNDEFINED_U32;
                  NativeAPI::GPUFence hGPUFence = NativeAPI::Null;
                  NativeAPI::CPUFence hCPUFence = NativeAPI::Null;
                  NativeAPI::GPUFence hExternalGPUFence = NativeAPI::Null;
                  NativeAPI::CPUFence hExternalCpuFence = NativeAPI::Null;
                  /// <summary>
                  /// Index of this back buffer
                  /// </summary>
                  uint32_t index = UNDEFINED_U32;

                  SPresentInfo PresentInfo;
              };

            protected:

                using BackBufferArray = Utils::TCDynamicArray< SBackBuffer, Config::RenderSystem::SwapChain::MAX_BACK_BUFFER_COUNT >;
           
            public:

                CSwapChain(CGraphicsContext* pCtx);
                ~CSwapChain();

                void operator=(const CSwapChain&) = delete;

                Result Create(const SSwapChainDesc& Desc, CommandBufferPtr);
                void Destroy();

                //uint32_t GetPresentationElementCount() const { return m_vAcquireElements.GetCount(); }
                uint32_t GetBackBufferCount() const { return m_vBackBuffers.GetCount(); }

                Result Resize(uint32_t width, uint32_t height);

                const RenderSystem::SBackBuffer* SwapBuffers( bool waitForPresent );
                Result  SwapBuffers();
                Result SwapBuffers( const NativeAPI::GPUFence&, const NativeAPI::CPUFence& );
                Result              Present(NativeAPI::GPUFence hWaitOnGPUFence, NativeAPI::CPUFence hFrameFence);
                void                NotifyPresent();
                void                Invalidate();

                TextureRefPtr       GetBackBufferTexture();
                const NativeAPI::GPUFence& GetBackBufferGPUFence() const;

                const SSwapChainDesc& GetDesc() const { return m_Desc; }
                WindowPtr           GetWindow() { return m_Desc.pWindow; }

                void BeginPass(CommandBufferPtr pCb);
                void EndPass(CommandBufferPtr pCb);
                void BeginFrame(CommandBufferPtr pCb);
                void EndFrame(CommandBufferPtr pCb);

                CGraphicsContext* GetContext() const { return m_pCtx; }
                //RenderTargetHandle GetRenderTarget() const { return m_pCurrAcquireElement->hRenderTarget; }
                const DDIRenderPass& GetDDIRenderPass() const { return m_DDISwapChain.hDDIRenderPass; }
                CGraphicsContext* GetGraphicsContext() const { return m_pCtx; }

                TextureSize GetSize() const;

                const RenderSystem::SBackBuffer&  GetCurrentBackBuffer() const { return *m_pCurrBackBuffer; }

                RenderTargetHandle GetCurrentRenderTarget() const { return m_pCurrBackBuffer->hRenderTarget; }

                const DDISwapChain& GetDDIObject() const { return m_DDISwapChain.hSwapChain; }

                bool NeedRecreate() const { return m_needRecreate; }

            protected:

                RenderSystem::SBackBuffer* _GetNextBackBuffer();
                uint32_t            _GetCurrentImageIndex() const { return m_pCurrBackBuffer->ddiBackBufferIdx; }
                const RenderSystem::SBackBuffer& _GetCurrentBackBuffer() const
                {
                    return *m_pCurrBackBuffer;
                }

                Result              _CreateBackBuffers(uint32_t count, CommandBufferPtr);
                BackBufferVec&      _GetBackBuffers() { return m_vBackBuffers; }
                void                _Reset();
                //AcquireElementVec&  _GetAcquireElements() { return m_vAcquireElements; }

            protected:
              
                SSwapChainDesc              m_Desc;
                SDDISwapChainDesc           m_DDIDesc;
                AcquireElementVec           m_vAcquireElements;
                BackBufferVec               m_vBackBuffers;
                SSwapChainBuffer            m_aSwapChainBuffers[ Config::RenderSystem::SwapChain::MAX_BACK_BUFFER_COUNT ];
                BackBufferArray             m_vInternalBackBufers;
                uint32_t                    m_backBufferIdx = 0;
                /// <summary>
                /// On every swapbuffer, last backbuffer index is pushed
                /// </summary>
                UintQueue                   m_qAcquiredBuffers;
                GPUFenceQueue               m_qPresentFrameFences;
                CBackBufferManager*         m_pBackBufferMgr = nullptr;
                RenderSystem::SBackBuffer*  m_pCurrBackBuffer = nullptr;
                CGraphicsContext*           m_pCtx = nullptr;
                Threads::SyncObject         m_SyncObj;
                SDDISwapChain               m_DDISwapChain;
                SViewportDesc               m_CurrViewport;
                SScissorDesc                m_CurrScissor;
                //SPresentSurfaceCaps         m_PresentSurfaceCaps;
                RenderPassRefPtr            m_pRenderPass;
                //DDIRenderPass               m_hDDIRenderPass;
                Vulkan::Queue               m_pQueue = nullptr;
                std::atomic<uint32_t>       m_acquireCount = 0;
                //uint32_t                    m_currBackBufferIdx = 0;
                bool                        m_needPresent = false;
                bool                        m_needRecreate = false;
                
                
                VKE_DEBUG_CODE(VkSwapchainCreateInfoKHR m_vkCreateInfo);
        };
    } // RenderSystem
} // VKE
