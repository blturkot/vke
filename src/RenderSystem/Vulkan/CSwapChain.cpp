#if VKE_VULKAN_RENDER_SYSTEM
#include "RenderSystem/Vulkan/CSwapChain.h"
#include "RenderSystem/Vulkan/Vulkan.h"
#include "Core/Utils/CLogger.h"

#include "RenderSystem/CDeviceContext.h"
#include "CVkEngine.h"
#include "RenderSystem/Vulkan/CRenderSystem.h"
#include "Core/Platform/CWindow.h"
#include "Core/Memory/Memory.h"
#include "RenderSystem/Vulkan/PrivateDescs.h"

#include "RenderSystem/CGraphicsContext.h"
#include "RenderSystem/CDeviceContext.h"

#include "RenderSystem/Vulkan/Wrappers/CCommandBuffer.h"
#include "RenderSystem/Managers/CAPIResourceManager.h"
#include "RenderSystem/Vulkan/CRenderPass.h"
#include "RenderSystem/Vulkan/CRenderingPipeline.h"
#include "RenderSystem/Managers/CBackBufferManager.h"

#include <iostream>

namespace VKE
{
    namespace RenderSystem
    {

        CSwapChain::CSwapChain(CGraphicsContext* pCtx) :
            m_pCtx(pCtx)
        {
        }

        CSwapChain::~CSwapChain()
        {
            Destroy();
        }

        void CSwapChain::Destroy()
        {
            Memory::DestroyObject( &HeapAllocator, &m_pBackBufferMgr );
            m_pCtx->GetDeviceContext()->_NativeAPI().DestroySwapChain( &m_DDISwapChain, nullptr );
        }

        Result CSwapChain::Create(const SSwapChainDesc& Desc, CommandBufferPtr pCmdBuffer)
        {
            Result ret = VKE_OK;
            m_Desc = Desc;
            m_Desc.pPrivate = &m_DDIDesc;

            if( m_Desc.pWindow.IsNull() )
            {
                //auto pEngine = m_pCtx->GetDeviceContext()->GetRenderSystem()->GetEngine();
                //m_Desc.pWindow = pEngine->GetWindow();
            }
            //const SWindowDesc& WndDesc = m_Desc.pWindow->GetDesc();

            ret = m_pCtx->GetDeviceContext()->_NativeAPI().CreateSwapChain( m_Desc, nullptr, &m_DDISwapChain );
            if( VKE_FAILED( ret ) )
            {
                goto ERR;
            }

            m_Desc.backBufferCount = static_cast< uint16_t >( m_DDISwapChain.vImages.GetCount() );


            /// @todo check for fullscreen if format is 32bit

            ret = Memory::CreateObject( &HeapAllocator, &m_pBackBufferMgr, m_pCtx );
            if( VKE_SUCCEEDED( ret ) )
            {
                Managers::SBackBufferManagerDesc MgrDesc;
                MgrDesc.backBufferCount = m_Desc.backBufferCount;
                if( VKE_SUCCEEDED( m_pBackBufferMgr->Create( MgrDesc ) ) )
                {
                    SBackBuffer aData[Config::MAX_BACK_BUFFER_COUNT];
                    m_backBufferIdx = m_pBackBufferMgr->AddCustomData( reinterpret_cast<uint8_t*>(aData), sizeof( SBackBuffer ) );
                }
                else
                {
                    goto ERR;
                }
            }
            else
            {
                VKE_LOG_ERR( "Unable to allocate memory for BackBufferManager object." );
                goto ERR;
            }

            ret = _CreateBackBuffers( m_Desc.backBufferCount, pCmdBuffer );
            if( VKE_SUCCEEDED( ret ) )
            {
                VKE_ASSERT2(m_DDISwapChain.Size == m_Desc.Size, "Initialization Swapchain size must be the same as window");
                ret = Resize( m_Desc.Size.width, m_Desc.Size.height );
                if( VKE_SUCCEEDED( ret ) )
                {
                    // Set the current back buffer
                    //res = GetNextBackBuffer();
                    //ret = SwapBuffers();
                }
                else
                {
                    goto ERR;
                }
            }

            m_CurrViewport.Size = m_Desc.Size;
            m_CurrViewport.Position = {0.0f, 0.0f};
            m_CurrViewport.MinMaxDepth = { 0.0f, 1.0f };
            m_CurrScissor.Size = m_Desc.Size;
            m_CurrScissor.Position = { 0, 0 };

            _Reset();
            return ret;

        ERR:
            Destroy();
            return ret;
        }

