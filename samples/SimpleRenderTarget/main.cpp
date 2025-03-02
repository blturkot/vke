

#include "../CSampleFramework.h"

struct SGfxContextListener : public VKE::RenderSystem::EventListeners::IGraphicsContext
{
    VKE::RenderSystem::VertexBufferRefPtr pVb;
    VKE::RenderSystem::ShaderRefPtr pVS;
    VKE::RenderSystem::ShaderRefPtr pPS;
    VKE::RenderSystem::SVertexInputLayoutDesc Layout;
    VKE::Core::STaskResult VsResult, PsResult;
    VKE::RenderSystem::PipelinePtr pPipeline;
    VKE::RenderSystem::TextureHandle hRenderTargetTex;
    VKE::RenderSystem::TextureViewHandle hRenderTargetView;
    VKE::RenderSystem::RenderTargetHandle hRenderTarget;

    VKE::RenderSystem::VertexBufferRefPtr pRtVb;
    VKE::RenderSystem::ShaderRefPtr pRtVS;
    VKE::RenderSystem::ShaderRefPtr pRtPS;
    VKE::RenderSystem::SVertexInputLayoutDesc RtLayout;

    VKE::RenderSystem::DescriptorSetHandle  hDescSet;
    VKE::RenderSystem::SamplerHandle hSampler;

    SFpsCounter m_Fps;

    SGfxContextListener()
    {

    }

    virtual ~SGfxContextListener()
    {

    }

    void LoadShaders( VKE::RenderSystem::CDeviceContext* pCtx )
    {
        VKE::RenderSystem::SCreateShaderDesc VsDesc, PsDesc;

        VsDesc.Create.flags = Core::CreateResourceFlags::DEFAULT;
        VsDesc.Create.stages = VKE::Core::ResourceStages::FULL_LOAD;
        VsDesc.Create.pOutput = &pVS;
        VsDesc.Shader.FileInfo.pFileName = "Data/Samples/Shaders/simple.vs";
        pCtx->CreateShader( VsDesc );

        PsDesc = VsDesc;
        PsDesc.Create.pOutput = &pPS;
        PsDesc.Shader.FileInfo.pFileName = "Data/Samples/shaders/simple.ps";
        pCtx->CreateShader( PsDesc );

        VsDesc.Create.flags = Core::CreateResourceFlags::DEFAULT;
        VsDesc.Create.stages = VKE::Core::ResourceStages::FULL_LOAD;
        VsDesc.Create.pOutput = &pRtVS;
        VsDesc.Shader.FileInfo.pFileName = "Data/Samples/Shaders/simple-fullscreen-quad.vs";
        pCtx->CreateShader( VsDesc );

        PsDesc = VsDesc;
        PsDesc.Create.pOutput = &pRtPS;
        PsDesc.Shader.FileInfo.pFileName = "Data/Samples/shaders/simple-copy.ps";
        pCtx->CreateShader( PsDesc );
    }

