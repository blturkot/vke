

#include "../CSampleFramework.h"

struct SGfxContextListener : public VKE::RenderSystem::EventListeners::IGraphicsContext
{
    VKE::RenderSystem::VertexBufferRefPtr pVertexBuffer;
    VKE::RenderSystem::ShaderRefPtr pVertexShader;
    VKE::RenderSystem::ShaderRefPtr pPixelShader;
    //VKE::RenderSystem::SVertexInputLayoutDesc LayoutDescriptor;
    VKE::RenderSystem::PipelineRefPtr pPipeline;
    VKE::RenderSystem::CFrameGraphNode::STaskResult UploadResult;

    SGfxContextListener()
    {

    }

    virtual ~SGfxContextListener()
    {

    }

    void LoadShaders( VKE::RenderSystem::CDeviceContext* pCtx )
    {
        VKE::RenderSystem::SCreateShaderDesc VertexShaderDescriptor, PixelShaderDescriptor;

        VertexShaderDescriptor.Create.flags = VKE::Core::CreateResourceFlags::DEFAULT;
        VertexShaderDescriptor.Create.stages = VKE::Core::ResourceStages::FULL_LOAD;
        VertexShaderDescriptor.Shader.type = VKE::RenderSystem::ShaderTypes::VERTEX;
        VertexShaderDescriptor.Shader.FileInfo.FileName = "Data/Samples/Shaders/simple.hlsl";
        VertexShaderDescriptor.Shader.EntryPoint = "VertexShaderMain";
        VertexShaderDescriptor.Shader.Name = "VKE_SimpleVS";

        pVertexShader = pCtx->CreateShader( VertexShaderDescriptor );

        PixelShaderDescriptor.Create.flags = VKE::Core::CreateResourceFlags::DEFAULT;
        PixelShaderDescriptor.Create.stages = VKE::Core::ResourceStages::FULL_LOAD;
        PixelShaderDescriptor.Shader.type = VKE::RenderSystem::ShaderTypes::PIXEL;
        PixelShaderDescriptor.Shader.FileInfo.FileName = "Data/Samples/Shaders/simple.hlsl";
        PixelShaderDescriptor.Shader.EntryPoint = "PixelShaderMain";
        PixelShaderDescriptor.Shader.Name = "VKE_SimplePS";

        pPixelShader = pCtx->CreateShader( PixelShaderDescriptor );
    }

    //bool LoadBuffers( VKE::RenderSystem::CDeviceContext* pCtx )
    bool LoadBuffers( VKE::RenderSystem::CommandBufferPtr pCommandBuffer )
    {
        const float TriangleVertices[] = {
            0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f,
        };

        auto pContextBase = pCommandBuffer->GetContext();
        auto pCtx = pContextBase->GetDeviceContext();

        VKE::RenderSystem::SCreateBufferDesc VertexBufferDescriptor;
        VertexBufferDescriptor.Create.flags = VKE::Core::CreateResourceFlags::DEFAULT;
        VertexBufferDescriptor.Buffer.memoryUsage
            = VKE::RenderSystem::MemoryUsages::GPU_ACCESS | VKE::RenderSystem::MemoryUsages::BUFFER;
        VertexBufferDescriptor.Buffer.usage = VKE::RenderSystem::BufferUsages::VERTEX_BUFFER;
        VertexBufferDescriptor.Buffer.size = sizeof( TriangleVertices );
        VertexBufferDescriptor.Buffer.SetDebugName( "VKE_SimpleTriangle" );
        auto hVertexBuffer = pCtx->CreateBuffer( VertexBufferDescriptor );
        pVertexBuffer = pCtx->GetBuffer( hVertexBuffer );

        VKE::RenderSystem::SUpdateMemoryInfo UpdateMemoryInfo;
        UpdateMemoryInfo.dataSize = sizeof( TriangleVertices );
        UpdateMemoryInfo.pData = ( const void* )TriangleVertices;
        pContextBase->UpdateBuffer( pCommandBuffer, UpdateMemoryInfo, &hVertexBuffer );
        
        //pContextBase->Execute( 0 );
        
        return true;

    }

