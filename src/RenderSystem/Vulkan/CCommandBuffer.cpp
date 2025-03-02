#include "RenderSystem/Vulkan/CCommandBuffer.h"
#include "RenderSystem/CDeviceContext.h"
#include "RenderSystem/Resources/CShader.h"
#if VKE_VULKAN_RENDER_SYSTEM
#include "Core/Utils/CProfiler.h"
#include "RenderSystem/CContextBase.h"
#include "RenderSystem/CDeviceContext.h"
#include "RenderSystem/CRenderPass.h"
#include "RenderSystem/CSwapChain.h"
#include "RenderSystem/Managers/CBufferManager.h"
#include "RenderSystem/Managers/CStagingBufferManager.h"
#include "RenderSystem/Vulkan/Managers/CPipelineManager.h"
#include "RenderSystem/Vulkan/Managers/CSubmitManager.h"

#define  VKE_LOG_CB_ENABLE 0

#if VKE_LOG_CB_ENABLE
#define VKE_LOG_CB() VKE_LOG( "Command buffer: " << this << ": " << __FUNCSIG__ )
#else
#define VKE_LOG_CB()
#endif

namespace VKE
{
    namespace RenderSystem
    {
        static CPipeline g_sDummyPipeline = CPipeline( nullptr );
        static PipelinePtr g_spDummyPipeline( &g_sDummyPipeline );
        CCommandBuffer::CCommandBuffer()
            : m_pCurrentPipeline( g_spDummyPipeline )
        {
        }
        CCommandBuffer::~CCommandBuffer() {}
        void CCommandBuffer::Init( const SCommandBufferInitInfo& Info )
        {
            VKE_ASSERT2( m_hDDIObject != DDI_NULL_HANDLE, "" );
            VKE_ASSERT2( Info.pBaseCtx != nullptr, "" );
            _Reset();
            m_pBaseCtx = Info.pBaseCtx;
            // Init pipeline desc only one time
            if( m_pBaseCtx != nullptr )
            {
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
                m_CurrentPipelineDesc.Create.flags = Core::CreateResourceFlags::DEFAULT;
                m_CurrentPipelineDesc.Pipeline = SPipelineDesc();
                if( Info.initGraphicsShaders )
                {
                    m_CurrentPipelineDesc.Pipeline.Shaders.apShaders[ ShaderTypes::VERTEX ] =
                        m_pBaseCtx->m_pDeviceCtx->GetDefaultShader( ShaderTypes::VERTEX );
                    m_CurrentPipelineDesc.Pipeline.Shaders.apShaders[ ShaderTypes::PIXEL ] =
                        m_pBaseCtx->m_pDeviceCtx->GetDefaultShader( ShaderTypes::PIXEL );
                    m_CurrentPipelineDesc.Pipeline.hLayout =
                        PipelineLayoutHandle{ m_pBaseCtx->m_pDeviceCtx->GetDefaultPipelineLayout()->GetHandle() };
                }
                else if( Info.initComputeShader )
                {
                    m_CurrentPipelineDesc.Pipeline.hLayout =
                        PipelineLayoutHandle{ m_pBaseCtx->m_pDeviceCtx->GetDefaultPipelineLayout()->GetHandle() };
                }
#endif
            }
        }
        bool CCommandBuffer::IsExecuted()
        {
            return m_state == States::EXECUTED;
        }
        void CCommandBuffer::AddWaitOnSemaphore( const DDISemaphore& hDDISemaphore )
        {
            m_vDDIWaitOnSemaphores.PushBack( hDDISemaphore );
        }
        void CCommandBuffer::_BeginProlog()
        {
            //_Reset();
            /*if( m_pCurrentPipelineLayout.IsNull() )
            {
                {
                    SPipelineLayoutDesc Desc;
                    PipelineLayoutPtr pLayout =
            m_pBaseCtx->m_pDeviceCtx->CreatePipelineLayout( Desc ); VKE_ASSERT2(
            pLayout.IsValid(), "Invalid pipeline layout." );
                    m_pCurrentPipelineLayout = pLayout;
                }
            }
            if( m_pCurrentPipeline.IsNull() )
            {
                SetState( m_pCurrentPipelineLayout );
            }*/
        }
        void CCommandBuffer::Begin()
        {
            VKE_ASSERT( m_state == CommandBufferStates::RESET );
            _BeginProlog();
            auto pThis = this;
            _Reset();
            m_pBaseCtx->_BeginCommandBuffer( &pThis );
            m_state = States::BEGIN;
            VKE_LOG_CB();
            m_pMgr->_LogCommand( this, "Begin" );
        }

        void CCommandBuffer::_ExecutePendingOperations()
        {
            if( m_state == States::BEGIN )
            {
                if( m_isRenderPassBound )
                {
                    Bind( RenderPassPtr() );
                }
                if( m_needExecuteBarriers )
                {
                    ExecuteBarriers();
                }
            }
        }

        Result CCommandBuffer::End()
        {
            Result ret = VKE_OK;
            if( m_state != CCommandBuffer::States::END )
            {
                auto pThis = this;
                ret = m_pBaseCtx->_EndCommandBuffer( &pThis );
                VKE_LOG_CB();
                m_pMgr->_LogCommand( this, "End" );
            }
            return ret;
        }

        void CCommandBuffer::Reset()
        {
            VKE_ASSERT( m_state != CommandBufferStates::BEGIN );
            _NotifyExecuted();
            m_pBaseCtx->_Reset( this );
            m_state = CommandBufferStates::RESET;
            m_pMgr->_LogCommand( this, "Reset" );
        }

        Result CCommandBuffer::Flush()
        {
            Result ret = VKE_OK;
            DumpDebugMarkerTexts();
            VKE_ASSERT( m_state != States::UNKNOWN && m_state != States::EXECUTED );
            if( m_state < States::END )
            {
                ret = End();
            }
            m_state = States::FLUSH;
            VKE_LOG_CB();
            m_pMgr->_LogCommand( this, "Flush" );
            return ret;
        }