        Result CSwapChain::_CreateBackBuffers(uint32_t count, CommandBufferPtr pCmdBuffer)
        {
            Result ret = VKE_OK;
            if( m_vBackBuffers.IsEmpty() )
            {
                if( !m_vBackBuffers.Resize( count ) || 
                    !m_vAcquireElements.Resize(count) ||
                    !m_vInternalBackBufers.Resize(count))
                {
                    ret = VKE_ENOMEMORY;
                }
                else
                {
                    /*CommandBufferPtr pCmdBuffer = m_pCtx->CreateCommandBuffer();
                    pCmdBuffer->Begin();*/
                    VkImageSubresourceRange SubresRange;
                    SubresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    SubresRange.baseArrayLayer = 0;
                    SubresRange.baseMipLevel = 0;
                    SubresRange.layerCount = 1;
                    SubresRange.levelCount = 1;

                    const uint32_t imgCount = m_DDISwapChain.vImages.GetCount();
                    const auto& vImages = m_DDISwapChain.vImages;
                    const auto& vImageViews = m_DDISwapChain.vImageViews;

                    for( uint32_t i = 0; i < imgCount; ++i )
                    {
                        RenderSystem::SBackBuffer& BackBuffer = m_vBackBuffers[ i ];
                        SBackBuffer& InternalBackBuffer = m_vInternalBackBufers[ i ];
                        InternalBackBuffer.index = i;

                        SAcquireElement& Element = m_vAcquireElements[i];
                        Element.hDDITexture = vImages[i];
                        Element.hDDITextureView = vImageViews[i];

                        {
                            SSemaphoreDesc Desc;
                            SFenceDesc FenceDesc;
                            Desc.SetDebugName( std::format( "VKE_SwapChain_GPUFence{}", i ).data() );
                            FenceDesc.SetDebugName( std::format( "VKE_SwapChain_CPUFence{}", i ).data() );
                            BackBuffer.hDDIPresentImageReadySemaphore = m_pCtx->GetDeviceContext()->_NativeAPI().CreateSemaphore( Desc, nullptr );
                            BackBuffer.hDDIQueueFinishedSemaphore = m_pCtx->GetDeviceContext()->_NativeAPI().CreateSemaphore( Desc, nullptr );
                            InternalBackBuffer.hGPUFence = m_pCtx->GetDeviceContext()->CreateGPUFence( Desc );
                            InternalBackBuffer.hCPUFence = m_pCtx->GetDeviceContext()->CreateCPUFence( FenceDesc );
                            if( BackBuffer.hDDIPresentImageReadySemaphore == DDI_NULL_HANDLE ||
                                BackBuffer.hDDIQueueFinishedSemaphore == DDI_NULL_HANDLE )
                            {
                                ret = VKE_FAIL;
                                break;
                            }
                        }
                        {
                            VkImageMemoryBarrier& ImgBarrier = Element.vkBarrierAttachmentToPresent;
                            Vulkan::InitInfo( &ImgBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER );
                            ImgBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                            ImgBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                            ImgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                            ImgBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                            ImgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                            ImgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                            ImgBarrier.image = Element.hDDITexture;
                            ImgBarrier.subresourceRange = SubresRange;
                        }
                        {
                            VkImageMemoryBarrier& ImgBarrier = Element.vkBarrierPresentToAttachment;
                            Vulkan::InitInfo( &ImgBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER );
                            ImgBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                            ImgBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                            ImgBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                            ImgBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                            ImgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                            ImgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                            ImgBarrier.image = Element.hDDITexture;
                            ImgBarrier.subresourceRange = SubresRange;
                        }


                        SCreateTextureDesc CreateTexDesc;
                        CreateTexDesc.Create.flags = Core::CreateResourceFlags::DEFAULT;
                        auto& TexDesc = CreateTexDesc.Texture;
                        TexDesc.format = m_DDISwapChain.Format.format;
                        TexDesc.arrayElementCount = 1;
                        TexDesc.memoryUsage = MemoryUsages::GPU_ACCESS | MemoryUsages::TEXTURE;
                        TexDesc.mipmapCount = 1;
                        TexDesc.multisampling = SampleCounts::SAMPLE_1;
                        TexDesc.Size = m_DDISwapChain.Size;
                        TexDesc.sliceCount = 1;
                        TexDesc.type = TextureTypes::TEXTURE_2D;
                        TexDesc.usage = TextureUsages::COLOR_RENDER_TARGET;
                        TexDesc.hNative = Element.hDDITexture;
                        TexDesc.hNativeView = Element.hDDITextureView;
                        TexDesc.Name = std::format( "SwapchainTexture_{}", i ).data();
                        TexDesc.SetDebugName( std::format( "SwapchainTexture_{}", i ).data() );
                        auto hTexture = m_pCtx->GetDeviceContext()->CreateTexture( CreateTexDesc );
                        auto hTextureView =
                            m_pCtx->GetDeviceContext()->GetTexture( hTexture )->GetView()->GetHandle();
                        auto pTexture = m_pCtx->GetDeviceContext()->GetTexture( hTexture );
                        // Set texture state as PRESENT
                        STextureBarrierInfo BarrierInfo;
                        pTexture->SetState( TextureStates::PRESENT, &BarrierInfo );
                        pCmdBuffer->Barrier( BarrierInfo );

                        SRenderTargetDesc RTDesc;
                        RTDesc.beginState = TextureStates::COLOR_RENDER_TARGET;
                        RTDesc.endState = TextureStates::PRESENT;
                        RTDesc.ClearValue = { 0.5, 0.5, 0.5, 1 };
                        RTDesc.format = m_DDISwapChain.Format.format;
                        RTDesc.memoryUsage = MemoryUsages::GPU_ACCESS | MemoryUsages::TEXTURE;
                        RTDesc.mipmapCount = 1;
                        RTDesc.multisampling = SampleCounts::SAMPLE_1;
                        RTDesc.renderPassUsage = RenderTargetRenderPassOperations::COLOR_CLEAR_STORE;
                        RTDesc.Size = m_DDISwapChain.Size;
                        RTDesc.type = TextureTypes::TEXTURE_2D;
                        RTDesc.usage = TextureUsages::COLOR_RENDER_TARGET;
                        RTDesc.hTexture = hTexture;
                        RTDesc.Name = std::format( "SwapchainRenderTarget_{}", i ).data();
                        RTDesc.SetDebugName(RTDesc.Name.GetData());
                        BackBuffer.hRenderTarget = m_pCtx->GetDeviceContext()->CreateRenderTarget( RTDesc );

                        m_aSwapChainBuffers[i].pTexture = pTexture;
                    }
                }
            }
            if( ret == VKE_OK )
            {
                m_pBackBufferMgr->UpdateCustomData( m_backBufferIdx, &m_vBackBuffers[0] );
                m_pCurrBackBuffer = _GetNextBackBuffer();
            }
            return ret;
        }

