#pragma once

#include "CResource.h"

#include "RenderSystem/Common.h"

#if VKE_USE_DIRECTXTEX
#   include "ThirdParty/DirectXTex/DirectXTex/DirectXTex.h"
#endif

namespace VKE
{
    namespace Core
    {
        struct BitsPerPixel
        {
            enum BPP : uint8_t
            {
                BPP_1 = 1,
                BPP_4 = 4,
                BPP_8 = 8,
                BPP_16 = 16,
                BPP_32 = 32,
                BPP_64 = 64,
                BPP_128 = 128,
                _MAX_COUNT = 5,
                UNKNOWN = 0
            };
        };
        using BITS_PER_PIXEL = BitsPerPixel::BPP;

        struct ImageFileFormats
        {
            enum FORMAT
            {
                UNKNOWN,
                BMP,
                DDS,
                GIF,
                HDR,
                JPG,
                JPEG,
                PNG,
                TIFF,
                TIF,
                TGA,
                _MAX_COUNT
            };
        };
        using IMAGE_FILE_FORMAT = ImageFileFormats::FORMAT;
        using PixelFormats = RenderSystem::Formats;
        using PIXEL_FORMAT = RenderSystem::FORMAT;
        using ImageTypes = RenderSystem::TextureTypes;
        using IMAGE_TYPE = RenderSystem::TextureTypes::TYPE;

        struct SImageDesc
        {
            ImageDimmension     Size;
            image_dimm_t        depth = 1;
            PIXEL_FORMAT        format;
            IMAGE_FILE_FORMAT   fileFormat = ImageFileFormats::UNKNOWN;
            IMAGE_TYPE          type;
            ResourceName        Name;
        };

        class VKE_API CImage : public CObject
        {
            friend class CImageManager;
            VKE_DECL_BASE_RESOURCE();
            public:

                CImage(CImageManager*);
                ~CImage();

                void        Release();

                ImageHandle GetHandle() const { return m_Handle; }

                const SImageDesc&   GetDesc() const { return m_Desc; }

                const uint8_t*  GetData() const;
                uint32_t        GetDataSize() const;
                uint16_t GetBitsPerChannel() const { return m_bitsPerChannel; }
                uint16_t GetBitsPerPixel() const { return m_bpp; }

                void            GetTextureDesc(RenderSystem::STextureDesc* pOut) const;

                Result          Resize(const ImageSize& NewSize);

            protected:

                Result      _Init( const SImageDesc& );
                void        _Destroy();

            protected:

                SImageDesc      m_Desc;
                CImageManager*  m_pMgr = nullptr;
                handle_t        m_hNative = INVALID_HANDLE;
                ImageHandle     m_Handle = INVALID_HANDLE;
                uint16_t         m_bitsPerChannel = 0;
                uint16_t         m_bpp = 0;
#if VKE_USE_DIRECTXTEX
                DirectX::ScratchImage   m_DXImage;
#endif
        };
    } // Core
    using ImagePtr = Utils::TCWeakPtr< Core::CImage >;
    using ImageRefPtr = Utils::TCObjectSmartPtr< Core::CImage >;
} // VKE