        void CCommandBuffer::Barrier( const SMemoryBarrierInfo& Info )
        {
            VKE_ASSERT2( m_state == States::BEGIN, "Command buffer must Begun" );
            m_BarrierInfo.vMemoryBarriers.PushBack( Info );
            m_needExecuteBarriers = true;
            VKE_LOG_CB();
            m_pMgr->_LogCommand( this, "Barrier" );
        }
        void CCommandBuffer::Barrier( const SBufferBarrierInfo& Info )
        {
            VKE_ASSERT2( m_state == States::BEGIN, "Command buffer must Begun" );
            m_BarrierInfo.vBufferBarriers.PushBack( Info );
            m_needExecuteBarriers = true;
            VKE_LOG_CB();
        }
        void CCommandBuffer::Barrier( const STextureBarrierInfo& Info )
        {
            VKE_ASSERT2( m_state == States::BEGIN, "Command buffer must Begun" );
            m_BarrierInfo.vTextureBarriers.PushBack( Info );
            m_needExecuteBarriers = true;
            VKE_LOG_CB();
        }

        void CCommandBuffer::SetState( TEXTURE_STATE state, TextureHandle* phTexInOut )
        {
            m_pBaseCtx->_SetTextureState( this, state, phTexInOut );
        }

        void CCommandBuffer::SetState(TEXTURE_STATE state, TexturePtr* ppOut)
        {
            STextureBarrierInfo Info;
            if( ( *ppOut )->SetState( state, &Info ) )
            {
                Barrier( Info );
            }
        }

        void CCommandBuffer::ExecuteBarriers()
        {
            VKE_ASSERT2( m_state == States::BEGIN, "" );
            m_pBaseCtx->m_pDeviceCtx->_NativeAPI().Barrier( this->GetDDIObject(), m_BarrierInfo );
            m_BarrierInfo.vBufferBarriers.Clear();
            m_BarrierInfo.vMemoryBarriers.Clear();
            m_BarrierInfo.vTextureBarriers.Clear();
            m_needExecuteBarriers = false;
            VKE_LOG_CB();
        }
        void CCommandBuffer::Bind( const RenderTargetHandle& hRT )
        {
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            RenderTargetPtr pRT = m_pBaseCtx->m_pDeviceCtx->GetRenderTarget( hRT );
            const auto& Desc = pRT->GetDesc();
            m_CurrentRenderPassDesc.vRenderTargets.PushBack( Desc );
            m_CurrentRenderPassDesc.Size = pRT->GetSize();
            m_needNewRenderPass = true;
#endif
        }
        void CCommandBuffer::_Reset()
        {
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            m_CurrentPipelineDesc.Pipeline.hRenderPass = INVALID_HANDLE;
            m_CurrentPipelineDesc.Pipeline.hDDIRenderPass = DDI_NULL_HANDLE;
            m_CurrentPipelineDesc.Pipeline.Viewport.vViewports.Clear();
            m_CurrentPipelineDesc.Pipeline.Viewport.vScissors.Clear();
            m_CurrentPipelineLayoutDesc.vDescriptorSetLayouts.Clear();
            m_pCurrentPipeline = nullptr;
            m_pCurrentPipelineLayout = nullptr;
            m_pCurrentRenderPass = nullptr;
            m_hCurrentdRenderPass = INVALID_HANDLE;
            m_hDDILastUsedLayout = DDI_NULL_HANDLE;
            m_CurrentRenderPassDesc.vRenderTargets.Clear();
            m_CurrentRenderPassDesc.vSubpasses.Clear();
#endif
            m_isPipelineBound = false;
            m_needExecuteBarriers = false;
            m_needNewPipeline = true;
            m_needNewPipelineLayout = true;
            m_needNewRenderPass = false;
            m_isRenderPassBound = false;
            m_isDirty = false;
            //m_hDDIFence = DDI_NULL_HANDLE;
            m_state = States::RESET;
            //m_pBaseCtx->_DestroyDescriptorSets( m_vUsedSets.GetData(), m_vUsedSets.GetCount() );
            m_pBaseCtx->GetDeviceContext()->_DestroyDescriptorSets( m_vUsedSets.GetData(), m_vUsedSets.GetCount() );
            m_vUsedSets.Clear();
            m_vDDIWaitOnSemaphores.Clear();
            VKE_LOG_CB();
        }
        void CCommandBuffer::SetState( PipelineLayoutPtr pLayout )
        {
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            m_pCurrentPipelineLayout = pLayout;
            m_CurrentPipelineDesc.Pipeline.hLayout = PipelineLayoutHandle{ m_pCurrentPipelineLayout->GetHandle() };
            m_CurrentPipelineDesc.Pipeline.hDDILayout = m_pCurrentPipelineLayout->GetDDIObject();
            // m_CurrentPipelineDesc.Pipeline.hRenderPass.handle =
            // reinterpret_cast< handle_t >( m_pCurrentRenderPass->GetDDIObject()
            // );
            VKE_ASSERT2( m_CurrentPipelineDesc.Pipeline.hLayout != INVALID_HANDLE, "Invalid pipeline object." );
            m_needNewPipeline = true;
            m_needNewPipelineLayout = false;
#endif
        }

        void CCommandBuffer::BeginRenderPass( const SBeginRenderPassInfo2& Info )
        {
            if( m_needExecuteBarriers )
            {
                ExecuteBarriers();
            }
            m_CurrViewport.Position = Info.RenderArea.Position;
            m_CurrViewport.Size = Info.RenderArea.Size;
            m_CurrScissor.Position = Info.RenderArea.Position;
            m_CurrScissor.Size = Info.RenderArea.Size;
            m_pBaseCtx->m_pDeviceCtx->NativeAPI().BeginRenderPass( GetDDIObject(), Info );
            m_isRenderPassBound = true;
        }