        Result CSwapChain::Resize(uint32_t width, uint32_t height)
        {
            Result ret = VKE_OK;
            // Do nothing if size is not changed
            if( m_DDISwapChain.Size.width != width || m_DDISwapChain.Size.height != height )
            {
                m_Desc.Size.width = static_cast< uint16_t >( width );
                m_Desc.Size.height = static_cast< uint16_t >( height );

                ret = m_pCtx->GetDeviceContext()->NativeAPI().ReCreateSwapChain( m_Desc, &m_DDISwapChain );
                if( VKE_SUCCEEDED( ret ) )
                {
                    m_CurrViewport.Size = m_Desc.Size;
                    m_CurrScissor.Size = m_Desc.Size;
                }
                _Reset();
            }

            return ret;
        }

        void CSwapChain::_Reset()
        {
            m_acquireCount = 0;
            m_needRecreate = false;
            m_needPresent = false;
            // Clear present data
            UintQueue sqEmpty;
            std::swap( m_qAcquiredBuffers, sqEmpty );
            m_backBufferIdx = 0;
            // Get new texture present index
            /*auto& Buffer = m_vInternalBackBufers[ m_backBufferIdx ];
            SDDIGetBackBufferInfo Info;
            Info.hSignalGPUFence = Buffer.hGPUFence;
            Info.waitTimeout = 0;

            m_pCtx->GetDeviceContext()->_NativeAPI().GetCurrentBackBufferIndex( m_DDISwapChain, Info,
                                                                                      &Buffer.swapChainBufferIndex );*/
        }

