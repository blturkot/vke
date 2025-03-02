#include "RenderSystem/Vulkan/Managers/CShaderManager.h"
#if VKE_VULKAN_RENDER_SYSTEM
#include "Core/VKEConfig.h"
#include "RenderSystem/Vulkan/Resources/CShader.h"
#include "RenderSystem/CDeviceContext.h"
#include "RenderSystem/CRenderSystem.h"
#include "CVkEngine.h"
#include "Core/Threads/CThreadPool.h"
#include "Core/Managers/CFileManager.h"
#include "Core/Math/Math.h"

namespace VKE
{
    namespace RenderSystem
    {
        class CShaderTextProcessor
        {
            using StringVec = Utils::TCDynamicArray< vke_string, 128 >;
            using StringMap = vke_hash_map< vke_string, vke_string >;

            Core::CFileManager* m_pFileMgr;
            StringMap           m_mIncludes; // processed includes: key="name.ext" value=name.ext file text

            Result _PreprocessIncludes( cstr_t pBaseDirPath,
                cstr_t pCode, vke_string& strLine, vke_string* pStrOutput )
            {
                std::smatch Match;

                static const std::regex Regex( "^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*" );
                char pFullFilePath[1024] = { 0 };
                std::stringstream ssCode( pCode );

                while( std::getline( ssCode, strLine ) )
                {
                    if( std::regex_search( strLine, Match, Regex ) )
                    {
                        const std::string& strMatch = Match[1];
                        Core::SLoadFileInfo Desc;

                        uint32_t fileNameLen = vke_sprintf( pFullFilePath, sizeof( pFullFilePath ), "%s\\%s", pBaseDirPath, strMatch.c_str() );
                        ( void )fileNameLen;
                        Desc.FileInfo.FileName = pFullFilePath;
                        /*Desc.FileInfo.fileNameLen = static_cast< uint16_t >( fileNameLen );
                        Desc.FileInfo.pName = strMatch.c_str();
                        Desc.FileInfo.nameLen = static_cast< uint16_t >( strMatch.length() );*/
                        Core::FilePtr pFile = m_pFileMgr->LoadFile( Desc );
                        if( pFile.IsValid() )
                        {
                            cstr_t pTmpCode = reinterpret_cast<cstr_t>(pFile->GetData());
                            _PreprocessIncludes( pBaseDirPath, pTmpCode, strLine, pStrOutput );
                        }
                        else
                        {
                            VKE_LOG_ERR( "Unable to include file: " << strMatch.c_str() );
                        }
                    }
                    else
                    {
                        pStrOutput->append( strLine );
                    }
                }
                pStrOutput->append( "\n" );
                return VKE_OK;
            }
        };

        CShaderTextProcessor g_ShadeTextProcessor;

        TaskState ShaderManagerTasks::SCreateShadersTask::_OnStart(uint32_t /*tid*/)
        {
            TaskState state = TaskStateBits::FAIL;

            return state;
        }

        //TaskState ShaderManagerTasks::SCreateProgramTask::_OnStart(uint32_t /*tid*/)
        //{
        //    TaskState state = TaskStateBits::FAIL;

        //    return state;
        //}

        //void ShaderManagerTasks::SCreateProgramTask::_OnGet( void** /*ppOut*/ )
        //{

        //}

        struct SShaderTaskGroups
        {
            struct SCreateGroup
            {
                SShadersCreateDesc          Desc;
                CShaderManager::ShaderVec   vpShaders;
                CShaderManager*             pMgr = nullptr;
                uint32_t                    taskFinishedCount = 0;

                struct SCreateTask : public Threads::ITask
                {
                    SCreateGroup*   pGroup;
                    ExtentU16       DescRange;
                    bool            block = true;

                    TaskState _OnStart( uint32_t tid ) override
                    {
                        TaskState state = TaskStateBits::NOT_ACTIVE;
                        auto& vDescs = pGroup->Desc.vCreateDescs;
                        CShaderManager::ShaderVec& vpTmpShaders = pGroup->vpShaders;
                        if( block )
                        {
                            for( uint32_t i = DescRange.begin; i < DescRange.end; ++i )
                            {
                                SCreateShaderDesc& TmpDesc = vDescs[ i ];
                                TmpDesc.Create.flags = Core::CreateResourceFlags::DEFAULT;
                                ShaderPtr pShader = pGroup->pMgr->CreateShader( TmpDesc );
                                vpTmpShaders[ i ] = pShader;
                                //state = TaskStateBits::OK;
                            }
                            pGroup->taskFinishedCount += ( DescRange.end - DescRange.begin );
                            if( pGroup->taskFinishedCount == pGroup->Desc.vCreateDescs.GetCount() )
                            {
                                pGroup->Desc.pfnCallback( &tid, &vpTmpShaders );
                            }
                        }
                        return state;
                    }
                };

                using TaskVec = Utils::TCDynamicArray< SCreateTask, 16 >;
                TaskVec vTasks;

                Result Init()
                {
                    return VKE_OK;
                }

                void FullClear()
                {
                    Desc.vCreateDescs.ClearFull();
                    vpShaders.ClearFull();
                    vTasks.ClearFull();
                }

                void Clear()
                {
                    Desc.vCreateDescs.Clear();
                    vpShaders.Clear();
                    vTasks.Clear();
                }
            };