        void CCommandBuffer::BeginRenderPass(const TexturePtrArray& vColorRenderTargets,
            const TextureRefPtr pDepthStencilRenderTarget)
        {
            auto pDevice = m_pBaseCtx->GetDeviceContext();
            SBeginRenderPassInfo2 PassInfo;
            for( uint32_t i = 0; i < vColorRenderTargets.GetCount(); ++i )
            {
                const TexturePtr pTex = vColorRenderTargets[ i ];
                const TextureViewPtr pView = pDevice->GetTextureView( pTex->GetHandle() );
                PassInfo.vColorRenderTargetInfos.PushBack( SRenderTargetInfo
                {
                    .hDDIView = pView->GetDDIObject(),
                    .ClearColor = SClearValue( 0, 0, 0, 0 ),
                    .state = pTex->GetState(),
                    .renderPassOp = RenderTargetRenderPassOperations::COLOR_CLEAR_STORE
                } );
            }
            if( pDepthStencilRenderTarget.IsValid() )
            {
                const auto pView = pDevice->GetTextureView( pDepthStencilRenderTarget->GetHandle() );
                PassInfo.DepthRenderTargetInfo =
                {
                    .hDDIView = pView->GetDDIObject(),
                    .ClearColor = SClearValue(1, 0),
                    .state = pDepthStencilRenderTarget->GetState(),
                    .renderPassOp = RenderTargetRenderPassOperations::COLOR_CLEAR_STORE
                };
            }
            PassInfo.RenderArea =
            {
                .Position = ExtentI32(0,0),
                .Size = ExtentU32( vColorRenderTargets[0]->GetDesc().Size )
            };
            BeginRenderPass( PassInfo );
        }

        void CCommandBuffer::EndRenderPass()
        {
            m_pBaseCtx->m_pDeviceCtx->NativeAPI().EndRenderPass( GetDDIObject() );
            m_isRenderPassBound = false;
        }