        RenderSystem::SBackBuffer* CSwapChain::_GetNextBackBuffer()
        {
            /*m_currBackBufferIdx++;
            m_currBackBufferIdx %= m_Desc.elementCount;
            m_pCurrBackBuffer = &m_vBackBuffers[ m_currBackBufferIdx ];*/
            m_pBackBufferMgr->AcquireNextBuffer();
            RenderSystem::SBackBuffer* pBackBuffer
                = reinterpret_cast<RenderSystem::SBackBuffer*>( m_pBackBufferMgr->GetCustomData( m_backBufferIdx ) );
            return pBackBuffer;
        }

        /*void CSwapChain::EndPresent()
        {
            auto& VkInternal = m_pPrivate->Vulkan;
            assert(m_vkCurrQueue != VK_NULL_HANDLE);

            auto& PresentInfo = m_PresentInfo;
            PresentInfo.pImageIndices = &m_currImageId;
            PresentInfo.pWaitSemaphores = &m_pCurrElement->vkSemaphore;

            VK_ERR(m_pDeviceCtx->QueuePresentKHR(m_vkCurrQueue, PresentInfo));

            m_currElementId++;
            m_currElementId %= m_Desc.elementCount;
        }*/

        const RenderSystem::SBackBuffer* CSwapChain::SwapBuffers( bool waitForPresent )
        {
            RenderSystem::SBackBuffer* pRet = nullptr;
            // do not acquire more than presented
            const uint32_t elCount = m_Desc.backBufferCount - waitForPresent;
            //const bool b = m_acquireCount < elCount;
            //VKE_LOG( "" << b << " m_acquireCount = " << m_acquireCount << " < elCount = " << elCount << "(" << waitForPresent<< ")" );
            if( m_acquireCount < elCount &&
                !m_needRecreate )
            {
                //if( m_pCurrBackBuffer->IsReady() )
                {
                    m_pCurrBackBuffer = _GetNextBackBuffer();
                    /*if( m_pCurrBackBuffer->presentDone )
                    {
                        pRet = m_pCurrBackBuffer;
                        m_pCurrBackBuffer->presentDone = false;
                    }*/
                }

                SDDIGetBackBufferInfo Info;
                Info.hSignalGPUFence = m_pCurrBackBuffer->hDDIPresentImageReadySemaphore;
                Info.waitTimeout = 0;
                Result res = m_pCtx->GetDeviceContext()->_NativeAPI().GetCurrentBackBufferIndex( m_DDISwapChain,
                    Info, &m_pCurrBackBuffer->ddiBackBufferIdx );

                if( VKE_SUCCEEDED( res ) )
                {
                    m_pCurrBackBuffer->isReady = true;
                    pRet = m_pCurrBackBuffer;
                    m_pCurrBackBuffer->pAcquiredElement = &m_vAcquireElements[ m_pCurrBackBuffer->ddiBackBufferIdx ];
                    m_acquireCount++;
                }
                else if( res == Results::NOT_READY  )
                {
                    m_pCurrBackBuffer->isReady = false;
                    pRet = m_pCurrBackBuffer;
                }
            }
            else
            {
                //VKE_LOG_WARN( "Wait for present?" );
            }
            return pRet;
        }