            template<class GroupType>
            Result InitGroup(CShaderManager* pMgr, GroupType* pInOut)
            {
                GroupType& Group = *pInOut;
                Group.pMgr = pMgr;
                const uint32_t res = Group.vTasks.Resize( Platform::Thread::GetMaxConcurrentThreadCount() - 1 );
                if( res != INVALID_POSITION )
                {
                    for( uint32_t i = 0; i < Group.vTasks.GetCount(); ++i )
                    {
                        Group.vTasks[ i ].pGroup = &Group;
                    }
                }
                else
                {
                    VKE_LOG_ERR( "Unable to create memory for SCreateTask objects." );
                    return VKE_ENOMEMORY;
                }

                return Group.Init();
            }

            template<class GroupType>
            using TaskBuffer = Utils::TSFreePool< GroupType, uint32_t >;

            TaskBuffer< SCreateGroup > CreateTaskBuffer;

            template<class TasksBufferType, class TaskGroupType>
            Result CreateTaskGroup(CShaderManager* pMgr, TasksBufferType* pInOut, TaskGroupType** ppOut)
            {
                /*if( !pInOut->vFreeElements.PopBack( &(*ppOut) ) )
                {
                    TaskGroupType Group;
                    uint32_t idx = pInOut->vPool.PushBack( Group );
                    if( idx != Utils::INVALID_POSITION )
                    {
                        TaskGroupType* pGroup = &pInOut->vPool[ idx ];
                        InitGroup( pMgr, pGroup );
                        pInOut->vFreeElements.PushBack( pGroup );
                        return CreateTaskGroup( pMgr, pInOut, ppOut );
                    }
                    else
                    {
                        VKE_LOG_ERR( "Unable to create memory for TaskGroup." );
                        return VKE_ENOMEMORY;
                    }
                }*/
                Result ret = VKE_FAIL;
                uint32_t idx;
                if( !pInOut->vFreeElements.PopBack( &idx ) )
                {
                    TaskGroupType Group;
                    idx = pInOut->vPool.PushBack( Group );
                }
                TaskGroupType* pGroup = &pInOut->vPool[idx];
                InitGroup( pMgr, pGroup );
                //ret = CreateTaskGroup( pMgr, pInOut, ppOut );
                return ret;
            }

            template<class TasksBufferType, class TaskGroupType>
            void FreeTaskGroup(TasksBufferType* pInOut, TaskGroupType* pIn)
            {
                pInOut->vFreeElements.PushBack( pIn );
            }
        };

        CShaderManager::CShaderManager(CDeviceContext* pCtx, Core::CFileManager* pFileMgr) :
            m_pCtx{ pCtx },
            m_pFileMgr{ pFileMgr }
        {

        }

        CShaderManager::~CShaderManager()
        {}

        void CShaderManager::Destroy()
        {
            if( m_pCompiler )
            {
                for( uint32_t i = 0; i < ShaderTypes::_MAX_COUNT; ++i )
                {
                    m_apDefaultShaders[ i ] = nullptr;
                    ShaderBuffer& Buffer = m_aShaderBuffers[ i ];
                    auto& Allocator = m_aShaderFreeListPools[ i ];

                    for(auto& Itr : Buffer.Resources.Container )
                    {
                        CShader* pShader = Itr.second;
                        _DestroyShader( &Allocator, &pShader );
                    }
                    Buffer.Clear();
                    Allocator.Destroy();
                }

                Memory::DestroyObject( &HeapAllocator, &m_pShaderTaskGroups );
                m_pCompiler->Destroy();
                Memory::DestroyObject( &HeapAllocator, &m_pCompiler );

                /*for( uint32_t i = 0; i < ShaderTypes::_MAX_COUNT; ++i )
                {
                    m_aShaderFreeListPools[ i ].Destroy();
                }
                m_ShaderProgramFreeListPool.Destroy();*/
            }
        }

        void CShaderManager::_DestroyShader( Memory::CFreeListPool* pAllocator, CShader** ppInOut )
        {
            CShader* pShader = *ppInOut;
            m_pCtx->NativeAPI().DestroyShader( &pShader->m_hDDIObject, nullptr );
            Memory::DestroyObject( pAllocator, &pShader );
        }

        Result CShaderManager::Create(const SShaderManagerDesc& Desc)
        {
            Result res = VKE_FAIL;
            m_Desc = Desc;
            res = VKE_OK;
            res = Memory::CreateObject( &HeapAllocator, &m_pCompiler, this );
            if( VKE_SUCCEEDED( res ) )
            {
                SShaderCompilerDesc CompilerDesc;
                res = m_pCompiler->Create( CompilerDesc );
                if( VKE_SUCCEEDED( res ) )
                {
                    for (uint32_t i = 0; i < ShaderTypes::_MAX_COUNT; ++i)
                    {
                        m_Desc.aMaxShaderCounts[ i ]
                            = Max( m_Desc.aMaxShaderCounts[ i ],
                                   Config::RenderSystem::Shader::MAX_SHADER_COUNT_PER_TYPE );

                    }
                                        
                    m_Desc.maxShaderProgramCount = Max( m_Desc.maxShaderProgramCount, Config::RenderSystem::Shader::MAX_SHADER_COUNT_PER_TYPE );

                    const uint32_t shaderSize = sizeof( CShader );
                    bool success = true;
                    for( uint32_t i = 0; i < ShaderTypes::_MAX_COUNT; ++i )
                    {
                        res = m_aShaderFreeListPools[ i ].Create( m_Desc.aMaxShaderCounts[ i ], shaderSize, 1 );
                        if( VKE_FAILED( res ) )
                        {
                            success = false;
                        }
                    }
                    if( success )
                    {
                        const uint32_t programSize = sizeof( CShaderProgram );
                        res = m_ShaderProgramFreeListPool.Create( m_Desc.maxShaderProgramCount, programSize, 1 );
                        if( VKE_SUCCEEDED( res ) )
                        {
                            res = Memory::CreateObject( &HeapAllocator, &m_pShaderTaskGroups );
                            if( VKE_SUCCEEDED( res ) )
                            {

                            }
                        }
                    }
                }
                if( VKE_SUCCEEDED( res ) )
                {
                    // Default init
                    SShaderManagerInitDesc MgrDesc;
                    res = Init( MgrDesc );
                    if( VKE_SUCCEEDED( res ) )
                    {
                        res = _CreateDefaultShaders();
                        if( VKE_FAILED( res ) )
                        {
                            Destroy();
                        }
                    }
                }
            }
            return res;
        }