        void CCommandBuffer::Bind( RenderPassPtr pRenderPass )
        {
            SBindRenderPassInfo Info;
            Info.hDDICommandBuffer = GetDDIObject();
            if( pRenderPass.IsValid() )
            {
                if( m_isRenderPassBound )
                {
                    // Unbind pipeline
                    m_isPipelineBound = false;
                    // If there is already render pass bound end it
                    m_pBaseCtx->m_pDeviceCtx->NativeAPI().UnbindRenderPass( GetDDIObject(),
                                                                      ( DDIRenderPass )( DDI_NULL_HANDLE ) );
                }
                if( m_needExecuteBarriers )
                {
                    ExecuteBarriers();
                }
                // Close current render pass
                if( m_pCurrentRenderPass != nullptr && !m_pCurrentRenderPass->IsActive() )
                {
                    Info.pBeginInfo = nullptr;
                    m_pBaseCtx->m_pDeviceCtx->NativeAPI().Bind( Info );
                    m_pCurrentRenderPass->_IsActive( false );
                }
                m_pCurrentRenderPass = pRenderPass;
                m_hCurrentdRenderPass = pRenderPass->GetHandle();
                m_hDDICurrentRenderPass = pRenderPass->GetDDIObject();
                m_pCurrentRenderPass->_IsActive( true );
                Info.pBeginInfo = &pRenderPass->GetBeginInfo();
                m_pBaseCtx->m_pDeviceCtx->NativeAPI().Bind( Info );
                m_isRenderPassBound = true;
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
                const auto hPass = RenderPassHandle{ m_pCurrentRenderPass->GetHandle() };
                m_CurrentPipelineDesc.Pipeline.hRenderPass = hPass;
                m_CurrentPipelineDesc.Pipeline.hDDIRenderPass = DDI_NULL_HANDLE;
                m_needNewPipeline = true; // m_CurrentPipelineDesc.Pipeline.hRenderPass
                                          // != hPass;
                VKE_ASSERT2( m_pCurrentRenderPass->GetHandle() == m_hCurrentdRenderPass.handle, "" );
                VKE_ASSERT2( m_pCurrentRenderPass->GetDDIObject() ==
                                m_pBaseCtx->m_pDeviceCtx->GetRenderPass( m_hCurrentdRenderPass )->GetDDIObject(),
                            "" );
#endif
            }
            else if( m_pCurrentRenderPass != nullptr )
            {
                m_pBaseCtx->m_pDeviceCtx->NativeAPI().UnbindRenderPass( GetDDIObject(),
                                                                  ( DDIRenderPass )( DDI_NULL_HANDLE ) );
                m_isRenderPassBound = false;
                m_pCurrentRenderPass = nullptr;
                m_hCurrentdRenderPass = INVALID_HANDLE;
            }
            VKE_LOG_CB();
        }
        void CCommandBuffer::Bind( PipelinePtr pPipeline )
        {
            if( pPipeline.IsValid() && pPipeline->IsResourceReady() )
            {
                SetState( pPipeline->GetLayout() );
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
                m_needNewPipeline = false;
#endif
                SBindPipelineInfo Info;
                Info.pCmdBuffer = this;
                Info.pPipeline = pPipeline.Get();
                m_isPipelineBound = true;
                m_pCurrentPipeline = pPipeline;
                m_pBaseCtx->m_pDeviceCtx->NativeAPI().Bind( Info );
                m_pBaseCtx->m_DDI.SetState( GetDDIObject(), m_CurrViewport );
                m_pBaseCtx->m_DDI.SetState( GetDDIObject(), m_CurrScissor );
                VKE_LOG_CB();
            }
        }
        void CCommandBuffer::SetState( ShaderPtr pShader )
        {
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            if( pShader.IsValid() )
            {
                const auto type = pShader->GetDesc().type;
                VKE_ASSERT2( type != ShaderTypes::_MAX_COUNT, "" );
                const ShaderHandle hShader( pShader->GetHandle() );
                {
                    m_CurrentPipelineDesc.Pipeline.Shaders.apShaders[ type ] = pShader;
                    m_needNewPipeline = true;
                }
            }
#endif
        }
        void CCommandBuffer::SetState( const ShaderHandle& hShader )
        {
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            ShaderPtr pShader = m_pBaseCtx->m_pDeviceCtx->GetShader( hShader );
            SetState( pShader );
#endif
        }
        /*void CCommandBuffer::SetState( const TEXTURE_STATE& newState,
        TexturePtr* ppInOut )
        {
            STextureBarrierInfo Info;
            if( VKE_SUCCEEDED( (*ppInOut)->SetState( newState, &Info ) ) )
            {
                Barrier( Info );
            }
        }*/
        void CCommandBuffer::Bind( VertexBufferPtr pBuffer, const uint32_t offset )
        {
            m_pBaseCtx->m_pDeviceCtx->NativeAPI().Bind( this->GetDDIObject(), pBuffer->GetDDIObject(), offset );
            VKE_LOG_CB();
        }
        void CCommandBuffer::Bind( const VertexBufferHandle& hBuffer, const uint32_t offset )
        {
            const auto& hDDIBuffer = m_pBaseCtx->m_pDeviceCtx->GetBuffer( hBuffer )->GetDDIObject();
            m_pBaseCtx->m_DDI.Bind( this->GetDDIObject(), hDDIBuffer, offset );
            VKE_LOG_CB();
        }
        void CCommandBuffer::Bind( const IndexBufferHandle& hBuffer, const uint32_t offset )
        {
            auto pBuffer = m_pBaseCtx->m_pDeviceCtx->GetBuffer( hBuffer );
            const auto& hDDIBuffer = pBuffer->GetDDIObject();
            INDEX_TYPE type = pBuffer->GetDesc().indexType;
            m_pBaseCtx->m_DDI.Bind( this->GetDDIObject(), hDDIBuffer, offset, type );
            VKE_LOG_CB();
        }
        void CCommandBuffer::Bind( CSwapChain* pSwapChain )
        {
            Bind( pSwapChain->m_DDISwapChain );
            SetState( pSwapChain->m_CurrViewport );
            SetState( pSwapChain->m_CurrScissor );
        }
        void CCommandBuffer::Bind( const SDDISwapChain& SwapChain )
        {
            if( m_isRenderPassBound )
            {
                Bind( RenderPassPtr{} );
            }
            if( m_needExecuteBarriers )
            {
                ExecuteBarriers();
            }
            SBindRenderPassInfo Info;
            SBeginRenderPassInfo BeginInfo;
            const auto idx = GetBackBufferIndex();
            BeginInfo.hDDIFramebuffer = SwapChain.vFramebuffers[ idx ];
            BeginInfo.hDDIRenderPass = SwapChain.hDDIRenderPass;
            BeginInfo.RenderArea.Size = SwapChain.Size;
            BeginInfo.RenderArea.Position = { 0, 0 };
            BeginInfo.vDDIClearValues.PushBack( { 0.5f, 0.5f, 0.5f, 1.0f } );
            Info.hDDICommandBuffer = GetDDIObject();
            Info.pBeginInfo = &BeginInfo;
            m_hDDICurrentRenderPass = SwapChain.hDDIRenderPass;
            m_pBaseCtx->m_pDeviceCtx->NativeAPI().Bind( Info );
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            m_needNewPipeline = m_CurrentPipelineDesc.Pipeline.hDDIRenderPass != SwapChain.hDDIRenderPass;
            m_CurrentPipelineDesc.Pipeline.hRenderPass = INVALID_HANDLE;
            m_CurrentPipelineDesc.Pipeline.hDDIRenderPass = SwapChain.hDDIRenderPass;
#endif
            m_isRenderPassBound = true;
            VKE_LOG_CB();
        }
        void CCommandBuffer::Bind( const DescriptorSetHandle& hSet, const uint32_t offset )
        {
            m_vBindings.PushBack( hSet );
            const DDIDescriptorSet& hDDISet = m_pBaseCtx->GetDeviceContext()->GetDescriptorSet( hSet );
            DescriptorSetLayoutHandle hLayout = m_pBaseCtx-> GetDeviceContext()-> GetDescriptorSetLayout( hSet );
            m_vDDIBindings.PushBack( hDDISet );
            m_vBindingOffsets.PushBack( offset );
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            m_CurrentPipelineLayoutDesc.vDescriptorSetLayouts.PushBack( hLayout );
            m_needNewPipelineLayout = true;
#endif
        }
        void CCommandBuffer::Bind( const uint32_t& index, const DescriptorSetHandle& hDescSet, const uint32_t* pOffsets,
                                   const uint16_t& offsetCount )
        {
            VKE_ASSERT2( m_pCurrentPipeline != nullptr, "Pipeline must be already bound to call this function." );
            VKE_ASSERT2( m_pCurrentPipeline->IsResourceReady(), "Pipeline must be compiled first." );
            SBindDDIDescriptorSetsInfo Info;
            const DDIDescriptorSet& hDDIDescSet = m_pBaseCtx-> GetDeviceContext()-> GetDescriptorSet( hDescSet );
            Info.aDDISetHandles = &hDDIDescSet;
            Info.aDynamicOffsets = pOffsets;
            Info.dynamicOffsetCount = offsetCount;
            Info.firstSet = ( uint16_t )index;
            Info.setCount = 1;
            Info.hDDICommandBuffer = GetDDIObject();
            Info.hDDIPipelineLayout = m_pCurrentPipeline->GetLayout()->GetDDIObject();
            Info.pipelineType = m_pCurrentPipeline->GetType();
            m_pBaseCtx->m_DDI.Bind( Info );
            VKE_LOG_CB();
        }
        void CCommandBuffer::Bind( const SBindDDIDescriptorSetsInfo& Info ) { m_pBaseCtx->m_DDI.Bind( Info ); }
        void CCommandBuffer::SetState( const SPipelineDesc::SDepthStencil& DepthStencil )
        {
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            m_CurrentPipelineDesc.Pipeline.DepthStencil = DepthStencil;
            m_needNewPipeline = true;
#endif
        }
        void CCommandBuffer::SetState( const SPipelineDesc::SRasterization& Rasterization )
        {
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            m_CurrentPipelineDesc.Pipeline.Rasterization = Rasterization;
            m_needNewPipeline = true;
#endif
        }
        void CCommandBuffer::SetState( const SPipelineDesc::SInputLayout& InputLayout )
        {
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            m_CurrentPipelineDesc.Pipeline.InputLayout = InputLayout;
            m_needNewPipeline = true;
#endif
        }
        void CCommandBuffer::SetState( const SViewportDesc& Viewport )
        {
            uint32_t h = Viewport.CalcHash();
            if( m_currViewportHash != h )
            {
                m_currViewportHash = h;
                // m_pBaseCtx->m_DDI.SetState( m_hDDIObject, Viewport );
                m_CurrViewport = Viewport;
                // m_CurrentPipelineDesc.Pipeline.Viewport.vViewports.PushBack(
                // Viewport );
            }
        }
        void CCommandBuffer::SetState( const SScissorDesc& Scissor )
        {
            uint32_t h = Scissor.CalcHash();
            if( m_currScissorHash != h )
            {
                m_currScissorHash = h;
                // m_pBaseCtx->m_DDI.SetState( m_hDDIObject, Scissor );
                m_CurrScissor = Scissor;
                // m_CurrentPipelineDesc.Pipeline.Viewport.vScissors[0] =
                // Scissor;
            }
        }
        void CCommandBuffer::SetState( const SViewportDesc& Viewport, bool )
        {
            uint32_t h = Viewport.CalcHash();
            if( m_currViewportHash != h )
            {
                m_currViewportHash = h;
                m_pBaseCtx->m_DDI.SetState( GetDDIObject(), Viewport );
                m_CurrViewport = Viewport;
            }
        }
        void CCommandBuffer::SetState( const SScissorDesc& Scissor, bool )
        {
            uint32_t h = Scissor.CalcHash();
            if( m_currScissorHash != h )
            {
                m_currScissorHash = h;
                m_pBaseCtx->m_DDI.SetState( GetDDIObject(), Scissor );
                m_CurrScissor = Scissor;
            }
        }
        void CCommandBuffer::SetState( const PRIMITIVE_TOPOLOGY& topology )
        {
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            m_CurrentPipelineDesc.Pipeline.InputLayout.topology = topology;
            m_needNewPipeline = true;
#endif
        }
        uint32_t ConvertFormatToSize( const FORMAT& format )
        {
            switch( format )
            {
                case Formats::R8_SINT:
                case Formats::R8_UINT:
                case Formats::R8_SNORM:
                case Formats::R8_UNORM: return sizeof( uint8_t ); break;
                case Formats::R8G8_SINT:
                case Formats::R8G8_UINT:
                case Formats::R8G8_SNORM:
                case Formats::R8G8_UNORM: return sizeof( uint8_t ) * 2; break;
                case Formats::R8G8B8_SINT:
                case Formats::R8G8B8_UINT:
                case Formats::R8G8B8_SNORM:
                case Formats::R8G8B8_UNORM: return sizeof( uint8_t ) * 3; break;
                case Formats::R8G8B8A8_SINT:
                case Formats::R8G8B8A8_UINT:
                case Formats::R8G8B8A8_SNORM:
                case Formats::R8G8B8A8_UNORM: return sizeof( uint8_t ) * 4; break;
                case Formats::R16_SINT:
                case Formats::R16_UINT:
                case Formats::R16_SNORM:
                case Formats::R16_UNORM: return sizeof( uint16_t ); break;
                case Formats::R16G16_SINT:
                case Formats::R16G16_UINT:
                case Formats::R16G16_SNORM:
                case Formats::R16G16_UNORM: return sizeof( uint16_t ) * 2; break;
                case Formats::R16G16B16_SINT:
                case Formats::R16G16B16_UINT:
                case Formats::R16G16B16_SNORM:
                case Formats::R16G16B16_UNORM: return sizeof( uint16_t ) * 3; break;
                case Formats::R16G16B16A16_SINT:
                case Formats::R16G16B16A16_UINT:
                case Formats::R16G16B16A16_SNORM:
                case Formats::R16G16B16A16_UNORM: return sizeof( uint16_t ) * 4; break;
                case Formats::R32_SINT:
                case Formats::R32_UINT:
                case Formats::R32_SFLOAT: return sizeof( float ); break;
                case Formats::R32G32_SINT:
                case Formats::R32G32_UINT:
                case Formats::R32G32_SFLOAT: return sizeof( float ) * 2; break;
                case Formats::R32G32B32_SINT:
                case Formats::R32G32B32_UINT:
                case Formats::R32G32B32_SFLOAT: return sizeof( float ) * 3; break;
                case Formats::R32G32B32A32_SINT:
                case Formats::R32G32B32A32_UINT:
                case Formats::R32G32B32A32_SFLOAT: return sizeof( float ) * 4; break;
                default: return 0; break;
            }
        }
        void CCommandBuffer::SetState( const SVertexInputLayoutDesc& VertexInputLayout )
        {
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            uint16_t currOffset = 0;
            uint16_t currLocation = 0;
            uint32_t vertexSize = 0;
            for( uint32_t i = 0; i < VertexInputLayout.vAttributes.GetCount(); ++i )
            {
                vertexSize += ConvertFormatToSize( static_cast<FORMAT>( VertexInputLayout.vAttributes[ i ].type ) );
            }
            m_CurrentPipelineDesc.Pipeline.InputLayout.vVertexAttributes.Clear();
            m_CurrentPipelineDesc.Pipeline.InputLayout.topology = VertexInputLayout.topology;
            m_CurrentPipelineDesc.Pipeline.InputLayout.enablePrimitiveRestart =
                VertexInputLayout.enablePrimitiveRestart;
            for( uint32_t i = 0; i < VertexInputLayout.vAttributes.GetCount(); ++i )
            {
                const auto& Curr = VertexInputLayout.vAttributes[ i ];
                SPipelineDesc::SInputLayout::SVertexAttribute VA;
                VA.vertexBufferBindingIndex = Curr.vertexBufferBinding;
                VA.offset = currOffset;
                VA.location = currLocation;
                VA.format = static_cast<FORMAT>( Curr.type );
                VA.pName = Curr.pName;
                VA.stride = static_cast<uint16_t>( vertexSize );
                currOffset += static_cast<uint16_t>(
                    ConvertFormatToSize( static_cast<FORMAT>( VertexInputLayout.vAttributes[ i ].type ) ) );
                ++currLocation;
                m_CurrentPipelineDesc.Pipeline.InputLayout.vVertexAttributes.PushBack( VA );
            }
            m_needNewPipeline = true;
#endif
        }
        Result CCommandBuffer::_DrawProlog()
        {
            Result ret = VKE_FAIL;
            if( m_needExecuteBarriers )
            {
                ExecuteBarriers();
            }
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            if( _UpdateCurrentRenderPass() == VKE_OK )
            {
                Bind( m_pCurrentRenderPass );
            }
#endif
            ret = _UpdateCurrentPipeline();
            VKE_ASSERT2( m_pCurrentPipeline.IsValid(), "Pipeline was not created successfully." );
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            if( !m_isPipelineBound )
            {
                Bind( m_pCurrentPipeline );
            }
#endif
            if( !m_vBindings.IsEmpty() )
            {
                if( VKE_SUCCEEDED( ret ) )
                {
                    _BindDescriptorSets();
                }
                m_vBindings.Clear();
                m_vDDIBindings.Clear();
                m_vBindingOffsets.Clear();
            }
            VKE_ASSERT2( m_isRenderPassBound == true, "Render pass must be bound before drawcall." );
            return ret;
        }
        void CCommandBuffer::_BindDescriptorSets()
        {
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            VKE_ASSERT2( m_pCurrentPipelineLayout->GetDDIObject() == m_pCurrentPipeline->GetDesc().hDDILayout, "" );
#endif
            SBindDDIDescriptorSetsInfo Info;
            Info.aDDISetHandles = m_vDDIBindings.GetData();
            Info.aDynamicOffsets = m_vBindingOffsets.GetData();
            Info.dynamicOffsetCount = static_cast<uint16_t>( m_vBindingOffsets.GetCount() );
            Info.firstSet = 0;
            Info.hDDICommandBuffer = GetDDIObject();
            Info.hDDIPipelineLayout = m_pCurrentPipeline->GetLayout()->GetDDIObject();
            Info.setCount = static_cast<uint16_t>( m_vDDIBindings.GetCount() );
            Info.pipelineType = m_pCurrentPipeline->GetType();
            m_pBaseCtx->m_DDI.Bind( Info );
            /*for( uint32_t i = 0; i < m_vBindings.GetCount(); ++i )
            {
                if( m_vUsedSets.Find( m_vBindings[i] ) < 0 )
                {
                    m_vUsedSets.PushBack( m_vBindings[i] );
                }
            }*/
        }
        void CCommandBuffer::DrawIndexedWithCheck( const SDrawParams& Params )
        {
            //VKE_PROFILE_SIMPLE();
            if( VKE_SUCCEEDED( _DrawProlog() ) )
            {
                //VKE_PROFILE_SIMPLE2( "Draw" );
                m_pBaseCtx->m_pDeviceCtx->NativeAPI().DrawIndexed( this->m_hDDIObject, Params );
            }
        }
        void CCommandBuffer::DrawWithCheck( const uint32_t& vertexCount, const uint32_t& instanceCount,
                                            const uint32_t& firstVertex, const uint32_t& firstInstance )
        {
            // if( m_needNewPipeline )
            {
                _DrawProlog();
            }
            VKE_ASSERT2( m_isPipelineBound, "Pipeline must be set." );
            // VKE_SIMPLE_PROFILE();
            m_pBaseCtx->m_DDI.Draw( GetDDIObject(), vertexCount, instanceCount, firstVertex, firstInstance );
        }
        void CCommandBuffer::DrawIndexedFast( const SDrawParams& Params )
        {
            VKE_ASSERT2( m_isPipelineBound, "Pipeline must be set." );
            //VKE_PROFILE_SIMPLE();
            m_pBaseCtx->m_pDeviceCtx->NativeAPI().DrawIndexed( this->m_hDDIObject, Params );
        }
        void CCommandBuffer::DrawFast( const uint32_t& vertexCount, const uint32_t& instanceCount,
                                       const uint32_t& firstVertex, const uint32_t& firstInstance )
        {
            VKE_ASSERT2( m_isPipelineBound, "Pipeline must be set." );
            m_pBaseCtx->m_DDI.Draw( GetDDIObject(), vertexCount, instanceCount, firstVertex, firstInstance );
        }
        void CCommandBuffer::DrawMesh(uint32_t width, uint32_t height, uint32_t depth)
        {
            m_pBaseCtx->m_DDI.DrawMesh( GetDDIObject(), width, height, depth );
        }
        void CCommandBuffer::Copy( const SCopyBufferInfo& Info )
        {
            if( m_needExecuteBarriers )
            {
                ExecuteBarriers();
            }
            m_pBaseCtx->m_DDI.Copy( m_hDDIObject, Info );
        }
        void CCommandBuffer::Copy( const SCopyTextureInfoEx& Info )
        {
            if( m_needExecuteBarriers )
            {
                ExecuteBarriers();
            }
            m_pBaseCtx->m_DDI.Copy( m_hDDIObject, Info );
        }
        void CCommandBuffer::Copy( const SCopyBufferToTextureInfo& Info )
        {
            if( m_needExecuteBarriers )
            {
                ExecuteBarriers();
            }
            m_pBaseCtx->m_DDI.Copy( m_hDDIObject, Info );
        }
        void CCommandBuffer::Blit( const SBlitTextureInfo& Info )
        {
            if( m_needExecuteBarriers )
            {
                ExecuteBarriers();
            }
            m_pBaseCtx->m_DDI.Blit( m_hDDIObject, Info );
        }