    bool Init( VKE::RenderSystem::CDeviceContext* pCtx )
    {
        LoadShaders( pCtx );

        VKE::RenderSystem::SCreateBufferDesc BuffDesc;
        BuffDesc.Create.flags = Core::CreateResourceFlags::DEFAULT;
        BuffDesc.Buffer.usage = VKE::RenderSystem::BufferUsages::VERTEX_BUFFER;
        BuffDesc.Buffer.memoryUsage = VKE::RenderSystem::MemoryUsages::GPU_ACCESS;
        BuffDesc.Buffer.size = ( sizeof( float ) * 4 ) * 3;
        auto hVb = pCtx->CreateBuffer( BuffDesc );
        pVb = pCtx->GetBuffer( hVb );
        const float vb[4 * 3] =
        {
            0.0f,  -0.5f,   0.0f,   1.0f,
            -0.5f, 0.5f,   0.0f,   1.0f,
            0.5f,  0.5f,   0.0f,   1.0f
        };
        VKE::RenderSystem::SUpdateMemoryInfo Info;
        Info.pData = vb;
        Info.dataSize = sizeof( vb );
        Info.dstDataOffset = 0;
        pCtx->UpdateBuffer( Info, &pVb );

        BuffDesc.Buffer.size = ( sizeof( float ) * 6 ) * 6;
        auto hRtVb = pCtx->CreateBuffer( BuffDesc );
        pRtVb = pCtx->GetBuffer( hRtVb );
        const float vb2[6*6] =
        {
            -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f
        };
        Info.pData = vb2;
        Info.dataSize = sizeof( vb2 );
        pCtx->UpdateBuffer( Info, &pRtVb );

        Layout.vAttributes.PushBack( VKE::RenderSystem::SVertexAttributeDesc{ "Position", VKE::RenderSystem::VertexAttributeTypes::POSITION3, 0 } );

        RtLayout.vAttributes =
        {
            { "Position", VKE::RenderSystem::VertexAttributeTypes::POSITION3, 0 },
            { "Texcoord0", VKE::RenderSystem::VertexAttributeTypes::TEXCOORD, 0 }
        };



        VKE::RenderSystem::SRenderTargetDesc RtDesc;
        RtDesc.beginState = VKE::RenderSystem::TextureStates::COLOR_RENDER_TARGET;
        RtDesc.endState = VKE::RenderSystem::TextureStates::COLOR_RENDER_TARGET;
        RtDesc.renderPassUsage = VKE::RenderSystem::RenderPassAttachmentUsages::COLOR_CLEAR_STORE;
        RtDesc.format = VKE::RenderSystem::Formats::R8G8B8A8_UNORM;
        RtDesc.memoryUsage = VKE::RenderSystem::MemoryUsages::GPU_ACCESS;
        RtDesc.mipLevelCount = 1;
        RtDesc.multisampling = VKE::RenderSystem::SampleCounts::SAMPLE_1;
        RtDesc.pDebugName = "Sample RenderTarget";
        RtDesc.Size = { 800, 600 };
        RtDesc.type = VKE::RenderSystem::TextureTypes::TEXTURE_2D;
        RtDesc.usage = VKE::RenderSystem::TextureUsages::COLOR_RENDER_TARGET | VKE::RenderSystem::TextureUsages::SAMPLED;
        hRenderTarget = pCtx->CreateRenderTarget( RtDesc );

        pCtx->SetTextureState( RtDesc.beginState, &hRenderTarget );

        VKE::RenderSystem::SSamplerDesc SampDesc;
        SampDesc.borderColor = VKE::RenderSystem::BorderColors::FLOAT_OPAQUE_BLACK;
        SampDesc.maxAnisotropy = 1;
        SampDesc.compareFunc = VKE::RenderSystem::CompareFunctions::NEVER;
        hSampler = pCtx->CreateSampler( SampDesc );

        VKE::RenderSystem::SCreateBindingDesc BindingDesc;
        BindingDesc.AddTextures( 0, VKE::RenderSystem::PipelineStages::PIXEL );
        BindingDesc.AddSamplers( 1, VKE::RenderSystem::PipelineStages::PIXEL );

        hDescSet = pCtx->CreateResourceBindings( BindingDesc );
        VKE::RenderSystem::SUpdateBindingsHelper UpdateInfo;
        UpdateInfo.AddBinding( 0, &hRenderTarget, 1 );
        UpdateInfo.AddBinding( 1, &hSampler, 1 );
        pCtx->UpdateDescriptorSet( UpdateInfo, &hDescSet );
        //pCtx->UpdateDescriptorSet( hSampler, hRenderTarget, &hDescSet );


        return pVb.IsValid();
    }

    bool OnRenderFrame(VKE::RenderSystem::CGraphicsContext* pCtx) override
    {
        char buff[128];
        vke_sprintf( buff, 128, "Simple RenderTarget - %d fps", m_Fps.GetFps() );
        pCtx->GetSwapChain()->GetWindow()->SetText( buff );

        auto pCmdbuffer = pCtx->BeginFrame();
        pCmdbuffer->SetState( Layout );
        pCmdbuffer->SetState( VKE::RenderSystem::PrimitiveTopologies::TRIANGLE_LIST );
        pCmdbuffer->Bind( pVb );
        pCmdbuffer->SetState( pVS );
        pCmdbuffer->SetState( pPS );
        pCmdbuffer->Bind( hRenderTarget );

        for( uint32_t i = 0; i < 1; ++i )
        {
            pCmdbuffer->Draw( 3 );
        }
        pCtx->SetTextureState( VKE::RenderSystem::TextureStates::SHADER_READ, &hRenderTarget );
        pCmdbuffer->Bind( pCtx->GetSwapChain() );
        pCmdbuffer->Bind( pRtVb );
        pCmdbuffer->SetState( RtLayout );
        pCmdbuffer->SetState( pRtVS );
        pCmdbuffer->SetState( pRtPS );
        pCmdbuffer->SetState( VKE::RenderSystem::PrimitiveTopologies::TRIANGLE_STRIP );
        pCmdbuffer->Bind( hDescSet, 0 );
        pCmdbuffer->Draw( 4 );
        pCtx->SetTextureState( VKE::RenderSystem::TextureStates::COLOR_RENDER_TARGET, &hRenderTarget );
        pCtx->EndFrame();
        return true;
    }
};

int main()
{
    VKE_DETECT_MEMORY_LEAKS();
    //VKE::Platform::Debug::BreakAtAllocation( 3307 );
    {
        CSampleFramework Sample;
        SSampleCreateDesc Desc;
        VKE::RenderSystem::EventListeners::IGraphicsContext* apListeners[1] =
        {
            VKE_NEW SGfxContextListener()
        };
        Desc.ppGfxListeners = apListeners;
        Desc.gfxListenerCount = 1;

        if( Sample.Create( Desc ) )
        {
            SGfxContextListener* pListener = reinterpret_cast<SGfxContextListener*>(apListeners[0]);
            if( pListener->Init( Sample.m_vpDeviceContexts[0] ) )
            {
                Sample.Start();
            }
        }
        Sample.Destroy();
    }

    return 0;
}