    bool LoadPipeline( VKE::RenderSystem::CDeviceContext* pCtx, VKE::RenderSystem::CFrameGraph* pFrameGraph )
    {
        /*
        VKE::RenderSystem::SPipelineCreateDesc PipelineTemplate;
        auto& PipelineDescriptor = PipelineTemplate.Pipeline;
        PipelineDescriptor.InputLayout.topology = VKE::RenderSystem::PrimitiveTopologies::TRIANGLE_LIST;
        PipelineDescriptor.InputLayout.vVertexAttributes
            = { { "POSITION", VKE::RenderSystem::Formats::R32G32B32_SFLOAT, 0u } };
        PipelineDescriptor.Shaders.apShaders[ VKE::RenderSystem::ShaderTypes::VERTEX ] = pVertexShader;
        PipelineDescriptor.Shaders.apShaders[ VKE::RenderSystem::ShaderTypes::PIXEL ] = pPixelShader;
        PipelineDescriptor.SetDebugName( "DebugView" );
        PipelineDescriptor.DepthStencil.Depth.write = false;
        PipelineDescriptor.DepthStencil.Depth.test = false;
        PipelineDescriptor.DepthStencil.Depth.compareFunc = VKE::RenderSystem::CompareFunctions::NEVER;
        PipelineDescriptor.Viewport.enable = true;
        PipelineDescriptor.Rasterization.Polygon.cullMode = VKE::RenderSystem::CullModes::NONE;
        VKE::RenderSystem::SCreateBindingDesc BindingDescriptor;
        // BindingDescriptor.AddStorageBuffer( 0, VKE::RenderSystem::PipelineStages::VERTEX, 1u );
        BindingDescriptor.LayoutDesc.SetDebugName( "VKE_Triangle_DebugView" );
        BindingDescriptor.SetDebugName( "VKE_Triangle_DebugView" );
        auto DescriptorSet = pCtx->CreateResourceBindings( BindingDescriptor );
        if( DescriptorSet == VKE::INVALID_HANDLE )
        {
            return false;
        }
        VKE::RenderSystem::SUpdateBindingsHelper UpdateBindingsHelper;
        // UpdateBindingsHelper.AddBinding( 0u, 0, pVertexBuffer->GetSize(), pVertexBuffer->GetHandle(),
        //                                  VKE::RenderSystem::BindingTypes::DYNAMIC_STORAGE_BUFFER );
        pCtx->UpdateDescriptorSet( UpdateBindingsHelper, &DescriptorSet );
        VKE::RenderSystem::SPipelineLayoutDesc LayoutDescriptor;
        LayoutDescriptor.vDescriptorSetLayouts = { pCtx->GetDescriptorSetLayout( DescriptorSet ) };
        auto pLayout = pCtx->CreatePipelineLayout( LayoutDescriptor );
        PipelineDescriptor.hLayout = pLayout->GetHandle();
        pPipeline = pCtx->CreatePipeline( PipelineTemplate );
        */

        auto pRenderPass = pFrameGraph->GetPass( "RenderFrame" );
        const auto& vColorFormaats = pRenderPass->GetColorRenderTargetFormats();
        const auto& depthFormat = pRenderPass->GetDepthRenderTargetFormat();

        //auto pDevice = pNode->GetContext()->GetDeviceContext();
        VKE::RenderSystem::SCreateBindingDesc BindingDesc;
        BindingDesc.SetDebugName( "TriangleBindings" );
        auto hBindings = pCtx->CreateResourceBindings( BindingDesc );
        auto hDescLayout = pCtx->GetDescriptorSetLayout( hBindings );
        VKE::RenderSystem::SPipelineLayoutDesc PipelineLayoutDesc;
        PipelineLayoutDesc.vDescriptorSetLayouts.PushBack( hDescLayout );
        auto hPipelineLayout = pCtx->CreatePipelineLayout( PipelineLayoutDesc );
        VKE::RenderSystem::SPipelineCreateDesc PipelineDesc;
        VKE::RenderSystem::SPipelineDesc& Pipeline = PipelineDesc.Pipeline;
        Pipeline.InputLayout.topology = VKE::RenderSystem::PrimitiveTopologies::TRIANGLE_LIST;
        Pipeline.InputLayout.vVertexAttributes
            = { { "POSITION", VKE::RenderSystem::Formats::R32G32B32_SFLOAT, 0u } };
        Pipeline.Rasterization.Polygon.cullMode = VKE::RenderSystem::CullModes::NONE;
        Pipeline.Shaders.apShaders[ VKE::RenderSystem::ShaderTypes::VERTEX ] = pVertexShader;
        Pipeline.Shaders.apShaders[ VKE::RenderSystem::ShaderTypes::PIXEL ] = pPixelShader;
        Pipeline.hLayout = hPipelineLayout->GetHandle();
        Pipeline.vColorRenderTargetFormats = vColorFormaats;
        Pipeline.depthRenderTargetFormat = depthFormat;
        Pipeline.SetDebugName( "TrianglePipeline" );
        pPipeline = pCtx->CreatePipeline( PipelineDesc );

        return true;
    }

    bool Init( VKE::RenderSystem::CDeviceContext* pCtx )
    {
        auto pFrameGraph = pCtx->GetRenderSystem()->GetFrameGraph();

        LoadShaders( pCtx );
        LoadPipeline( pCtx, pFrameGraph );

        auto pPass = pFrameGraph->GetPass( "UploadData" );
        pPass->AddTask(
            [ & ]( const VKE::RenderSystem::CFrameGraphNode* pNode, uint8_t backBufferIdx ) {
                auto pCmdBuffer = pPass->GetCommandBuffer( backBufferIdx );
                LoadBuffers( pCmdBuffer );
                return VKE::VKE_OK;
            },
            &UploadResult );

        auto pRenderFrame = pFrameGraph->CreatePass( { .pName = "RenderTriangle" } );

        pRenderFrame->SetWorkload( [ & ]( VKE::RenderSystem::CFrameGraphNode* const pPass, uint8_t backBufferIdx ) {
            if( UploadResult.executedOnGPU &&
                pPipeline.IsValid() && pPipeline->IsResourceReady() )
            {
                auto pCmdBuffer = pPass->GetCommandBuffer( backBufferIdx );
                
                pCmdBuffer->Bind( pPipeline );
                //const auto hVB = HandleCast<VKE::RenderSystem::VertexBufferHandle>( pVertexBuffer->GetHandle() );
                pCmdBuffer->Bind( pVertexBuffer );
                pCmdBuffer->Draw( 3 );
            }
            return VKE::VKE_OK;
        } );

        auto pRenderFramePass = pFrameGraph->GetPass( "RenderFrame" );
        pRenderFramePass->AddSubpass( pRenderFrame );
        pFrameGraph->Build();

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
        Desc.featureLevel = VKE::RenderSystem::FeatureLevels::LEVEL_1_2;

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