        void CCommandBuffer::GenerateMipmaps( TexturePtr pTex )
        {
            STextureFormatFeatures Features;
            m_pBaseCtx->GetDeviceContext()->GetFormatFeatures( pTex->GetDesc().format, &Features );
            if( Features.blitDst && Features.blitSrc && pTex->IsReady() )
            {
                //pCmdBuffer->Sync( pTex->GetCommandBuffer() );
                //pTex->SetCommandBuffer( pCmdBuffer );
                SBlitTextureInfo BlitInfo;
                BlitInfo.hAPISrcTexture = pTex->GetDDIObject();
                BlitInfo.hAPIDstTexture = pTex->GetDDIObject();
                BlitInfo.filter = TextureFilters::LINEAR;
                BlitInfo.srcTextureState = TextureStates::TRANSFER_SRC;
                BlitInfo.dstTextureState = TextureStates::TRANSFER_DST;
                BlitInfo.vRegions.Resize( 1 );
                uint16_t mipCount = pTex->GetDesc().mipmapCount;
                const auto BaseSize = pTex->GetDesc().Size;
                SBlitTextureRegion& Region = BlitInfo.vRegions[ 0 ];
                Region.SrcSubresource.aspect = pTex->GetAspect();
                Region.SrcSubresource.beginArrayLayer = 0;
                Region.SrcSubresource.layerCount = 1;
                Region.SrcSubresource.mipmapLevelCount = 1;
                Region.DstSubresource.aspect = pTex->GetAspect();
                Region.DstSubresource.beginArrayLayer = 0;
                Region.DstSubresource.layerCount = 1;
                Region.DstSubresource.mipmapLevelCount = 1;
                Region.srcOffsets[ 0 ] = { 0, 0, 0 };
                Region.dstOffsets[ 0 ] = { 0, 0, 0 };
                STextureBarrierInfo SrcBarrierInfo, DstBarrierInfo;
                pTex->SetState( TextureStates::TRANSFER_SRC, &SrcBarrierInfo );
                SrcBarrierInfo.SubresourceRange.mipmapLevelCount = 1;
                Barrier( SrcBarrierInfo );
                DstBarrierInfo = SrcBarrierInfo;
                DstBarrierInfo.SubresourceRange.mipmapLevelCount = 1;
                for( uint16_t currentMimapLevel = 0; currentMimapLevel < mipCount - 1; ++currentMimapLevel )
                {
                    uint16_t nextMipmapLevel = currentMimapLevel + 1;
                    Region.SrcSubresource.beginMipmapLevel = currentMimapLevel;
                    Region.DstSubresource.beginMipmapLevel = nextMipmapLevel;
                    Region.srcOffsets[ 1 ].x = ( int32_t )( BaseSize.width >> currentMimapLevel );
                    Region.srcOffsets[ 1 ].y = ( int32_t )( BaseSize.height >> currentMimapLevel );
                    Region.srcOffsets[ 1 ].z = 1;
                    Region.dstOffsets[ 1 ].x = ( int32_t )( BaseSize.width >> nextMipmapLevel );
                    Region.dstOffsets[ 1 ].y = ( int32_t )( BaseSize.height >> nextMipmapLevel );
                    Region.dstOffsets[ 1 ].z = 1;
 
                    DstBarrierInfo.currentState = TextureStates::UNDEFINED;
                    DstBarrierInfo.newState = TextureStates::TRANSFER_DST;
                    DstBarrierInfo.SubresourceRange.beginMipmapLevel = nextMipmapLevel;
                    Barrier( DstBarrierInfo );
                    Blit( BlitInfo );
                    DstBarrierInfo.currentState = TextureStates::TRANSFER_DST;
                    DstBarrierInfo.newState = TextureStates::TRANSFER_SRC;
                    DstBarrierInfo.SubresourceRange.beginMipmapLevel = nextMipmapLevel;
                    Barrier( DstBarrierInfo );
 
                }
                pTex->SetState( TextureStates::SHADER_READ, &SrcBarrierInfo );
                SrcBarrierInfo.currentState = TextureStates::UNDEFINED;
                Barrier( SrcBarrierInfo );
                SrcBarrierInfo.SubresourceRange.beginMipmapLevel = 0;
                SrcBarrierInfo.SubresourceRange.mipmapLevelCount = 1;
                // pCmdBuffer->Barrier( SrcBarrierInfo );
                AddDebugMarkerText(
                    std::format( "Generate mipmaps for: {}", pTex->GetDesc().Name.GetData() ) );
            }
            else
            {
                VKE_LOG_WARN( "Implement non-blit mipmap generation." );
            }
        }