        Result CSwapChain::SwapBuffers( const NativeAPI::GPUFence& hGPUFence,
                                        const NativeAPI::CPUFence& hCPUFence )
        {
            VKE_ASSERT( !( hCPUFence != NativeAPI::Null && hGPUFence != NativeAPI::Null ) );

            Result ret = VKE_FAIL;
            while (m_qAcquiredBuffers.size() > m_qPresentFrameFences.size())
            {
                Platform::ThisThread::Pause();
            }
            if( !m_qPresentFrameFences.empty() )
            {
                m_qPresentFrameFences.pop_front();
            }
            const auto& PrevBuffer = m_vInternalBackBufers[ m_backBufferIdx ];
            ( void )PrevBuffer;

            m_backBufferIdx = ( m_backBufferIdx + 1 ) % m_vInternalBackBufers.GetCount();
            auto& Buffer = m_vInternalBackBufers[ m_backBufferIdx ];
            Buffer.hExternalCpuFence = hCPUFence;
            Buffer.hExternalGPUFence = hGPUFence;
            // Get new texture present index
            SDDIGetBackBufferInfo Info;
            Info.hSignalGPUFence = hGPUFence;
            Info.hSignalCPUFence = hCPUFence;
            Info.waitTimeout
                = ( hCPUFence == NativeAPI::Null && hGPUFence == NativeAPI::Null ) ? UINT64_MAX : 0;
            //std::unique_lock<std::mutex> l( m_mutex );
            // This sync is workaround of validation error when swapchain is
            // used in more threads.
            // Present and get next image can be used in parallel.
            
            {
                Threads::ScopedLock l( m_SyncObj );
                ret = m_pCtx->GetDeviceContext()->_NativeAPI().GetCurrentBackBufferIndex(
                    m_DDISwapChain, Info, &Buffer.swapChainBufferIndex );
            }
            // VKE_LOG( "Result: " << ret << ", signal gpu fence: " << ( void* )Info.hSignalGPUFence );
            // In case when there are more frames rendered than it can be presented
            // a driver can return not_ready result as the present surface is still to be
            // presented. Brute-force query for the surface instead of return fail.
            /// TODO: implement timeout to not get into endless loop.
            while(ret == VKE_ENOTREADY)
            {
                Threads::ScopedLock l( m_SyncObj );
                ret = m_pCtx->GetDeviceContext()->_NativeAPI().GetCurrentBackBufferIndex(
                    m_DDISwapChain, Info, &Buffer.swapChainBufferIndex );
            }
            VKE_ASSERT( VKE_SUCCEEDED( ret ) );
            if( VKE_SUCCEEDED( ret ) )
            {
                //VKE_LOG( "Acquire with m_qAcquiredBuffers: " << m_qAcquiredBuffers.size()
                //                                             << " img idx: " << Buffer.swapChainBufferIndex );
                Buffer.PresentInfo.pSwapChain = this;
                Buffer.PresentInfo.imageIndex = Buffer.swapChainBufferIndex;
                m_qAcquiredBuffers.push( m_backBufferIdx );
                /*Platform::Debug::PrintOutput( "Swap %d %d\n", GetBackBufferTexture()->GetDDIObject(),
                                              Buffer.swapChainBufferIndex );*/
            }
            return ret;
        }

        Result CSwapChain::SwapBuffers()
        {
            auto& Buffer = m_vInternalBackBufers[ m_backBufferIdx ];
            return SwapBuffers(Buffer.hGPUFence, NativeAPI::Null);
        }

        TextureRefPtr CSwapChain::GetBackBufferTexture()
        {
            auto bufferIndex = m_vInternalBackBufers[ m_backBufferIdx ].swapChainBufferIndex;
            //VKE_LOG( "swpchainIdx: " << bufferIndex );
            return m_aSwapChainBuffers[ bufferIndex ].pTexture;
        }

        const NativeAPI::GPUFence& CSwapChain::GetBackBufferGPUFence() const
        {
            return m_vInternalBackBufers[ m_backBufferIdx ].hGPUFence;
        }

        Result CSwapChain::Present(NativeAPI::GPUFence hWaitOnGPUFence, NativeAPI::CPUFence hFrameFence)
        {
            Result ret = VKE_ENOTREADY;
            if( m_qAcquiredBuffers.size() > 0 )
            {
                uint32_t backBufferIndex = m_qAcquiredBuffers.front();
                m_qAcquiredBuffers.pop();
                auto& Buffer = m_vInternalBackBufers[ backBufferIndex ];
                VKE_ASSERT( backBufferIndex == Buffer.index );
                Buffer.PresentInfo.hDDIWaitSemaphore = hWaitOnGPUFence;
                //VKE_LOG( "present img idx: " << Buffer.PresentInfo.imageIndex << " push frame fence: " << hFrameFence );
                //   This sync is workaround of validation error when swapchain is
                //  used in more threads.
                //  Present and get next image can be used in parallel.
                Threads::ScopedLock l( m_SyncObj );
                //VKE_LOG( "presentImgIndex: " << Buffer.PresentInfo.imageIndex );
                /*Platform::Debug::PrintOutput( "Present: %d, %d\n",
                    hWaitOnGPUFence, Buffer.PresentInfo.imageIndex );*/
                ret = m_pCtx->Present( Buffer.PresentInfo );
                if( VKE_SUCCEEDED(ret) )
                {
                    m_qPresentFrameFences.push_back( hWaitOnGPUFence );
                }
            }
            return ret;
        }