        Result CShaderManager::Init(const SShaderManagerInitDesc& Desc)
        {
            Result ret = VKE_OK;
            m_InitDesc = Desc;
            // There could be user defined shader file extensions
            // In case there aren't prepare default ones
            {
                auto& vExts = m_InitDesc.avShaderExtensions[ ShaderTypes::VERTEX ];
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "vs.hlsl" );
                }
            }
            {
                auto& vExts = m_InitDesc.avShaderExtensions[ ShaderTypes::COMPUTE ];
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "cs.hlsl" );
                }
            }
            {
                auto& vExts = m_InitDesc.avShaderExtensions[ ShaderTypes::GEOMETRY ];
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "gs.hlsl" );
                }
            }
            {
                auto& vExts = m_InitDesc.avShaderExtensions[ ShaderTypes::PIXEL ];
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "ps.hlsl" );
                }
            }
            {
                auto& vExts = m_InitDesc.avShaderExtensions[ ShaderTypes::DOMAIN ];
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "ds.hlsl" );
                }
            }
            {
                auto& vExts = m_InitDesc.avShaderExtensions[ ShaderTypes::HULL ];
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "hs.hlsl" );
                }
            }
            {
                auto& vExts = m_InitDesc.avShaderExtensions[ ShaderTypes::TASK ];
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "ts.hlsl" );
                }
            }
            {
                auto& vExts = m_InitDesc.avShaderExtensions[ ShaderTypes::MESH ];
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "ms.hlsl" );
                }
            }
            {
                auto& vExts = m_InitDesc.avShaderExtensions[ ShaderTypes::RAYGEN ];
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "rt.hlsl" );
                }
            }
            {
                auto& vExts = m_InitDesc.avShaderExtensions[ ShaderTypes::ANY_HIT ];
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "rt.hlsl" );
                }
            }
            {
                auto& vExts = m_InitDesc.avShaderExtensions[ ShaderTypes::CLOSEST_HIT ];
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "rt.hlsl" );
                }
            }
            {
                auto& vExts = m_InitDesc.avShaderExtensions[ ShaderTypes::MISS ];
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "rt.hlsl" );
                }
            }
            {
                auto& vExts = m_InitDesc.avShaderExtensions[ ShaderTypes::INTERSECTION ];
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "rt.hlsl" );
                }
            }
            {
                auto& vExts = m_InitDesc.avShaderExtensions[ ShaderTypes::CALLABLE ];
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "rt.hlsl" );
                }
            }

            {
                auto& vExts = m_InitDesc.vProgramExtensions;
                if( vExts.IsEmpty() )
                {
                    vExts.PushBack( "shader" );
                    vExts.PushBack( "program" );
                }
            }
            return ret;
        }

        void CShaderManager::FreeUnusedResources()
        {
            for( uint32_t i = 0; i < ShaderTypes::_MAX_COUNT; ++i )
            {
            }
        }

        static const EShLanguage g_aLanguages[ EShLangCount ] =
        {
            EShLangVertex,
            EShLangTessControl,
            EShLangTessEvaluation,
            EShLangGeometry,
            EShLangFragment,
            EShLangCompute
        };

        ShaderRefPtr CShaderManager::CreateShader(const SCreateShaderDesc& Desc)
        {
            ShaderRefPtr pRet;
            // Try to find shader type if none is set
            SHADER_TYPE shaderType = Desc.Shader.type;
            if( shaderType == ShaderTypes::_MAX_COUNT )
            {
                if( Desc.Shader.pData && Desc.Shader.pData->type != ShaderTypes::_MAX_COUNT )
                {
                    shaderType = Desc.Shader.pData->type;
                }
                else if( !Desc.Shader.FileInfo.FileName.IsEmpty() )
                {
                    shaderType = FindShaderType( Desc.Shader.FileInfo.FileName.GetData() );
                }
            }
            VKE_ASSERT2( shaderType != ShaderTypes::_MAX_COUNT && shaderType >= 0, "Shader type must be a enum type." );
            if( shaderType >= ShaderTypes::_MAX_COUNT )
            {
                VKE_LOG_ERR( "Invalid shader type:" << shaderType );
                goto ERR;
            }
            {
                CShader* pShader = nullptr;
                hash_t hash = CShader::CalcHash( Desc.Shader );
                bool reuseShader = false;
                CShader::SHandle Handle;
                Handle.value = hash;
                Threads::SyncObject& SyncObj = m_aShaderTypeSyncObjects[ shaderType ];
                {
                    Threads::ScopedLock l( SyncObj );
                    ShaderBuffer& Buffer = m_aShaderBuffers[ shaderType ];
                    // Try to get this object if already created
                    reuseShader = Buffer.Find( Handle.hash, &pShader );
                    if( !reuseShader )
                    {
                        if( !Buffer.Reuse( Handle.hash, Handle.hash, &pShader ) )
                        {
                            auto& Allocator = m_aShaderFreeListPools[ shaderType ];
                            if( VKE_SUCCEEDED( Memory::CreateObject( &Allocator, &pShader, this, shaderType ) ) )
                            {
                                if( Buffer.Add( Handle.hash, pShader ) )
                                {
                                    pShader->Init( Desc.Shader, hash );
                                    pShader->m_Desc.type = shaderType;
                                    pShader->m_resourceStages = Desc.Create.stages;
                                    pRet = ShaderRefPtr{ pShader };
                                }
                                else
                                {
                                    VKE_LOG_ERR( "Unable to add CShader object to the buffer." );
                                }
                            }
                            else
                            {
                                VKE_LOG_ERR( "Unable to allocate memory for CShader object." );
                            }
                        }
                    }
                    else
                    {
                        pRet = ShaderRefPtr{ pShader };
                    }
                }
                if( !reuseShader )
                {
                    if( ( Desc.Create.flags & Core::CreateResourceFlags::ASYNC ) == Core::CreateResourceFlags::ASYNC )
                    {
                        /*ShaderManagerTasks::SCreateShaderTask* pTask;
                        {
                            Threads::ScopedLock l( m_aTaskSyncObjects[ShaderManagerTasks::CREATE_SHADER] );
                            pTask = _GetTask( &m_CreateShaderTaskPool );
                        }*/
                        Threads::TSDataTypedTask<CShader*>* pTask;
                        {
                            Threads::ScopedLock l( m_aTaskSyncObjects[ ShaderManagerTasks::CREATE_SHADER ] );
                            pTask = _GetTask( &m_CreateShaderTaskPool );
                        }
                        /*pTask->Desc = Desc;
                        pTask->hash = hash;
                        pTask->shaderType = shaderType;*/
                        Threads::TaskFunction Func = [ this ]( void* pData )
                        {
                            Threads::TASK_RESULT Ret = TaskResults::FAIL;
                            CShader* pShader = ( CShader* )pData;
                            Result res = this->_CreateShader( &pShader );
                            if( VKE_SUCCEEDED( res ) )
                            {
                                Ret = TaskResults::OK;
                            }
                            return Ret;
                        };
                        

                        pTask->TaskData = pShader;
                        pTask->Func = [ & ]( Threads::ITask* pThisTask ) {
                            auto pTask = ( CreateShaderTask* )pThisTask;
                            uint32_t ret = TaskStateBits::FAIL;
                            Result res = this->_CreateShader( &pTask->TaskData );
                            if( VKE_SUCCEEDED( res ) )
                            {
                                ret = TaskStateBits::OK;
                            }
                            return ret;
                        };
                        /*m_pCtx->GetRenderSystem()->GetEngine()->GetThreadPool()->AddTask(
                            Threads::ThreadUsageBits::COMPILE, pTask );*/
                        m_pCtx->GetRenderSystem()->GetEngine()->GetThreadPool()->AddTask(
                        Threads::ThreadUsageBits::COMPILE,
                            Desc.Shader.Name.GetData(), Func, pShader );
                    }
                    else
                    {
                        // return _CreateShaderTask( shaderType, hash, Desc );
                        if( VKE_FAILED( _CreateShader( &pShader ) ) )
                        {
                            goto ERR;
                        }
                    }
                }
            }
        ERR:
            return pRet;
        }

        SHADER_TYPE CShaderManager::FindShaderType(cstr_t pFileName)
        {
            /*cstr_t pExt = strrchr(pFileName, '.' );
            if( pExt )
            {
                cstr_t pFileExt = pExt + 1;
                for( uint32_t i = 0; i < ShaderTypes::_MAX_COUNT; ++i )
                {
                    for( uint32_t j = 0; j < m_InitDesc.avShaderExtensions[ i ].GetCount(); ++j )
                    {
                        if( strcmp( pFileExt, m_InitDesc.avShaderExtensions[ i ][ j ] ) == 0 )
                        {
                            return static_cast< SHADER_TYPE >( i );
                        }
                    }
                }
            }*/
            for (uint32_t i = 0; i < ShaderTypes::_MAX_COUNT; ++i)
            {
                for (uint32_t j = 0; j < m_InitDesc.avShaderExtensions[i].GetCount(); ++j)
                {
                    const char* pExt = m_InitDesc.avShaderExtensions[ i ][ j ];
                    std::regex Re( std::format( ".*[.]{}$", pExt ), std::regex_constants::ECMAScript | std::regex_constants::icase );
                    if (std::regex_match(pFileName, Re))
                    {
                        return static_cast<SHADER_TYPE>( i );
                    }
                }
            }
            return ShaderTypes::_MAX_COUNT;
        }

        Result CShaderManager::_CreateShader( CShader** ppInOut )
        {
            Result ret = VKE_OK;
            CShader* pShader = *ppInOut;
            const auto stages = pShader->m_resourceStages;
            //const uint32_t resState = pShader->GetResourceState();

            if( stages & Core::ResourceStages::INIT )
            {
                // TODO: hash is already calculated, use it

            }

            ShaderPtr pTmp{ pShader };

            if( stages & Core::ResourceStages::LOAD )
            {
                ret = LoadShader( &pTmp );
                if( VKE_FAILED( ret ) )
                {
                    goto ERR;
                }
            }
            if( stages & Core::ResourceStages::PREPARE )
            {
                ret = PrepareShader( &pTmp );
                if( VKE_FAILED( ret ) )
                {
                    goto ERR;
                }
            }
            return ret;
        ERR:
            /*auto& Allocator = m_aShaderFreeListPools[pShader->m_Desc.type];
            Threads::SyncObject l( m_aShaderTypeSyncObjects[pShader->m_Desc.type] );
            _DestroyShader( &Allocator, ppInOut );*/
            pShader->m_resourceStates |= Core::ResourceStates::INVALID;
            return ret;
        }

        ShaderRefPtr CShaderManager::_CreateShaderTask( SHADER_TYPE shaderType, hash_t hash,
            const SCreateShaderDesc& Desc )
        {
            ShaderPtr pRet;

            bool reuseShader = false;
            CShader* pShader = nullptr;
            {
                CShader::SHandle Handle;
                Handle.value = hash;

                Threads::SyncObject& SyncObj = m_aShaderTypeSyncObjects[shaderType];
                Threads::ScopedLock l( SyncObj );

                ShaderBuffer& Buffer = m_aShaderBuffers[shaderType];
                // Try to get this object if already created
                reuseShader = Buffer.Find( Handle.hash, &pShader );
                if( !reuseShader )
                {
                    if( !Buffer.Reuse( Handle.hash, Handle.hash, &pShader ) )
                    {
                        auto& Allocator = m_aShaderFreeListPools[shaderType];
                        if( VKE_SUCCEEDED( Memory::CreateObject( &Allocator, &pShader, this, shaderType ) ) )
                        {
                            if( Buffer.Add( Handle.hash, pShader ) )
                            {
                                pShader->Init( Desc.Shader, hash );
                                pShader->m_Desc.type = shaderType;
                            }
                            else
                            {
                                VKE_LOG_ERR( "Unable to add CShader object to the buffer." );
                            }
                        }
                        else
                        {
                            VKE_LOG_ERR( "Unable to allocate memory for CShader object." );
                        }
                    }
                }
                else
                {
                    pRet = ShaderPtr( pShader );
                }
            }
            if( pShader && !reuseShader )
            {
                //const uint32_t resState = pShader->GetResourceState();

                if( Desc.Create.stages & Core::ResourceStages::INIT )
                {
                    // TODO: hash is already calculated, use it

                }
                pRet = ShaderPtr( pShader );
                if( Desc.Create.stages & Core::ResourceStages::LOAD )
                {
                    if( VKE_SUCCEEDED( LoadShader( &pRet ) ) )
                    {
                    }
                    else
                    {
                        goto FAIL;
                    }
                }
                if( Desc.Create.stages & Core::ResourceStages::PREPARE )
                {
                    if( VKE_SUCCEEDED( PrepareShader( &pRet ) ) )
                    {

                    }
                    else
                    {
                        goto FAIL;
                    }
                }

            }
            return ShaderRefPtr( pRet );
        FAIL:
            pRet.Release();
            _FreeShader( pShader );
            return ShaderRefPtr( pRet );
        }

        Result CShaderManager::PrepareShader(ShaderPtr* ppShader)
        {
            CShader* pShader = ( *ppShader ).Get();
            return _PrepareShaderTask( &pShader );
        }

        Result CShaderManager::LoadShader(ShaderPtr* ppShader)
        {
            CShader* pShader = ( *ppShader ).Get();
            return _LoadShaderTask( &pShader );
        }

        Result CShaderManager::_LoadShaderTask(CShader** ppShader)
        {
            Result res = VKE_FAIL;
            CShader* pShader = ( *ppShader );
            Threads::ScopedLock l( pShader->m_SyncObj );
            if( !(pShader->GetResourceState() == Core::ResourceStates::LOADED ) )
            {
                Core::SLoadFileInfo Desc;
                Desc.FileInfo = pShader->m_Desc.FileInfo;
                if( !Desc.FileInfo.FileName.IsEmpty() )
                {
                    Core::FilePtr pFile = m_pFileMgr->LoadFile( Desc );
                    if( pFile.IsValid() )
                    {
                        if( pFile->GetDataSize() > 1 ) // BOM
                        {
                            VKE_ASSERT2( pShader->m_pFile.IsNull(), "Current file must be released." );
                            pShader->_SetFile( pFile );
                            res = VKE_OK;
                        }
                        else
                        {
                            pFile->Release();
                            VKE_LOG_WARN( "Shader file: '" << Desc.FileInfo.FileName << "' is empty." );
                        }
                    }
                }
                else
                {
                    VKE_LOG_ERR("LOADED state is set for a resource file but no fileName provided.");
                }
            }
            else
            {
                res = VKE_OK;
            }
            return res;
        }

        Result _PreprocessIncludes(Core::CFileManager* pFileMgr, cstr_t pBaseDirPath,
            cstr_t pCode, vke_string& strLine, vke_string* pStrOutput)
        {
            std::smatch Match;

            static const std::regex Regex("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");
            char pFullFilePath[ 1024 ] = { 0 };
            std::stringstream ssCode( pCode );

            while( std::getline( ssCode, strLine ) )
            {
                if( std::regex_search( strLine, Match, Regex ) )
                {
                    const std::string& strMatch = Match[ 1 ];
                    Core::SLoadFileInfo Desc;

                    uint32_t fileNameLen = vke_sprintf( pFullFilePath, sizeof( pFullFilePath ), "%s\\%s", pBaseDirPath, strMatch.c_str() );
                    ( void )fileNameLen;
                    Desc.FileInfo.FileName = pFullFilePath;
                    /*Desc.FileInfo.fileNameLen = static_cast< uint16_t >( fileNameLen );
                    Desc.FileInfo.pName = strMatch.c_str();
                    Desc.FileInfo.nameLen = static_cast< uint16_t >( strMatch.length() );*/
                    Core::FilePtr pFile = pFileMgr->LoadFile( Desc );
                    if( pFile.IsValid() )
                    {
                        cstr_t pTmpCode = reinterpret_cast< cstr_t >( pFile->GetData() );
                        _PreprocessIncludes( pFileMgr, pBaseDirPath, pTmpCode, strLine, pStrOutput );
                    }
                    else
                    {
                        VKE_LOG_ERR( "Unable to include file: " << strMatch.c_str() );
                    }
                }
                else
                {
                    pStrOutput->append(strLine);
                }
            }
            pStrOutput->append( "\n" );
            return VKE_OK;
        }

        Result CShaderManager::_PrepareShaderTask(CShader** ppShader)
        {
            Result res = VKE_OK;
            CShader* pShader = ( *ppShader );
            Threads::ScopedLock l( pShader->m_SyncObj );
            if( !( pShader->GetResourceState() == Core::ResourceStates::PREPARED ) )
            {
                // Add preprocessor and includes
                cstr_t pShaderData = reinterpret_cast< cstr_t >( pShader->m_Data.pCode ); //( pShader->m_pFile->GetData() );
                uint32_t shaderDataSize = pShader->m_Data.codeSize; //pShader->m_pFile->GetDataSize();
                VKE_ASSERT2( pShaderData != nullptr && shaderDataSize > 0, "Invalid shader data." );

                const SShaderDesc& Desc = pShader->GetDesc();

                char fileDir[1024];
                char* pFileDir = fileDir;
                Platform::File::GetDirectory( Desc.FileInfo.FileName.GetData(), Desc.FileInfo.FileName.GetCount(), &pFileDir );

                /// @TODO this function reports not freed memory blocks!!!
                //res = _PreprocessIncludes( m_pFileMgr, pFileDir, pShaderData, strLine, &strCode );

                if( VKE_SUCCEEDED( res ) )
                {
                    //pShaderData = strCode.c_str();
                    //shaderDataSize = strCode.length();

                    SCompileShaderInfo Info;
                    Info.pBuffer = pShaderData;
                    Info.bufferSize = shaderDataSize;
                    //Info.type = pShader->m_Desc.type;
                    //Info.pEntryPoint = pShader->m_Desc.EntryPoint.GetData();
                    //Info.pName = pShader->m_Desc.FileInfo.pName;
                    Info.pDesc = &pShader->GetDesc();
                    VKE_ASSERT2(Info.pBuffer, "Shader file must be loaded.");
                    SCompileShaderData Data;
                    const hash_t bytecodeHash = _CalcShaderBytecodeHash(Info);
                    if( bytecodeHash )
                    {
                        res = _ReadShaderCache( bytecodeHash, &Data );
                        bool writeCache = false;
                        if( res == VKE_ENOTFOUND )
                        {
                            res = m_pCompiler->Compile( Info, &Data );
                            cstr_t p = ( cstr_t )&Data.vShaderBinary[ 0 ];
                            VKE_LOG( p );
                            writeCache = true; // write shader cache only if a shader is not available in current cache
                        }
                        if( VKE_SUCCEEDED( res ) )
                        {
                            pShader->m_resourceStates |= Core::ResourceStates::PREPARED;
                            res = _CreateShaderObject( &Data.vShaderBinary[ 0 ], Data.codeByteSize, &pShader );
                        }
                        if( VKE_SUCCEEDED( res ) && writeCache )
                        {
                            _WriteShaderCache( bytecodeHash, Data );
                        }
                    }
                    else
                    {
                        res = VKE_FAIL;
                        VKE_LOG_ERR( "Invalid shader code for shader: " << Info.pDesc->FileInfo.FileName  );
                    }
                    pShader->m_pFile = Core::FileRefPtr();
                }
            }
            return res;
        }

        Result CShaderManager::CreateShaders(const SShadersCreateDesc& Desc, ShaderVec* pvOut)
        {
            Result res = VKE_OK;

            return res;
        }

        Result CShaderManager::Compile()
        {
            Result res = VKE_FAIL;

            SCreateShaderDesc Desc;
            Desc.Create.flags = Core::CreateResourceFlags::DEFAULT;
            //Desc.Create.pfnCallback = []( const void*, void* ){};
            Desc.Shader.FileInfo.FileName = "data\\shaders\\test.vs";
            //Desc.Shader.FileInfo.fileNameLen = static_cast< uint16_t >( strlen( Desc.Shader.FileInfo.pFileName ) );
            Desc.Shader.type = FindShaderType( Desc.Shader.FileInfo.FileName.GetData() );
            Desc.Shader.vPreprocessor.PushBack( Utils::CString( "#define TEST 1" ) );
            Desc.Shader.vPreprocessor.PushBack( Utils::CString( "#define TEST2 2" ) );
            //ShaderPtr pShader = CreateShader( std::move( Desc ) );
            return res;
        }

        Result CShaderManager::Link()
        {
            Result res = VKE_FAIL;
            //SLinkShaderInfo Info;
            //res = Memory::CreateObject( &m_ShaderProgramFreeListPool, &Info.pProgram );
            //if( VKE_SUCCEEDED( res ) )
            //{
            //    //Memory::Copy< glslang::TShader*, ShaderTypes::_MAX_COUNT >( Info.apShaders, m_CurrCompilationUnit.apShaders );
            //    for( uint32_t i = 0; i < ShaderTypes::_MAX_COUNT; ++i )
            //    {
            //        Info.apShaders[ i ] = m_CurrCompilationUnit.aInfos[ i ].pShader;
            //    }
            //    SLinkShaderData Data;
            //    res = m_pCompiler->Link( Info, &Data );
            //}
            //else
            //{
            //    VKE_LOG_ERR( "Unable to allocate memory for glslang::TProgram object." );
            //}
            return res;
        }

        void CShaderManager::_FreeShader(CShader* pShader)
        {
            //if( pShader->GetRefCount() == 0 )
            {
                SHADER_TYPE type = pShader->m_Desc.type;
                VKE_ASSERT2( type < ShaderTypes::_MAX_COUNT, "Invalid shader type." );
                Threads::ScopedLock l( m_aShaderTypeSyncObjects[ type ] );
                {
                    //m_pCtx->_GetDevice().DestroyObject( nullptr, &pShader->m_vkModule );
                    m_pCtx->_NativeAPI().DestroyShader( &pShader->m_hDDIObject, nullptr );
                    //m_aShaderBuffers[ type ].vFreeElements.PushBack( pShader );
                    m_aShaderBuffers[ type ].AddFree( pShader->GetHandle().handle );
                }
            }
        }

        void* CShaderManager::_AllocateMemory(size_t size, size_t alignment)
        {
            const size_t alignedSize = Math::Round( size, alignment );
            void* pMem = VKE_MALLOC( alignedSize );
            return pMem;
        }

        void CShaderManager::_FreeMemory(void* /*pMemory*/, size_t /*size*/, size_t /*alignment*/)
        {

        }

        ShaderRefPtr CShaderManager::GetShader( const hash_t& hash, SHADER_TYPE type )
        {
            CShader* pShader;
            ShaderRefPtr pRet;
            if( m_aShaderBuffers[type].Find( hash, &pShader ) )
            {
                pRet = ShaderRefPtr( pShader );
            }
            return pRet;
        }

        ShaderRefPtr CShaderManager::GetShader( ShaderHandle hShader )
        {
            CShader::SHandle Handle;
            Handle.value = hShader.handle;
            SHADER_TYPE type = static_cast< SHADER_TYPE >( Handle.type );
            return GetShader( Handle.hash, type );
        }



        /*typedef void* (VKAPI_PTR *PFN_vkAllocationFunction)(
    void*                                       pUserData,
    size_t                                      size,
    size_t                                      alignment,
    VkSystemAllocationScope                     allocationScope);*/
        void* VkAllocateCallback(void* pUser, size_t size, size_t alignment, VkSystemAllocationScope /*vkScope*/)
        {
            CShaderManager* pMgr = reinterpret_cast< CShaderManager* >( pUser );
            return pMgr->_AllocateMemory(size, alignment);
        }

        Result CShaderManager::_CreateShaderObject(const SCompileShaderData::BinaryElement* pBinary, size_t size, CShader** ppInOut)
        {
            Result ret = VKE_FAIL;
            {
                CShader* pShader = ( *ppInOut );

                SShaderData Data;
                Data.pCode = reinterpret_cast<const uint8_t*>(pBinary);
                Data.codeSize = static_cast< uint32_t >( size );
                Data.stage = ShaderCompilationStages::COMPILED_IR_BINARY;
                Data.type = pShader->GetDesc().type;
                DDIShader hShader = m_pCtx->_NativeAPI().CreateShader( Data, nullptr );
                if( hShader != DDI_NULL_HANDLE )
                {
                    pShader->m_hDDIObject = hShader;
                    ret = VKE_OK;
                }
            }
            return ret;
        }

        ShaderPtr CShaderManager::GetDefaultShader( SHADER_TYPE type )
        {
            return m_apDefaultShaders[ type ];
        }

        Result CShaderManager::_CreateDefaultShaders()
        {
            Result ret = VKE_OK;

            {
                static cstr_t pGLSLShaderCode = VKE_TO_STRING(
                    #version 450 core\r\n
                    layout( location = 0 ) in vec4 pos;
                    void main()
                    {
                        gl_Position = pos;
                    }
                );

                static cstr_t pHLSLShaderCode = VKE_TO_STRING(
                    float4 main(in float4 pos : SV_Position) : SV_Position
                    {
                        return pos;
                    }
                );

                cstr_t pShaderCode = VKE_USE_HLSL_SYNTAX? pHLSLShaderCode : pGLSLShaderCode;

                SHADER_TYPE type = ShaderTypes::VERTEX;

                SCreateShaderDesc Desc;
                Desc.Create.flags = Core::CreateResourceFlags::DEFAULT;
                Desc.Create.stages = Core::ResourceStages::CREATE | Core::ResourceStages::INIT | Core::ResourceStages::PREPARE;
                SShaderData Data;
                Data.codeSize = static_cast< uint32_t >( strlen( pShaderCode ) );
                Data.pCode = reinterpret_cast< const uint8_t* >( pShaderCode );
                Data.stage = ShaderCompilationStages::HIGH_LEVEL_TEXT;
                Data.type = type;

                Desc.Shader.pData = &Data;
                Desc.Shader.EntryPoint = "main";
                Desc.Shader.type = type;

                ShaderPtr pShader = CreateShader( Desc );
                if( pShader.IsNull() )
                {
                    ret = VKE_FAIL;
                }
                m_apDefaultShaders[ type ] = pShader;
            }
            {
                static cstr_t pGLSLShaderCode =
                    "#version 450 core\r\n"
                    "layout(location=0) out vec4 color;"
                    "void main() { color = vec4(0.0, 1.0, 0.0, 1.0); }";

                static cstr_t pHLSLShaderCode = VKE_TO_STRING
                (
                    float4 main() : SV_TARGET0 { return float4( 0.0, 1.0, 0.0, 1.0 ); }
                );

                cstr_t pShaderCode = VKE_USE_HLSL_SYNTAX ? pHLSLShaderCode : pGLSLShaderCode;

                SHADER_TYPE type = ShaderTypes::PIXEL;

                SCreateShaderDesc Desc;
                Desc.Create.flags = Core::CreateResourceFlags::DEFAULT;
                Desc.Create.stages = Core::ResourceStages::CREATE | Core::ResourceStages::INIT | Core::ResourceStages::PREPARE;
                SShaderData Data;
                Data.codeSize = static_cast< uint32_t >( strlen( pShaderCode ) );
                Data.pCode = reinterpret_cast< const uint8_t* >( pShaderCode );
                Data.stage = ShaderCompilationStages::HIGH_LEVEL_TEXT;
                Data.type = type;

                Desc.Shader.pData = &Data;
                Desc.Shader.EntryPoint = "main";
                Desc.Shader.type = type;

                ShaderPtr pShader = CreateShader( Desc );
                if( pShader.IsNull() )
                {
                    ret = VKE_FAIL;
                }
                m_apDefaultShaders[ type ] = pShader;
            }

            return ret;
        }

        hash_t CShaderManager::_CalcShaderBytecodeHash(const SCompileShaderInfo& Info)
        {
            Utils::SHash Ret;

            const auto& Desc = *Info.pDesc;
            SShaderData* pData = nullptr;
            SShaderData TmpData;
            if (Desc.pData)
            {
                pData = Desc.pData;
            }
            else
            {
                TmpData.pCode = reinterpret_cast<const uint8_t*>(Info.pBuffer);
                TmpData.codeSize = Info.bufferSize;
                pData = &TmpData;
            }
            if( pData && pData->pCode )
            {
                uint64_t* pPtr = ( uint64_t* )pData->pCode;
                uint32_t size = pData->codeSize / sizeof( uint64_t );
                for( uint32_t i = 0; i < size; ++i, ++pPtr )
                {
                    Ret += *pPtr;
                }
            }
            
            return Ret.value;
        }

        Result CShaderManager::_ReadShaderCache(const hash_t& hash, SCompileShaderData* pOut)
        {
            Result ret = VKE_ENOTFOUND;
            if (m_Desc.pShaderCacheFileName)
            {
                if (Platform::File::Exists(m_Desc.pShaderCacheFileName))
                {
                    if (Platform::File::IsDirectory(m_Desc.pShaderCacheFileName))
                    {
                        char pFileName[Config::Resource::MAX_NAME_LENGTH];
                        vke_sprintf(pFileName, sizeof(pFileName), "%s/%llu.%s",
                            m_Desc.pShaderCacheFileName, hash, m_Desc.pShaderCacheFileExt);
                        if (Platform::File::Exists(pFileName))
                        {
                            handle_t hFile = Platform::File::Open(pFileName, Platform::File::Modes::READ);
                            if (hFile)
                            {
                                const uint32_t fileSize = Platform::File::GetSize(hFile);
                                pOut->vShaderBinary.resize(fileSize);

                                Platform::File::SReadData ReadData;
                                ReadData.pData = &pOut->vShaderBinary[0];
                                ReadData.readByteCount = fileSize;
                                ReadData.offset = 0;

                                uint32_t readSize = Platform::File::Read(hFile, &ReadData);
                                Platform::File::Close( &hFile );
                                if (readSize == fileSize)
                                {
                                    pOut->codeByteSize = readSize;
                                    ret = VKE_OK;
                                }
                            }
                        }
                    }
                }
            }
            return ret;
        }

        Result CShaderManager::_WriteShaderCache(const hash_t& hash, const SCompileShaderData& Data)
        {
            Result ret = VKE_FAIL;
            if (m_Desc.pShaderCacheFileName)
            {
                if (!Platform::File::Exists(m_Desc.pShaderCacheFileName))
                {
                    cstr_t pExt = Platform::File::GetExtension(m_Desc.pShaderCacheFileName);
                    if (pExt == nullptr || strcmp(pExt, "") == 0)
                    {
                        Platform::File::CreateDir(m_Desc.pShaderCacheFileName);
                    }
                }
                if (Platform::File::IsDirectory(m_Desc.pShaderCacheFileName))
                {
                    {
                        {
                            char pFileName[Config::Resource::MAX_NAME_LENGTH];
                            vke_sprintf(pFileName, sizeof(pFileName), "%s/%llu.%s",
                                m_Desc.pShaderCacheFileName, hash, m_Desc.pShaderCacheFileExt);
                            handle_t hFile = Platform::File::Create(pFileName, Platform::File::Modes::WRITE);
                            if (hFile)
                            {
                                Platform::File::SWriteInfo Info;
                                Info.pData = &Data.vShaderBinary[0];
                                Info.dataSize = (uint32_t)Data.vShaderBinary.size();
                                Info.offset = 0;
                                Platform::File::Write(hFile, Info);
                                Platform::File::Close(&hFile);
                            }
                        }
                    }
                }
            }
            else
            {
                ret = VKE_OK;
            }
            return ret;
        }

    } // RenderSystem
} // VKE
#endif // VKE_VULKAN_RENDER_SYSTEM