        void CCommandBuffer::_FreeDescriptorSet( const DescriptorSetHandle& hSet ) { m_vUsedSets.PushBack( hSet ); }
        Result CCommandBuffer::_UpdateCurrentPipeline()
        {
            Result ret = VKE_OK;
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            if( m_needNewPipelineLayout )
            {
                m_pCurrentPipelineLayout =
                    m_pBaseCtx->m_pDeviceCtx->CreatePipelineLayout( m_CurrentPipelineLayoutDesc );
                if( m_pCurrentPipelineLayout.IsValid() )
                {
                    m_CurrentPipelineLayoutDesc.vDescriptorSetLayouts.Clear();
                    const DDIPipelineLayout& hDDILayout = m_pCurrentPipelineLayout->GetDDIObject();
                    // If pipeline layout didn't change do not to try to create
                    // new pipeline
                    if( hDDILayout != m_hDDILastUsedLayout )
                    {
                        // m_CurrentPipelineDesc.Pipeline.hDDILayout =
                        // m_pCurrentPipelineLayout->GetDDIObject();
                        // m_needNewPipeline = true;
                        SetState( m_pCurrentPipelineLayout );
                        m_hDDILastUsedLayout = hDDILayout;
                    }
                    m_needNewPipelineLayout = false;
                }
                else
                {
                    ret = VKE_FAIL;
                }
            }
            if( ( m_needNewPipeline && ret == VKE_OK ) || m_pCurrentPipeline.IsNull() )
            {
                // m_CurrentPipelineDesc.Pipeline.hDDILayout =
                // m_pCurrentPipelineLayout->GetDDIObject();
                VKE_ASSERT2( m_CurrentPipelineDesc.Pipeline.hDDILayout != DDI_NULL_HANDLE, "" );
                m_pCurrentPipeline = m_pBaseCtx->m_pDeviceCtx->CreatePipeline( m_CurrentPipelineDesc );
                if( m_pCurrentPipeline.IsNull() )
                {
                    ret = VKE_FAIL;
                }
                m_needNewPipeline = false;
                m_isPipelineBound = false;
            }
#endif
            if( !m_pCurrentPipeline->IsResourceReady() )
            {
                ret = VKE_FAIL;
            }
            return ret;
        }
        Result CCommandBuffer::_UpdateCurrentRenderPass()
        {
            Result ret = VKE_FAIL;
#if !VKE_ENABLE_SIMPLE_COMMAND_BUFFER
            if( m_needNewRenderPass )
            {
                auto hPass = m_pBaseCtx->m_pDeviceCtx->CreateRenderPass( m_CurrentRenderPassDesc );
                RenderPassPtr pPass = m_pBaseCtx->m_pDeviceCtx->GetRenderPass( hPass );
                m_needNewRenderPass = false;
                if( m_hCurrentdRenderPass != hPass )
                {
                    ret = VKE_OK; // render pass changed
                    m_hCurrentdRenderPass = hPass;
                    m_pCurrentRenderPass = pPass;
                }
            }
            else if( !m_isRenderPassBound )
            {
            }
#endif
            return ret;
        }
        void CCommandBuffer::SetEvent( const DDIEvent& hDDIEvent, const PIPELINE_STAGES& stages )
        {
            m_pBaseCtx->m_DDI.SetEvent( GetDDIObject(), hDDIEvent, stages );
        }
        void CCommandBuffer::ResetEvent( const DDIEvent& hDDIEvent, const PIPELINE_STAGES& stages )
        {
            m_pBaseCtx->m_DDI.Reset( GetDDIObject(), hDDIEvent, stages );
        }
        void CCommandBuffer::SetEvent( const EventHandle& hEvent, const PIPELINE_STAGES& stages )
        {
            auto hDDIEvent = m_pBaseCtx->m_pDeviceCtx->GetEvent( hEvent );
            SetEvent( hDDIEvent, stages );
        }
        void CCommandBuffer::ResetEvent( const EventHandle& hEvent, const PIPELINE_STAGES& stages )
        {
            auto hDDIEvent = m_pBaseCtx->m_pDeviceCtx->GetEvent( hEvent );
            ResetEvent( hDDIEvent, stages );
        }
        // Debug
        void CCommandBuffer::BeginDebugInfo( const SDebugInfo* pInfo )
        {
#if VKE_RENDER_SYSTEM_DEBUG
            m_pBaseCtx->m_DDI.BeginDebugInfo( m_hDDIObject, pInfo );
#endif
        }
        void CCommandBuffer::EndDebugInfo()
        {
#if VKE_RENDER_SYSTEM_DEBUG
            m_pBaseCtx->m_DDI.EndDebugInfo( m_hDDIObject );
#endif
        }
        handle_t CCommandBuffer::GetLastUsedStagingBufferAllocation() const
        {
            handle_t hRet;
            if( !m_vStagingBufferAllocations.IsEmpty() )
            {
                hRet = m_vStagingBufferAllocations.Back();
            }
            else
            {
                hRet = INVALID_HANDLE;
            }
            return hRet;
        }
        void CCommandBuffer::UpdateStagingBufferAllocation( const handle_t& hStagingBuffer )
        {
            if( m_vStagingBufferAllocations.Find(hStagingBuffer) == INVALID_POSITION )
            {
                m_vStagingBufferAllocations.PushBack( hStagingBuffer );
            }
        }
        void CCommandBuffer::_NotifyExecuted()
        {
            m_state = States::EXECUTED;
            // Notify resources awaiting for execution
            for (uint32_t i = 0; i < m_vpNotifyResources.GetCount(); ++i)
            {
                ( *m_vpNotifyResources[ i ] ) = true;
            }
            m_vpNotifyResources.Clear();
            _FreeResources();
        }
        void CCommandBuffer::_FreeResources()
        {
            for( uint32_t i = 0; i < m_vStagingBufferAllocations.GetCount(); ++i )
            {
                m_pBaseCtx->m_pDeviceCtx->m_pBufferMgr->GetStagingBufferManager()->FreeBuffer(
                    m_vStagingBufferAllocations[ i ] );
            }
            ClearStagingBufferAllocations();
        }

        void CCommandBuffer::Sync(CommandBufferPtr pCmdBuffer)
        {
            VKE_ASSERT( pCmdBuffer->m_pExecuteBatch != nullptr );
            VKE_ASSERT( m_pExecuteBatch != pCmdBuffer->m_pExecuteBatch );
            GetContext()->SyncExecute( pCmdBuffer );
        }

        void CCommandBuffer::SetDebugName( cstr_t pDbgName )
        {
#if VKE_RENDER_SYSTEM_DEBUG
            m_DbgName = pDbgName;
            m_pBaseCtx->_GetDDI().SetObjectDebugName( (uint64_t)m_hDDIObject, ApiObjectTypes::COMMAND_BUFFER, pDbgName );
#endif
        }

    } // namespace RenderSystem
} // namespace VKE
#endif // VKE_VULKAN_RENDER_SYSTEM