        void CSwapChain::NotifyPresent()
        {
            //Threads::ScopedLock l( m_pCurrBackBuffer->SyncObj );
            m_pCurrBackBuffer->presentDone = true;
            if( m_acquireCount > 0 )
            {
                // Debug Swapchain
                //VKE_LOG("release: " << m_acquireCount);
                m_acquireCount--;
                //VKE_LOG( "m_acquireCount = " << m_acquireCount );
            }
        }

        void CSwapChain::Invalidate()
        {
            m_needRecreate = true;
        }

        TextureSize CSwapChain::GetSize() const
        {
            //ExtentU32 Size = { m_vkSurfaceCaps.currentExtent.width, m_vkSurfaceCaps.currentExtent.height };
            //return Size;
            return m_DDISwapChain.Size;
        }

        void CSwapChain::BeginFrame(CommandBufferPtr pCb)
        {
            SAcquireElement* pElement = m_pCurrBackBuffer->pAcquiredElement;

            STextureBarrierInfo Info;
            Info.currentState = TextureStates::PRESENT;
            Info.newState = TextureStates::COLOR_RENDER_TARGET;
            Info.hDDITexture = pElement->hDDITexture;
            Info.srcMemoryAccess = MemoryAccessTypes::GPU_MEMORY_READ;
            Info.dstMemoryAccess = MemoryAccessTypes::COLOR_RENDER_TARGET_WRITE;
            Info.SubresourceRange.aspect = TextureAspects::COLOR;
            Info.SubresourceRange.beginArrayLayer = 0;
            Info.SubresourceRange.beginMipmapLevel = 0;
            Info.SubresourceRange.layerCount = 1;
            Info.SubresourceRange.mipmapLevelCount = 1;
            pCb->Barrier( Info );
            //m_pCurrBackBuffer->pAcquiredElement->oldState = m_pCurrBackBuffer->pAcquiredElement->vkBarrierPresentToAttachment.oldLayout;
            //m_pCurrBackBuffer->pAcquiredElement->vkCurrLayout = m_pCurrBackBuffer->pAcquiredElement->vkBarrierPresentToAttachment.newLayout;
        }

        void CSwapChain::EndFrame(CommandBufferPtr pCb)
        {

            SAcquireElement* pElement = m_pCurrBackBuffer->pAcquiredElement;

            STextureBarrierInfo Info;
            Info.currentState = TextureStates::COLOR_RENDER_TARGET;
            Info.newState = TextureStates::PRESENT;
            Info.hDDITexture = pElement->hDDITexture;
            Info.srcMemoryAccess = MemoryAccessTypes::COLOR_RENDER_TARGET_WRITE;
            Info.dstMemoryAccess = MemoryAccessTypes::CPU_MEMORY_READ;
            Info.SubresourceRange.aspect = TextureAspects::COLOR;
            Info.SubresourceRange.beginArrayLayer = 0;
            Info.SubresourceRange.beginMipmapLevel = 0;
            Info.SubresourceRange.layerCount = 1;
            Info.SubresourceRange.mipmapLevelCount = 1;
            pCb->Barrier( Info );
            //m_pCurrBackBuffer->pAcquiredElement->vkOldLayout = m_pCurrBackBuffer->pAcquiredElement->vkBarrierAttachmentToPresent.oldLayout;
            //m_pCurrBackBuffer->pAcquiredElement->vkCurrLayout = m_pCurrBackBuffer->pAcquiredElement->vkBarrierAttachmentToPresent.newLayout;
        }

        void CSwapChain::BeginPass(CommandBufferPtr pCb)
        {
            VKE_ASSERT2( m_pRenderPass.IsValid(), "SwapChain RenderPass must be created." );
            pCb->Bind( m_DDISwapChain );
        }

        void CSwapChain::EndPass(CommandBufferPtr pCb)
        {
            //m_VkDevice.GetICD().vkCmdEndRenderPass(vkCb);
            //m_pCtx->GetDeviceContext()->_GetDDI().EndRenderPass( vkCb );
            //m_pCurrAcquireElement->pRenderPass->End( vkCb );
            pCb->Bind( RenderPassPtr() );
        }

    } // RenderSystem
} // VKE
#endif // VKE_VULKAN_RENDER_SYSTEM