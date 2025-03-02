#pragma once

#include "Core/VKECommon.h"
#include "Core/Utils/TCSingleton.h"

namespace VKE
{
    namespace Memory
    {
        class CFreeList;

        class CFreeListPool
        {
            using FreeListVec = std::vector< CFreeList* >;
            using MemRange = TSExtent< memptr_t >;
            using MemRangeVec = std::vector< MemRange >;

            public:

                            CFreeListPool();
                virtual     ~CFreeListPool();

                Result      Create(uint32_t freeListElementCount, size_t freeListElemenetSize, uint32_t freeListCount);
                Result      AddNewLists(uint32_t count);
                void        Destroy();

                memptr_t    Allocate(const uint32_t = 0 /*used for Memory::Create*/);
                /*template<typename _T_> vke_force_inline _T_* Allocate()
                {
                    return reinterpret_cast<_T_*>( this->Alloc() );
                }*/

                Result      Free(const uint32_t, void** ppPtr);

            protected:

                FreeListVec     m_vpFreeLists;
                MemRangeVec     m_vFreeListMemRanges;
                CFreeList*      m_pCurrList = nullptr;
                uint32_t        m_currListId = 0;
                uint32_t        m_freeListElemCount = 0;
                size_t          m_freeListElemSize = 0;
        };
    } // Memory
} // VKE
