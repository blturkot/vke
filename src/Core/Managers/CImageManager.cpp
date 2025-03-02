#include "Core/Managers/CImageManager.h"
#include "Core/Managers/CFileManager.h"
#include "Core/Resources/CImage.h"

#include "RenderSystem/CRenderSystem.h"

#if VKE_USE_DEVIL
#include "IL/il.h"
#include "IL/ilu.h"
#include "IL/ilut.h"
#endif

#if VKE_USE_DIRECTXTEX
#   include "ThirdParty/DirectXTex/DirectXTex/DirectXTex.h"
#endif

#define VKE_LOG_IMAGE_MANAGER 0
#if VKE_LOG_IMAGE_MANAGER
#   define VKE_LOG_IMGR( _msg ) VKE_LOG(_msg)
#else
#   define VKE_LOG_IMGR(_msg)
#endif

namespace VKE
{
    namespace Core
    {
#if DXGI_FORMAT_DEFINED
        vke_force_inline RenderSystem::FORMAT MapDXGIFormatToRenderSystemFormat( DXGI_FORMAT fmt )
        {
            using namespace RenderSystem;
            static const RenderSystem::FORMAT aFormats[] =
            {
                Formats::UNDEFINED, // unknown
                Formats::R32G32B32A32_SFLOAT, // r32g32b32a32 typeless
                Formats::R32G32B32A32_SFLOAT, //
                Formats::R32G32B32A32_UINT, //
                Formats::R32G32B32A32_SINT, //
                Formats::R32G32B32_SFLOAT, // r32g32b32 typeless
                Formats::R32G32B32_SFLOAT, //
                Formats::R32G32B32_UINT, //
                Formats::R32G32B32_SINT, //
                Formats::R16G16B16A16_SFLOAT, // r16g16b16a16 typeless
                Formats::R16G16B16A16_SFLOAT, //
                Formats::R16G16B16A16_UNORM, //
                Formats::R16G16B16A16_UINT, //
                Formats::R16G16B16A16_SNORM, //
                Formats::R16G16B16A16_SINT, //
                Formats::R32G32_SFLOAT, // r32g32 typeless
                Formats::R32G32_SFLOAT, //
                Formats::R32G32_UINT, //
                Formats::R32G32_SINT, //
                Formats::UNDEFINED, // r32g8x24 typeless
                Formats::UNDEFINED, // d32 float s8x24 uint
                Formats::UNDEFINED, // r32 float x8x24 uint
                Formats::UNDEFINED, // x32 typeless g8x24 uint
                Formats::A2R10G10B10_UNORM_PACK32, // r10g10b10a2 typeless
                Formats::A2R10G10B10_UNORM_PACK32, //
                Formats::A2R10G10B10_UINT_PACK32, //
                Formats::B10G11R11_UFLOAT_PACK32, //
                Formats::UNDEFINED, // r8g8b8a8 typeless
                Formats::R8G8B8A8_UNORM, //
                Formats::R8G8B8A8_SRGB, //
                Formats::R8G8B8A8_UINT, //
                Formats::R8G8B8A8_SNORM, //
                Formats::R8G8B8A8_SINT, //
                Formats::UNDEFINED, // r16g16 typeless
                Formats::R16G16_SFLOAT, //
                Formats::R16G16_UNORM, //
                Formats::R16G16_UINT, //
                Formats::R16G16_SNORM, //
                Formats::R16G16_SINT, //
                Formats::UNDEFINED, // r32 typeless
                Formats::D32_SFLOAT, //
                Formats::R32_SFLOAT, //
                Formats::R32_UINT, //
                Formats::R32_SINT, //
                Formats::UNDEFINED, // r24g8 typeless
                Formats::D24_UNORM_S8_UINT, //
                Formats::X8_D24_UNORM_PACK32, //
                Formats::UNDEFINED, // x24 typeless g8 uint
                Formats::UNDEFINED, // r8g8 typeless
                Formats::R8G8_UNORM, //
                Formats::R8G8_UINT, //
                Formats::R8G8_SNORM, //
                Formats::R8G8_SINT, //
                Formats::UNDEFINED, // r16 typeless
                Formats::R16_SFLOAT, //
                Formats::D16_UNORM, //
                Formats::R16_UNORM, //
                Formats::R16_UINT, //
                Formats::R16_SNORM, //
                Formats::R16_SINT, //
                Formats::UNDEFINED, // r8 typeless
                Formats::R8_UNORM, //
                Formats::R8_UINT, //
                Formats::R8_SNORM, //
                Formats::R8_SINT, //
                Formats::UNDEFINED, // a8 unorm
                Formats::UNDEFINED, // r1 unorm
                Formats::UNDEFINED, // r9g9b9e5 sharedxp
                Formats::UNDEFINED, // r8g8 b8g8 unorm
                Formats::UNDEFINED, // g8r8 g8b8 unorm
                Formats::UNDEFINED, // bc1 typeless
                Formats::BC1_RGB_UNORM_BLOCK, //
                Formats::BC1_RGB_SRGB_BLOCK, //
                Formats::UNDEFINED, // bc2 typeless
                Formats::BC2_UNORM_BLOCK, //
                Formats::BC2_SRGB_BLOCK, //
                Formats::BC3_UNORM_BLOCK, // bc3 typeless
                Formats::BC3_UNORM_BLOCK, //
                Formats::BC3_SRGB_BLOCK, //
                Formats::BC4_UNORM_BLOCK, //
                Formats::BC4_UNORM_BLOCK, //
                Formats::BC4_SNORM_BLOCK, //
                Formats::BC5_UNORM_BLOCK, // typeless
                Formats::BC5_UNORM_BLOCK, //
                Formats::BC5_SNORM_BLOCK, //
                Formats::B5G6R5_UNORM_PACK16, //
                Formats::B5G5R5A1_UNORM_PACK16, //
                Formats::B8G8R8A8_UNORM, //
                Formats::B8G8R8A8_UNORM, // b8g8r8x8 unorm
                Formats::UNDEFINED, // r10g10b10 xr bias a2 unorm
                Formats::B8G8R8A8_UNORM, // b8g8r8a8 typeless
                Formats::B8G8R8A8_SRGB, //
                Formats::B8G8R8A8_UNORM, // b8g8r8x8 typeless
                Formats::B8G8R8A8_SRGB, // b8g8r8x8 srgb
                Formats::BC6H_SFLOAT_BLOCK, // bc6h typeless
                Formats::BC6H_UFLOAT_BLOCK, //
                Formats::BC6H_SFLOAT_BLOCK, //
                Formats::BC7_UNORM_BLOCK, // bc7 typeless
                Formats::BC7_UNORM_BLOCK, //
                Formats::BC7_SRGB_BLOCK, //
                Formats::UNDEFINED, // ayuv
                Formats::UNDEFINED, // y410
                Formats::UNDEFINED, // y416
                Formats::UNDEFINED, // nv12
                Formats::UNDEFINED, // p010
                Formats::UNDEFINED, // p016
                Formats::UNDEFINED, // 420 opaque
                Formats::UNDEFINED, // yuv2
                Formats::UNDEFINED, // y210
                Formats::UNDEFINED, // y216
                Formats::UNDEFINED, // nv11
                Formats::UNDEFINED, // ai44
                Formats::UNDEFINED, // ia44
                Formats::UNDEFINED, // p8
                Formats::UNDEFINED, // a8p8
                Formats::B4G4R4A4_UNORM_PACK16, //
                Formats::UNDEFINED, // p208
                Formats::UNDEFINED, // v208
                Formats::UNDEFINED // v408
            };
            return aFormats[ fmt ];
        }

        vke_force_inline DXGI_FORMAT MapPixelFormatToDXGIFormat( const PIXEL_FORMAT& fmt )
        {
            static const DXGI_FORMAT aFormats[] =
            {
                DXGI_FORMAT_UNKNOWN, // UNDEFINED,
                DXGI_FORMAT_UNKNOWN, // R4G4_UNORM_PACK8,
                DXGI_FORMAT_UNKNOWN, // R4G4B4A4_UNORM_PACK16,
                DXGI_FORMAT_B4G4R4A4_UNORM, // B4G4R4A4_UNORM_PACK16,
                DXGI_FORMAT_UNKNOWN, // R5G6B5_UNORM_PACK16,
                DXGI_FORMAT_B5G6R5_UNORM, // B5G6R5_UNORM_PACK16,
                DXGI_FORMAT_B5G5R5A1_UNORM, // R5G5B5A1_UNORM_PACK16,
                DXGI_FORMAT_UNKNOWN, // B5G5R5A1_UNORM_PACK16,
                DXGI_FORMAT_UNKNOWN, // A1R5G5B5_UNORM_PACK16,
                DXGI_FORMAT_R8_UNORM, // R8_UNORM,
                DXGI_FORMAT_R8_SNORM, // R8_SNORM,
                DXGI_FORMAT_UNKNOWN, // R8_USCALED,
                DXGI_FORMAT_UNKNOWN, // R8_SSCALED,
                DXGI_FORMAT_R8_UINT, // R8_UINT,
                DXGI_FORMAT_R8_SINT, // R8_SINT,
                DXGI_FORMAT_R8_TYPELESS, // R8_SRGB,
                DXGI_FORMAT_R8G8_UNORM, // R8G8_UNORM,
                DXGI_FORMAT_R8G8_SNORM, // R8G8_SNORM,
                DXGI_FORMAT_UNKNOWN, // R8G8_USCALED,
                DXGI_FORMAT_UNKNOWN, // R8G8_SSCALED,
                DXGI_FORMAT_R8G8_UINT, // R8G8_UINT,
                DXGI_FORMAT_R8G8_SINT, // R8G8_SINT,
                DXGI_FORMAT_R8G8_TYPELESS, // R8G8_SRGB,
                DXGI_FORMAT_UNKNOWN, // R8G8B8_UNORM,
                DXGI_FORMAT_UNKNOWN, // R8G8B8_SNORM,
                DXGI_FORMAT_UNKNOWN, // R8G8B8_USCALED,
                DXGI_FORMAT_UNKNOWN, // R8G8B8_SSCALED,
                DXGI_FORMAT_UNKNOWN, // R8G8B8_UINT,
                DXGI_FORMAT_UNKNOWN, // R8G8B8_SINT,
                DXGI_FORMAT_UNKNOWN, // R8G8B8_SRGB,
                DXGI_FORMAT_B8G8R8X8_UNORM, // B8G8R8_UNORM,
                DXGI_FORMAT_B8G8R8X8_UNORM_SRGB, // B8G8R8_SNORM,
                DXGI_FORMAT_UNKNOWN, // B8G8R8_USCALED,
                DXGI_FORMAT_UNKNOWN, // B8G8R8_SSCALED,
                DXGI_FORMAT_UNKNOWN, // B8G8R8_UINT,
                DXGI_FORMAT_UNKNOWN, // B8G8R8_SINT,
                DXGI_FORMAT_UNKNOWN, // B8G8R8_SRGB,
                DXGI_FORMAT_R8G8B8A8_UNORM, // R8G8B8A8_UNORM,
                DXGI_FORMAT_R8G8B8A8_SNORM, // R8G8B8A8_SNORM,
                DXGI_FORMAT_UNKNOWN, // R8G8B8A8_USCALED,
                DXGI_FORMAT_UNKNOWN, // R8G8B8A8_SSCALED,
                DXGI_FORMAT_R8G8B8A8_UINT, // R8G8B8A8_UINT,
                DXGI_FORMAT_R8G8B8A8_SINT, // R8G8B8A8_SINT,
                DXGI_FORMAT_R8G8B8A8_TYPELESS, // R8G8B8A8_SRGB,
                DXGI_FORMAT_B8G8R8A8_UNORM, // B8G8R8A8_UNORM,
                DXGI_FORMAT_UNKNOWN, // B8G8R8A8_SNORM,
                DXGI_FORMAT_UNKNOWN, // B8G8R8A8_USCALED,
                DXGI_FORMAT_UNKNOWN, // B8G8R8A8_SSCALED,
                DXGI_FORMAT_UNKNOWN, // B8G8R8A8_UINT,
                DXGI_FORMAT_UNKNOWN, // B8G8R8A8_SINT,
                DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, // B8G8R8A8_SRGB,
                DXGI_FORMAT_UNKNOWN, // A8B8G8R8_UNORM_PACK32,
                DXGI_FORMAT_UNKNOWN, // A8B8G8R8_SNORM_PACK32,
                DXGI_FORMAT_UNKNOWN, // A8B8G8R8_USCALED_PACK32,
                DXGI_FORMAT_UNKNOWN, // A8B8G8R8_SSCALED_PACK32,
                DXGI_FORMAT_UNKNOWN, // A8B8G8R8_UINT_PACK32,
                DXGI_FORMAT_UNKNOWN, // A8B8G8R8_SINT_PACK32,
                DXGI_FORMAT_UNKNOWN, // A8B8G8R8_SRGB_PACK32,
                DXGI_FORMAT_UNKNOWN, // A2R10G10B10_UNORM_PACK32,
                DXGI_FORMAT_UNKNOWN, // A2R10G10B10_SNORM_PACK32,
                DXGI_FORMAT_UNKNOWN, // A2R10G10B10_USCALED_PACK32,
                DXGI_FORMAT_UNKNOWN, // A2R10G10B10_SSCALED_PACK32,
                DXGI_FORMAT_UNKNOWN, // A2R10G10B10_UINT_PACK32,
                DXGI_FORMAT_UNKNOWN, // A2R10G10B10_SINT_PACK32,
                DXGI_FORMAT_UNKNOWN, // A2B10G10R10_UNORM_PACK32,
                DXGI_FORMAT_UNKNOWN, // A2B10G10R10_SNORM_PACK32,
                DXGI_FORMAT_UNKNOWN, // A2B10G10R10_USCALED_PACK32,
                DXGI_FORMAT_UNKNOWN, // A2B10G10R10_SSCALED_PACK32,
                DXGI_FORMAT_UNKNOWN, // A2B10G10R10_UINT_PACK32,
                DXGI_FORMAT_UNKNOWN, // A2B10G10R10_SINT_PACK32,
                DXGI_FORMAT_R16_UNORM, // R16_UNORM,
                DXGI_FORMAT_R16_SNORM, // R16_SNORM,
                DXGI_FORMAT_UNKNOWN, // R16_USCALED,
                DXGI_FORMAT_UNKNOWN, // R16_SSCALED,
                DXGI_FORMAT_R16_UINT, // R16_UINT,
                DXGI_FORMAT_R16_SINT, // R16_SINT,
                DXGI_FORMAT_R16_FLOAT, // R16_SFLOAT,
                DXGI_FORMAT_R16G16_UNORM, // R16G16_UNORM,
                DXGI_FORMAT_R16G16_SNORM, // R16G16_SNORM,
                DXGI_FORMAT_UNKNOWN, // R16G16_USCALED,
                DXGI_FORMAT_UNKNOWN, // R16G16_SSCALED,
                DXGI_FORMAT_R16G16_UINT, // R16G16_UINT,
                DXGI_FORMAT_R16G16_SINT, // R16G16_SINT,
                DXGI_FORMAT_R16G16_FLOAT, // R16G16_SFLOAT,
                DXGI_FORMAT_UNKNOWN, // R16G16B16_UNORM,
                DXGI_FORMAT_UNKNOWN, // R16G16B16_SNORM,
                DXGI_FORMAT_UNKNOWN, // R16G16B16_USCALED,
                DXGI_FORMAT_UNKNOWN, // R16G16B16_SSCALED,
                DXGI_FORMAT_UNKNOWN, // R16G16B16_UINT,
                DXGI_FORMAT_UNKNOWN, // R16G16B16_SINT,
                DXGI_FORMAT_UNKNOWN, // R16G16B16_SFLOAT,
                DXGI_FORMAT_R16G16B16A16_UNORM, // R16G16B16A16_UNORM,
                DXGI_FORMAT_R16G16B16A16_SNORM, // R16G16B16A16_SNORM,
                DXGI_FORMAT_UNKNOWN, // R16G16B16A16_USCALED,
                DXGI_FORMAT_UNKNOWN, // R16G16B16A16_SSCALED,
                DXGI_FORMAT_R16G16B16A16_UINT, // R16G16B16A16_UINT,
                DXGI_FORMAT_R16G16B16A16_SINT, // R16G16B16A16_SINT,
                DXGI_FORMAT_R16G16B16A16_FLOAT, // R16G16B16A16_SFLOAT,
                DXGI_FORMAT_R32_UINT, // R32_UINT,
                DXGI_FORMAT_R32_SINT, // R32_SINT,
                DXGI_FORMAT_R32_FLOAT, // R32_SFLOAT,
                DXGI_FORMAT_R32G32_UINT, // R32G32_UINT,
                DXGI_FORMAT_R32G32_SINT, // R32G32_SINT,
                DXGI_FORMAT_R32G32_FLOAT, // R32G32_SFLOAT,
                DXGI_FORMAT_R32G32B32_UINT, // R32G32B32_UINT,
                DXGI_FORMAT_R32G32B32_SINT, // R32G32B32_SINT,
                DXGI_FORMAT_R32G32B32_FLOAT, // R32G32B32_SFLOAT,
                DXGI_FORMAT_R32G32B32A32_UINT, // R32G32B32A32_UINT,
                DXGI_FORMAT_R32G32B32A32_SINT, // R32G32B32A32_SINT,
                DXGI_FORMAT_R32G32B32A32_FLOAT, // R32G32B32A32_SFLOAT,
                DXGI_FORMAT_UNKNOWN, // R64_UINT,
                DXGI_FORMAT_UNKNOWN, // R64_SINT,
                DXGI_FORMAT_UNKNOWN, // R64_SFLOAT,
                DXGI_FORMAT_UNKNOWN, // R64G64_UINT,
                DXGI_FORMAT_UNKNOWN, // R64G64_SINT,
                DXGI_FORMAT_UNKNOWN, // R64G64_SFLOAT,
                DXGI_FORMAT_UNKNOWN, // R64G64B64_UINT,
                DXGI_FORMAT_UNKNOWN, // R64G64B64_SINT,
                DXGI_FORMAT_UNKNOWN, // R64G64B64_SFLOAT,
                DXGI_FORMAT_UNKNOWN, // R64G64B64A64_UINT,
                DXGI_FORMAT_UNKNOWN, // R64G64B64A64_SINT,
                DXGI_FORMAT_UNKNOWN, // R64G64B64A64_SFLOAT,
                DXGI_FORMAT_UNKNOWN, // B10G11R11_UFLOAT_PACK32,
                DXGI_FORMAT_R9G9B9E5_SHAREDEXP, // E5B9G9R9_UFLOAT_PACK32,
                DXGI_FORMAT_D16_UNORM, // D16_UNORM,
                DXGI_FORMAT_R24_UNORM_X8_TYPELESS, // X8_D24_UNORM_PACK32,
                DXGI_FORMAT_D32_FLOAT, // D32_SFLOAT,
                DXGI_FORMAT_D32_FLOAT_S8X24_UINT, // S8_UINT,
                DXGI_FORMAT_UNKNOWN, // D16_UNORM_S8_UINT,
                DXGI_FORMAT_D24_UNORM_S8_UINT, // D24_UNORM_S8_UINT,
                DXGI_FORMAT_D32_FLOAT_S8X24_UINT, // D32_SFLOAT_S8_UINT,
                DXGI_FORMAT_BC1_UNORM, // BC1_RGB_UNORM_BLOCK,
                DXGI_FORMAT_BC1_UNORM_SRGB, // BC1_RGB_SRGB_BLOCK,
                DXGI_FORMAT_BC1_UNORM, // BC1_RGBA_UNORM_BLOCK,
                DXGI_FORMAT_BC1_UNORM_SRGB, // BC1_RGBA_SRGB_BLOCK,
                DXGI_FORMAT_BC2_UNORM, // BC2_UNORM_BLOCK,
                DXGI_FORMAT_BC2_UNORM_SRGB, // BC2_SRGB_BLOCK,
                DXGI_FORMAT_BC3_UNORM, // BC3_UNORM_BLOCK,
                DXGI_FORMAT_BC2_UNORM_SRGB, // BC3_SRGB_BLOCK,
                DXGI_FORMAT_BC4_UNORM, // BC4_UNORM_BLOCK,
                DXGI_FORMAT_BC4_SNORM, // BC4_SNORM_BLOCK,
                DXGI_FORMAT_BC5_UNORM, // BC5_UNORM_BLOCK,
                DXGI_FORMAT_BC5_SNORM, // BC5_SNORM_BLOCK,
                DXGI_FORMAT_BC6H_UF16, // BC6H_UFLOAT_BLOCK,
                DXGI_FORMAT_BC6H_SF16, // BC6H_SFLOAT_BLOCK,
                DXGI_FORMAT_BC7_UNORM, // BC7_UNORM_BLOCK,
                DXGI_FORMAT_BC7_UNORM_SRGB, // BC7_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ETC2_R8G8B8_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ETC2_R8G8B8_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ETC2_R8G8B8A1_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ETC2_R8G8B8A1_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ETC2_R8G8B8A8_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ETC2_R8G8B8A8_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // EAC_R11_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // EAC_R11_SNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // EAC_R11G11_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // EAC_R11G11_SNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_4x4_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_4x4_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_5x4_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_5x4_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_5x5_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_5x5_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_6x5_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_6x5_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_6x6_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_6x6_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_8x5_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_8x5_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_8x6_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_8x6_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_8x8_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_8x8_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_10x5_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_10x5_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_10x6_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_10x6_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_10x8_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_10x8_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_10x10_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_10x10_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_12x10_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_12x10_SRGB_BLOCK,
                DXGI_FORMAT_UNKNOWN, // ASTC_12x12_UNORM_BLOCK,
                DXGI_FORMAT_UNKNOWN // ASTC_12x12_SRGB_BLOCK,
            };
            return aFormats[ fmt ];
        }

        vke_force_inline RenderSystem::TEXTURE_TYPE MapTexDimmensionToTextureType( const DirectX::TEX_DIMENSION& dimm )
        {
            static const RenderSystem::TEXTURE_TYPE aTypes[] =
            {
                RenderSystem::TextureTypes::TEXTURE_1D, //
                RenderSystem::TextureTypes::TEXTURE_1D, //
                RenderSystem::TextureTypes::TEXTURE_1D, // TEX_DIMENSION_TEXTURE1D = 2,
                RenderSystem::TextureTypes::TEXTURE_2D, // TEX_DIMENSION_TEXTURE2D = 3,
                RenderSystem::TextureTypes::TEXTURE_3D, // TEX_DIMENSION_TEXTURE3D = 4,
            };
            return aTypes[ dimm ];
        }

#endif
#if VKE_USE_DIRECTXTEX

        bool IsCompressed( DXGI_FORMAT fmt )
        {
            bool ret = false;
            switch( fmt )
            {
                case DXGI_FORMAT_BC1_TYPELESS:
                case DXGI_FORMAT_BC1_UNORM:
                case DXGI_FORMAT_BC1_UNORM_SRGB:
                case DXGI_FORMAT_BC2_TYPELESS:
                case DXGI_FORMAT_BC2_UNORM:
                case DXGI_FORMAT_BC2_UNORM_SRGB:
                case DXGI_FORMAT_BC3_TYPELESS:
                case DXGI_FORMAT_BC3_UNORM:
                case DXGI_FORMAT_BC3_UNORM_SRGB:
                case DXGI_FORMAT_BC4_SNORM:
                case DXGI_FORMAT_BC4_TYPELESS:
                case DXGI_FORMAT_BC4_UNORM:
                case DXGI_FORMAT_BC5_SNORM:
                case DXGI_FORMAT_BC5_TYPELESS:
                case DXGI_FORMAT_BC5_UNORM:
                case DXGI_FORMAT_BC6H_SF16:
                case DXGI_FORMAT_BC6H_TYPELESS:
                case DXGI_FORMAT_BC6H_UF16:
                case DXGI_FORMAT_BC7_TYPELESS:
                case DXGI_FORMAT_BC7_UNORM:
                case DXGI_FORMAT_BC7_UNORM_SRGB:
                {
                    ret = true;
                    break;
                }
            };
            return ret;
        }
#endif

        bool IsCompressed( PIXEL_FORMAT fmt )
        {
#if VKE_USE_DIRECTXTEX
            DXGI_FORMAT dxFmt = MapPixelFormatToDXGIFormat( fmt );
            return IsCompressed( dxFmt );
#else
#error "implement"
#endif
        }

        CImageManager::CImageManager()
        {

        }

        CImageManager::~CImageManager()
        {
            _Destroy();
        }

        Result CImageManager::_Create(const SImageManagerDesc& Desc)
        {
            Result ret = VKE_OK;
            VKE_ASSERT2(Desc.pFileMgr, "");
            m_pFileMgr = Desc.pFileMgr;
            m_MemoryPool.Create( Desc.maxImageCount, sizeof(CImage), 1 );

#if VKE_USE_DEVIL
            ret = _InitDevIL();
#elif VKE_USE_DIRECTXTEX
            ret = _InitDirectXTex();
#endif
            return ret;
        }

        Result CImageManager::_InitDevIL()
        {
            Result ret = VKE_FAIL;
#if VKE_USE_DEVIL
            ilInit();
            iluInit();
            ret = VKE_OK;
#endif
            return ret;
        }

        Result CImageManager::_InitDirectXTex()
        {
            Result ret = VKE_FAIL;
#if VKE_USE_DIRECTXTEX
            ::HRESULT hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            if (hr != S_OK)
            {
                VKE_LOG_ERR("Unable to initialize COM library.");
                ret = VKE_FAIL;
            }
            else
            {
                ret = VKE_OK;
            }
#endif
            return ret;
        }

        void CImageManager::_Destroy()
        {
            uint32_t i = 0;
            ( void )i;
            for( auto& Itr : m_Buffer.Resources.Container )
            {
                auto pImg = Itr.second;
                VKE_LOG_IMGR( "destroy: " << i++ << " " << pImg->m_Desc.Size.width << " " << pImg->GetHandle().handle );
                _DestroyImage( &pImg );
            }
            m_MemoryPool.Destroy();
        }

        void CImageManager::_DestroyImage(Core::CImage** ppImgInOut)
        {
            VKE_ASSERT2( ppImgInOut != nullptr && *ppImgInOut != nullptr, "Invalid image" );
            CImage* pImg = *ppImgInOut;
            pImg->_Destroy();
            Memory::DestroyObject( &m_MemoryPool, &pImg );
            *ppImgInOut = nullptr;
        }

        void CImageManager::DestroyImage(ImageHandle* phImg)
        {
            Threads::ScopedLock l( m_SyncObj );
            CImage* pImg = GetImage(*phImg).Get();
            _FreeImage( pImg );
            *phImg = INVALID_HANDLE;
        }

        hash_t CalcImageHash(const SLoadFileInfo& Info)
        {
            return Info.FileInfo.FileName.CalcHash();
        }

        hash_t CalcImageHash(const SImageDesc& Desc)
        {
            Utils::SHash Hash;
            Hash.Combine(Desc.depth, Desc.format, Desc.Name.GetData(), Desc.Size.width, Desc.Size.height, Desc.type);
            return Hash.value;
        }

        Result CImageManager::_CreateImage(const hash_t& hash, CImage** ppOut)
        {
            Result ret = VKE_FAIL;
            CImage* pImage = nullptr;
            if(!m_Buffer.Find(hash, &pImage))
            {
                Threads::ScopedLock l( m_SyncObj );
                auto Itr = m_Buffer.GetFree( &pImage );
                //if( !m_Buffer.IsValid( Itr ) )
                if(true )
                {
                    if (VKE_SUCCEEDED(Memory::CreateObject(&m_MemoryPool, &pImage, this)))
                    {
                        if (m_Buffer.Add(hash, pImage))
                        {
                            pImage->m_Handle.handle = hash;
                            VKE_LOG_IMGR( "Create image: " << hash );
                            ret = VKE_OK;
                        }
                        else
                        {
                            Memory::DestroyObject(&m_MemoryPool, &pImage);
                        }
                    }
                    else
                    {
                        VKE_LOG_ERR("Unable to allocate memory for CImage object");
                        ret = VKE_ENOMEMORY;
                    }
                }
                else
                {
                    m_Buffer.Remove( Itr );
                    if( m_Buffer.Add( hash, pImage ) )
                    {
                        pImage->m_Handle.handle = hash;
                        ret = VKE_OK;
                    }
                }
            }
            *ppOut = pImage;
            return ret;
        }

        Result CImageManager::Load(const SLoadFileInfo& Info, ImageHandle* phOut)
        {
            // Check if such image is already loaded
            const hash_t hash = CalcImageHash( Info );
            CImage* pImage = nullptr;
            *phOut = INVALID_HANDLE;
            Result ret = VKE_OK;
            bool foundImage = false;
            VKE_LOG_IMGR( "Loading image: '" << Info.FileInfo.FileName << "' with hash: " << hash );
            /*VKE_LOG( "h1 " << std::hash<cstr_t>{}( "data/textures/terrain/heightmap16k_7_7.png" ) );
            VKE_LOG( "h2 " << std::hash<cstr_t>{}( "data/textures/terrain/splat01_7_7.dds" ) );
            VKE_LOG( "h3 " << std::hash<cstr_t>{}( Info.FileInfo.pFileName ) << " " << Info.FileInfo.pFileName );*/
            
            {
                Threads::ScopedLock l( m_SyncObj );
                foundImage = m_Buffer.Find( hash, &pImage );
            /*}
            {*/
                if( !foundImage )
                {
                    {
                        //Threads::ScopedLock l( m_SyncObj );
                        if( !m_Buffer.Reuse( INVALID_HANDLE, hash, &pImage ) )
                        {
                            VKE_LOG_IMGR( "Can't reuse Image. Create image: '" << Info.FileInfo.FileName << "' ( "
                                                                               << hash
                                                                          << " )" );
                            if( VKE_FAILED( Memory::CreateObject( &m_MemoryPool, &pImage, this ) ) )
                            {
                                VKE_LOG_ERR( "Unable to create memory for CImage object: " << Info.FileInfo.FileName );
                            }
                            else
                            {
                                pImage->_LockResource();
                                if( !m_Buffer.Add( hash, pImage ) )
                                {
                                    std::stringstream ss;
                                    ss << "Resource buffer\n";
                                    for( auto& Pair: m_Buffer.Resources.Container )
                                    {
                                        ss << Pair.first << " " << Pair.second->GetDesc().Name.GetData() << "\n";
                                    }
                                    VKE_LOG_IMGR( ss.str() );
                                    VKE_LOG_ERR( "Unable to add Image: '" << Info.FileInfo.FileName << "' ( " << hash
                                                                          << " ) resource to the resource buffer." );
                                    ret = VKE_FAIL;
                                    Memory::DestroyObject( &m_MemoryPool, &pImage );
                                }
                                else
                                {
                                    ret = VKE_OK;
                                    pImage->_AddResourceState( Core::ResourceStates::ALLOCATED );
                                }
                            }
                        }
                        else
                        {
                            VKE_LOG_IMGR( "Reusing Image hash: " << hash );
                            ret = VKE_OK;
                        }
                    }
                }
                else
                {
                    VKE_LOG_IMGR( "Found image: " << Info.FileInfo.FileName << " with hash: " << hash );
                }
            }
            if( VKE_SUCCEEDED(ret) )
            {
                VKE_ASSERT( pImage != nullptr );
                
                if( pImage != nullptr && pImage->IsLockedInThisThread() && pImage->GetHandle() == INVALID_HANDLE )
                {
                    if(!pImage->IsResourceStateSet(Core::ResourceStates::LOADED))
                    {
                        ret = VKE_FAIL;
                        FilePtr pFile = m_pFileMgr->LoadFile(Info);
                        VKE_ASSERT( pFile.IsValid() );
                        if (pFile.IsValid())
                        {
                            pImage->_AddResourceState( Core::ResourceStates::LOADED );
                            VKE_LOG_IMGR( "Creating Image: " << Info.FileInfo.FileName );
                            if (VKE_SUCCEEDED(_CreateImage(pFile.Get(), &pImage)))
                            {
                                VKE_LOG_IMGR( "Image: " << Info.FileInfo.FileName << " created." );
                                pImage->m_Handle.handle = hash;
                                *phOut = pImage->GetHandle();
                                pImage->_AddResourceState( Core::ResourceStates::CREATED );
                                pImage->_AddResourceState( Core::ResourceStates::PREPARED );
                                ret = VKE_OK;
                            }
                            else
                            {
                                pImage->Release();
                            }
                            pFile->Release();
                        }
                    }
                    else
                    {
                        VKE_LOG_IMGR( "Image: " << pImage->GetDesc().Name << " is already loaded." );
                    }
                    pImage->_UnlockResource();
                }
            }
            VKE_ASSERT( ret == VKE_OK );
            if(VKE_SUCCEEDED(ret))
            {
                if( pImage->IsResourceReady() )
                {
                    ret = VKE_OK;
                    *phOut = pImage->GetHandle();
                    VKE_ASSERT2( ( *phOut ) != INVALID_HANDLE, pImage->GetDesc().Name.GetData() );
                }
                else
                {
                    ret = VKE_ENOTREADY;
                }
            }

            return ret;
        }

        void CImageManager::_FreeImage(CImage* pImg)
        {
            Threads::ScopedLock l( m_SyncObj );
            m_Buffer.AddFree( pImg->GetHandle().handle );
        }

        vke_force_inline BITS_PER_PIXEL MapBitsPerPixel(uint32_t bpp)
        {
            BITS_PER_PIXEL ret = BitsPerPixel::UNKNOWN;
#if VKE_USE_DEVIL
            ret = (BitsPerPixel::BPP)bpp;
#endif
            switch( bpp )
            {
                case 1: ret = BitsPerPixel::BPP_1; break;
            }
            return ret;
        }

        vke_force_inline uint32_t GetImageFormatChannelCount(const PIXEL_FORMAT& format)
        {
            static const uint32_t aChannels[] =
            {
                0, // unknown
                1, // red
                2, // rg
                3, // rgb
                4, // rgba
                1, // alpha
                3, // bgr
                4, // bgra
                1, // luminance
            };
            return aChannels[format];
        }

        struct SImageExtValue
        {
            cstr_t              pExt;
            IMAGE_FILE_FORMAT   format;
        };

        static const SImageExtValue g_aFileExtensions[ImageFileFormats::_MAX_COUNT] =
        {
            { "", ImageFileFormats::UNKNOWN },
            { "bmp", ImageFileFormats::BMP },
            { "dds", ImageFileFormats::DDS },
            { "gif", ImageFileFormats::GIF },
            { "hdr", ImageFileFormats::HDR },
            { "jpg", ImageFileFormats::JPG },
            { "jpeg", ImageFileFormats::JPG },
            { "png", ImageFileFormats::PNG },
            { "tif", ImageFileFormats::TIFF },
            { "tiff", ImageFileFormats::TIFF },
            { "tga", ImageFileFormats::TGA },
        };

        IMAGE_FILE_FORMAT CImageManager::_DetermineFileFormat(const CFile* pFile) const
        {
            IMAGE_FILE_FORMAT ret = ImageFileFormats::UNKNOWN;
            // Try use name extension
            cstr_t pExt = pFile->GetExtension();
            if( pExt != nullptr )
            {
                for (uint32_t i = 0; i < ImageFileFormats::_MAX_COUNT; ++i)
                {
                    if (strcmp(pExt, g_aFileExtensions[i].pExt) == 0)
                    {
                        ret = g_aFileExtensions[i].format;
                        break;
                    }
                }
            }
            else
            {
                // If file name ext doesn't exists, use file data
                /// TODO: determine file  format by its header
            }
            return ret;
        }

#if VKE_USE_DIRECTXTEX
        vke_force_inline DirectX::WICCodecs MapFileFormatToWICCodec(IMAGE_FILE_FORMAT fmt)
        {
            static const DirectX::WICCodecs aCodecs[] =
            {
                (DirectX::WICCodecs)0, // unknown
                DirectX::WIC_CODEC_BMP,
                (DirectX::WICCodecs)0, // dds
                DirectX::WIC_CODEC_GIF,
                (DirectX::WICCodecs)0, // hdr
                DirectX::WIC_CODEC_JPEG,
                DirectX::WIC_CODEC_PNG,
                DirectX::WIC_CODEC_TIFF,
                (DirectX::WICCodecs)0, // tga
            };
            return aCodecs[fmt];
        }

        vke_force_inline IMAGE_TYPE MapDXGIDimmensionToImageType(DirectX::TEX_DIMENSION dimm)
        {
            static const IMAGE_TYPE aTypes[] =
            {
                RenderSystem::TextureTypes::TEXTURE_1D, // 0 unused
                RenderSystem::TextureTypes::TEXTURE_1D, // 1 unused
                RenderSystem::TextureTypes::TEXTURE_1D, // 1d
                RenderSystem::TextureTypes::TEXTURE_2D, // 2d
                RenderSystem::TextureTypes::TEXTURE_3D // 3d
            };
            return aTypes[dimm];
        }
#endif


        Result CImageManager::_CreateDirectXTexImage(const CFile* pFile, CImage** ppInOut)
        {
            Result ret = VKE_FAIL;
#if VKE_USE_DIRECTXTEX
            CImage* pImg = *ppInOut;

            const void* pData = pFile->GetData();
            const auto dataSize = pFile->GetDataSize();
            auto fileFormat = _DetermineFileFormat(pFile);
            cstr_t pFileName = pFile->GetDesc().FileName.GetData();
            DirectX::TexMetadata Metadata;
            auto& Image = pImg->m_DXImage;
            
            
            if( fileFormat == ImageFileFormats::DDS )
            {
                static const DirectX::DDS_FLAGS ddsFlags = DirectX::DDS_FLAGS_ALLOW_LARGE_FILES |
                    DirectX::DDS_FLAGS_LEGACY_DWORD;

                ::HRESULT hr = DirectX::LoadFromDDSMemory(pData, dataSize, ddsFlags, &Metadata, Image);
                if( SUCCEEDED( hr ) )
                {
                    if( DirectX::IsTypeless( Metadata.format ) )
                    {
                        Metadata.format = DirectX::MakeTypelessUNORM( Metadata.format );
                        if (!DirectX::IsTypeless(Metadata.format))
                        {
                            Image.OverrideFormat(Metadata.format);
                        }
                        else
                        {
                            VKE_LOG_ERR("Failed to load DDS image: " << pFileName << " due to typeless format.");
                            goto ERR;
                        }
                    }

                    ret = VKE_OK;
                }
                else
                {
                    VKE_LOG_ERR( "Failed to load DDS image: " << pFileName << " error: " << hr );
                    goto ERR;
                }
            }
            else if (fileFormat == ImageFileFormats::TGA)
            {
                ::HRESULT hr = DirectX::LoadFromTGAMemory(pData, dataSize, &Metadata, Image);
                if( SUCCEEDED( hr ) )
                {
                    ret = VKE_OK;
                }
            }
            else if (fileFormat == ImageFileFormats::HDR)
            {
                ::HRESULT hr = DirectX::LoadFromHDRMemory(pData, dataSize, &Metadata, Image);
                if (SUCCEEDED(hr))
                {
                    ret = VKE_OK;
                }
            }
            else // Copied from Texconv.cpp
            {
                // WIC shares the same filter values for mode and dither
                static_assert(static_cast<int>(DirectX::WIC_FLAGS_DITHER) == static_cast<int>(DirectX::TEX_FILTER_DITHER), "WIC_FLAGS_* & TEX_FILTER_* should match");
                static_assert(static_cast<int>(DirectX::WIC_FLAGS_DITHER_DIFFUSION) == static_cast<int>(DirectX::TEX_FILTER_DITHER_DIFFUSION), "WIC_FLAGS_* & TEX_FILTER_* should match");
                static_assert(static_cast<int>(DirectX::WIC_FLAGS_FILTER_POINT) == static_cast<int>(DirectX::TEX_FILTER_POINT), "WIC_FLAGS_* & TEX_FILTER_* should match");
                static_assert(static_cast<int>(DirectX::WIC_FLAGS_FILTER_LINEAR) == static_cast<int>(DirectX::TEX_FILTER_LINEAR), "WIC_FLAGS_* & TEX_FILTER_* should match");
                static_assert(static_cast<int>(DirectX::WIC_FLAGS_FILTER_CUBIC) == static_cast<int>(DirectX::TEX_FILTER_CUBIC), "WIC_FLAGS_* & TEX_FILTER_* should match");
                static_assert(static_cast<int>(DirectX::WIC_FLAGS_FILTER_FANT) == static_cast<int>(DirectX::TEX_FILTER_FANT), "WIC_FLAGS_* & TEX_FILTER_* should match");

                const DirectX::TEX_FILTER_FLAGS texFilter = DirectX::TEX_FILTER_DEFAULT;

                DirectX::WIC_FLAGS wicFlags = DirectX::WIC_FLAGS_NONE | texFilter;

                //hr = LoadFromWICFile(pConv->szSrc, wicFlags, &Metadata, Image);
                ::HRESULT hr = LoadFromWICMemory(pData, dataSize, wicFlags, &Metadata, Image);
                if( SUCCEEDED( hr ) )
                {
                    ret = VKE_OK;
                }
                else
                {
                    VKE_LOG_ERR( "Failed to load WIC image: " << pFileName << " error: " << hr );
                    goto ERR;
                }
            }
            if (VKE_SUCCEEDED(ret))
            {
                SImageDesc Desc;
                Desc.Size.width = (image_dimm_t)Metadata.width;
                Desc.Size.height = (image_dimm_t)Metadata.height;
                Desc.depth = (image_dimm_t)Metadata.depth;
                Desc.type = MapDXGIDimmensionToImageType(Metadata.dimension);
                Desc.format = MapDXGIFormatToRenderSystemFormat(Metadata.format);
                Desc.fileFormat = fileFormat;
                pImg->m_bpp = ( uint16_t )DirectX::BitsPerPixel( Metadata.format );
                pImg->m_bitsPerChannel = (uint16_t)DirectX::BitsPerColor( Metadata.format );
                pImg->_Init( Desc );
            }
#endif
            return ret;
#if VKE_USE_DIRECTXTEX
            ERR:
                ret = VKE_FAIL;
                return ret;
#endif
        }

        Result CImageManager::_CreateImage(CFile* pFile, CImage** ppInOut)
        {
            Result ret = VKE_FAIL;
            CImage* pImg = *ppInOut;
            SImageDesc Desc;
#if VKE_USE_DEVIL
            auto idx = ilGenImage();
            ilBindImage( idx );
            void* pData = (void*)pFile->GetData();
            uint32_t size = pFile->GetDataSize();
            if( ilLoadL( IL_TYPE_UNKNOWN, pData, size ) )
            {
                pImg->m_hNative = idx;
                auto bpp = ilGetInteger(IL_IMAGE_BITS_PER_PIXEL);
                auto format = ilGetInteger(IL_IMAGE_FORMAT);
                auto type = ilGetInteger(IL_IMAGE_TYPE);
                Desc.bitsPerPixel = MapBitsPerPixel( bpp );
                Desc.Size.width = (image_dimm_t)ilGetInteger(IL_IMAGE_WIDTH);
                Desc.Size.height = (image_dimm_t)ilGetInteger(IL_IMAGE_HEIGHT);
                Desc.format = MapImageFormat( format );
                Desc.type = MapImageDataType( type );

                if (pImg->m_hNative != 0)
                {
                    pImg->_Init(Desc);
                    ret = VKE_OK;
                }
            }
            else
            {
                ilDeleteImage(idx);
                VKE_LOG_ERR( "Unable to create DevIL image for image: " << pFile->GetDesc().pFileName );
            }
#elif VKE_USE_DIRECTXTEX
            ret = _CreateDirectXTexImage( pFile, &pImg );
#endif
            if (VKE_SUCCEEDED(ret))
            {
                pImg->m_Desc.Name = pFile->GetDesc().FileName;
            }
            return ret;
        }

        ImageRefPtr CImageManager::GetImage(const ImageHandle& hImg)
        {
            CImage* pImg = nullptr;
            m_Buffer.Find( hImg.handle, &pImg );
            return ImageRefPtr( pImg );
        }

        ImageHandle CImageManager::Copy( const SCreateCopyImageInfo& Info )
        {
            ImageHandle hRet = INVALID_HANDLE;
            CImage* pImg = _Copy( Info );
            if( pImg != nullptr )
            {
                hRet = pImg->GetHandle();
            }
            return hRet;
        }

        CImage* CImageManager::_Copy(const SCreateCopyImageInfo& Info)
        {
            ImageHandle hRet = INVALID_HANDLE;
            ImagePtr pSrcImg = GetImage(Info.hSrcImage);
            const hash_t& srcHash = Info.hSrcImage.handle;
            CImage* pDstImg = nullptr;

            if( pSrcImg.IsValid() )
            {
                SImageRegion Region;
                Region.Size = Info.DstSize;
                Region.Offset = Info.SrcOffset;

                Utils::SHash Hash;
                Hash.Combine( srcHash, Region.Size.width, Region.Size.height, Region.Offset.x,
                              Region.Offset.y );
                Result res = _CreateImage( Hash.value, &pDstImg );
                if( res == VKE_OK )
                {
#if VKE_USE_DIRECTXTEX
                    const auto& SrcMetadata = pSrcImg->m_DXImage.GetMetadata();

                    DirectX::CP_FLAGS flags = DirectX::CP_FLAGS_NONE;
                    //DXGI_FORMAT format = MapPixelFormatToDXGIFormat( Desc.format );

                    ::HRESULT hr = S_OK;
                    if( hr == S_OK )
                    {
                        DirectX::ScratchImage* pSrcDx = &pSrcImg->m_DXImage;
                        DirectX::ScratchImage DecompressedImage;
                        const auto* pSrcMeta = &pSrcDx->GetMetadata();
                        DirectX::ScratchImage TmpDstImg, *pDstDx = nullptr;
                        const bool needDecompress = IsCompressed( pSrcMeta->format );

                        DirectX::Rect Rect;
                        Rect.w = Region.Size.width;
                        Rect.h = Region.Size.height;
                        Rect.x = Region.Offset.x;
                        Rect.y = Region.Offset.y;

                        if( needDecompress )
                        {    
                            hr = DirectX::Decompress( pSrcDx->GetImages(), pSrcDx->GetImageCount(),
                                                      *pSrcMeta, DXGI_FORMAT_UNKNOWN, DecompressedImage );
                            if( hr == S_OK )
                            {
                                pSrcDx = &DecompressedImage;
                                pSrcMeta = &DecompressedImage.GetMetadata();

                                hr = TmpDstImg.Initialize2D( pSrcMeta->format, Rect.w, Rect.h,
                                                             pSrcMeta->arraySize, pSrcMeta->mipLevels );
                                if( hr == S_OK )
                                {
                                    pDstDx = &TmpDstImg;
                                }
                            }
                        }
                        else
                        {
                            hr = pDstImg->m_DXImage.Initialize2D( pSrcMeta->format, Rect.w, Rect.h,
                                                                  pSrcMeta->arraySize,
                                                                  pSrcMeta->mipLevels, flags );
                            if( hr == S_OK )
                            {
                                pDstDx = &pDstImg->m_DXImage;
                            }
                        }

                        if( hr == S_OK )
                        {
                            VKE_ASSERT2( pDstImg != nullptr, "" );
                            hr = DirectX::CopyRectangle( *pSrcDx->GetImage( 0, 0, 0 ), Rect,
                                                         *pDstDx->GetImage( 0, 0, 0 ),
                                                         DirectX::TEX_FILTER_DEFAULT, 0, 0 );
                            if( hr == S_OK )
                            {
                                DXGI_FORMAT dstFmt = ( Info.dstFormat != PixelFormats::UNDEFINED )
                                                         ? MapPixelFormatToDXGIFormat( Info.dstFormat )
                                                         : pSrcMeta->format;
                                const bool isDstFormatCompressed = IsCompressed( dstFmt );
                                dstFmt = ( isDstFormatCompressed ) ? dstFmt : SrcMetadata.format;

                                if( needDecompress || isDstFormatCompressed )
                                {
                                    hr = DirectX::Compress( pDstDx->GetImages(), pDstDx->GetImageCount(),
                                                            SrcMetadata, SrcMetadata.format,
                                                            DirectX::TEX_COMPRESS_DEFAULT,
                                                            DirectX::TEX_THRESHOLD_DEFAULT,
                                                            pSrcImg->m_DXImage );
                                }
                                else
                                {

                                }

                                if( hr == S_OK )
                                {
                                    const auto& SrcDesc = pSrcImg->GetDesc();
                                    auto& Desc = pDstImg->m_Desc;
                                    Desc.depth = SrcDesc.depth;
                                    Desc.format = SrcDesc.format;
                                    Desc.Size = Region.Size;
                                    Desc.type = SrcDesc.type;
                                    Desc.fileFormat = SrcDesc.fileFormat;
                                }
                            }
                        }
                    }
                    else
                    {
                        VKE_LOG_ERR( "Failed to DirectX::ScratchImage::Initialize2D: " << hr );
                    }   
                }
#else
#endif
            }
            return pDstImg;
        }

        Result CImageManager::Slice(const SSliceImageInfo& Info, ImageHandle* pOut)
        {
            Result ret = VKE_OK;
            ImagePtr pImg = GetImage(Info.hSrcImage);
            const auto& SrcDesc = pImg->GetDesc();

#if VKE_USE_DIRECTXTEX

            const bool needDecompress = IsCompressed( pImg->m_DXImage.GetMetadata().format );

            if( !needDecompress )
            {
                for( uint32_t i = 0; i < Info.vRegions.GetCount(); ++i )
                {
                    const SImageRegion& Region = Info.vRegions[ i ];
                    {
                        SCreateCopyImageInfo CopyInfo;
                        CopyInfo.hSrcImage = Info.hSrcImage;
                        CopyInfo.DstSize = Region.Size;
                        CopyInfo.SrcOffset = Region.Offset;
                        CImage* pCopyImg = _Copy( CopyInfo );
                        if( pCopyImg != nullptr )
                        {
                            pCopyImg->m_Desc.Name.Format( "%s_%d", SrcDesc.Name.GetData(), i );
                        }
                        else
                        {
                            ret = VKE_FAIL;
                            break;
                        }
                        pOut[ i ] = pCopyImg->GetHandle();
                    }
                }
            }
            else
            {
                ret = _SliceCompressed( pImg.Get(), Info, pOut );
            }
#else
#error "implement"
#endif
            return ret;
        }

        Result CImageManager::_SliceCompressed( CImage* pImg, const SSliceImageInfo& Info,
                                                ImageHandle* pOut )
        {
            Result ret = VKE_FAIL;
            ImageHandle hRet = INVALID_HANDLE;
#if VKE_USE_DIRECTXTEX
            const auto& SrcMeta = pImg->m_DXImage.GetMetadata();
            const DirectX::ScratchImage* pSrcDx = &pImg->m_DXImage;
            const auto srcHash = pImg->GetHandle().handle;

            DirectX::ScratchImage DecompressedImg, TmpImg;
            ::HRESULT hr = DirectX::Decompress( pSrcDx->GetImages(), pSrcDx->GetImageCount(),
                                                SrcMeta, DXGI_FORMAT_UNKNOWN, DecompressedImg );
            if( hr == S_OK )
            {
                const auto& DecompressedMeta = DecompressedImg.GetMetadata();
                Utils::TCString< wchar_t, 1 > Path, Name, FullPath;
                Utils::TCString< char, 1 > Dir, FileName;

                for( uint32_t i = 0; i < Info.vRegions.GetCount(); ++i )
                {
                    const SImageRegion& Region = Info.vRegions[ i ];
                    const DirectX::TexMetadata* pTmpMeta = &TmpImg.GetMetadata();
                    if( pTmpMeta->width != Region.Size.width || pTmpMeta->height != Region.Size.height )
                    {
                        hr = TmpImg.Initialize2D( DecompressedMeta.format, Region.Size.width,
                                                  Region.Size.height, DecompressedMeta.arraySize,
                                                  DecompressedMeta.mipLevels );
                        pTmpMeta = &TmpImg.GetMetadata();
                    }
                    if( hr == S_OK )
                    {
                        DirectX::Rect Rect;
                        Rect.w = Region.Size.width;
                        Rect.h = Region.Size.height;
                        Rect.x = Region.Offset.x;
                        Rect.y = Region.Offset.y;

                        hr = DirectX::CopyRectangle( *DecompressedImg.GetImage( 0, 0, 0 ), Rect,
                                                     *TmpImg.GetImage( 0, 0, 0 ),
                                                     DirectX::TEX_FILTER_DEFAULT, 0, 0 );

                        if( hr == S_OK )
                        {
                            Utils::SHash Hash;
                            Hash.Combine( srcHash, Region.Size.width, Region.Size.height, Region.Offset.x,
                                          Region.Offset.y, i );
                            CImage* pDstImg;
                            ret = _CreateImage( Hash.value, &pDstImg );
                            if( VKE_SUCCEEDED( ret ) )
                            {
                                auto& DstDx = pDstImg->m_DXImage;
                                const auto& TmpMeta = TmpImg.GetMetadata();
                                hr = DstDx.Initialize2D( SrcMeta.format, TmpMeta.width, TmpMeta.height,
                                                         TmpMeta.arraySize, TmpMeta.mipLevels );
                                if( hr == S_OK )
                                {
                                    hr = DirectX::Compress( TmpImg.GetImages(), TmpImg.GetImageCount(),
                                                            TmpImg.GetMetadata(), SrcMeta.format,
                                                            DirectX::TEX_COMPRESS_DEFAULT,
                                                            DirectX::TEX_THRESHOLD_DEFAULT,
                                                            pDstImg->m_DXImage );
                                    if( hr == S_OK )
                                    {
                                        const auto& DestMeta = pDstImg->m_DXImage.GetMetadata();
                                        auto& Desc = pDstImg->m_Desc;
                                        Desc.depth = ( image_dimm_t )SrcMeta.depth;
                                        Desc.format = MapDXGIFormatToRenderSystemFormat( DestMeta.format );
                                        char name[ Config::Resource::MAX_NAME_LENGTH ];
                                        cstr_t pName = Info.pSaveName ? Info.pSaveName : pImg->GetDesc().Name.GetData();
                                        char* pTmpName = name;
                                        Platform::File::GetFileName( pName, false, &pTmpName );
                                        Desc.Name.Format( "%s_%d_%d", pTmpName, Region.Offset.x, Region.Offset.y );
                                        Desc.Size = Region.Size;
                                        Desc.type = ImageTypes::TEXTURE_2D;
                                        pOut[ i ] = pDstImg->GetHandle();

                                        if( Info.pSavePath )
                                        {
                                            Dir.Resize( Config::Resource::MAX_NAME_LENGTH );
                                            char* pDir = Dir.GetData();
                                            Platform::File::GetDirectory( Info.pSavePath,
                                                (uint32_t)strlen( Info.pSavePath ), &pDir );

                                            if( Platform::File::CreateDir( pDir ) )
                                            {
                                                Path.Resize( Config::Resource::MAX_NAME_LENGTH );
                                                Name.Resize( Config::Resource::MAX_NAME_LENGTH );
                                                FileName.Resize( Config::Resource::MAX_NAME_LENGTH );
                                                FullPath.Resize( Config::Resource::MAX_NAME_LENGTH );
                                                Path.Convert( Info.pSavePath );
                                                pTmpName = FileName.GetData();
                                                Platform::File::GetFileName( Desc.Name.GetData(), false, &pTmpName );
                                                Name.Convert( pTmpName );
                                                FullPath.Format( L"%s/%s.dds", Path.GetData(), Name.GetData() );
                                                
                                                hr = DirectX::SaveToDDSFile( DstDx.GetImages(),
                                                                             DstDx.GetImageCount(),
                                                                             DestMeta,
                                                                             DirectX::DDS_FLAGS_NONE,
                                                                             FullPath.GetData() );
                                            }
                                        }
                                    }
                                    else
                                    {
                                        pOut[ i ] = INVALID_HANDLE;
                                        break;
                                    }
                                }
                                else
                                {
                                    pOut[ i ] = INVALID_HANDLE;
                                    break;
                                }
                            }
                            else
                            {
                                pOut[ i ] = INVALID_HANDLE;
                                break;
                            }
                        }
                        else
                        {
                            pOut[ i ] = INVALID_HANDLE;
                            break;
                        }
                    }
                    else
                    {
                        pOut[ i ] = INVALID_HANDLE;
                        break;
                    }
                }
            }

#else
#error "implement"
#endif
            return ret;
        }

        ImageHandle CImageManager::CreateNormalMap(const ImageHandle& hSrcImg)
        {
            ImageHandle hRet = INVALID_HANDLE;
#if VKE_USE_DIRECTXTEX
            CImage* pImg = GetImage(hSrcImg).Get();
            const auto& Meta = pImg->m_DXImage.GetMetadata();

            SImageDesc ImgDesc = pImg->GetDesc();
            ImgDesc.format = RenderSystem::Formats::R8G8B8A8_UNORM;
            ImgDesc.Name += "_normal";
            hash_t dstHash = CalcImageHash(ImgDesc);
            CImage* pDstImg;
            Result res = _CreateImage(dstHash, &pDstImg);
            if (VKE_SUCCEEDED(res))
            {
                pImg->_Init(ImgDesc);
                auto& DstImage = pDstImg->m_DXImage;
                ::HRESULT hr = DstImage.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, Meta.width, Meta.height, Meta.arraySize, Meta.mipLevels);
                if (hr == S_OK)
                {
                    hr = DirectX::ComputeNormalMap(pImg->m_DXImage.GetImages(), pImg->m_DXImage.GetImageCount(), Meta,
                        DirectX::CNMAP_DEFAULT, 0.0f, DXGI_FORMAT_R8G8B8A8_UNORM, DstImage);
                    if (hr == S_OK)
                    {
                        hRet = pDstImg->GetHandle();
                    }
                    else
                    {

                    }
                }
                else
                {
                }
            }
#else

#endif
            return hRet;
        }

#if VKE_USE_DIRECTXTEX
        DirectX::WICCodecs MapImageFormatToCodec(const IMAGE_FILE_FORMAT& fmt)
        {
            static const DirectX::WICCodecs aCodecs[] =
            {
                (DirectX::WICCodecs)0, // UNKNOWN,
                DirectX::WIC_CODEC_BMP, // BMP,
                (DirectX::WICCodecs)0, // DDS,
                DirectX::WIC_CODEC_GIF, // GIF,
                (DirectX::WICCodecs)0, // HDR,
                DirectX::WIC_CODEC_JPEG, // JPG,
                DirectX::WIC_CODEC_JPEG, // JPEG,
                DirectX::WIC_CODEC_PNG, // PNG,
                DirectX::WIC_CODEC_TIFF, // TIFF,
                DirectX::WIC_CODEC_TIFF, // TIF,
                (DirectX::WICCodecs)0, // TGA,
            };
            return aCodecs[fmt];
        }

#endif

        Result CImageManager::Save(const SSaveImageInfo& Info)
        {
            Result ret = VKE_FAIL;
#if VKE_USE_DIRECTXTEX
            DirectX::Image Img;

            if (Info.hImage != INVALID_HANDLE)
            {
                ImagePtr pImg = GetImage(Info.hImage);
                const auto& DxImage = pImg->m_DXImage;
                //const DirectX::Image* pDxImg = DxImage.GetImage(0, 0, 0);
                const auto& Metadata = DxImage.GetMetadata();

                size_t dataSize = DxImage.GetPixelsSize();

                Img.format = Metadata.format;
                Img.width = Metadata.width;
                Img.height = Metadata.height;
                Img.rowPitch = dataSize / Img.height;
                Img.slicePitch = dataSize;
                Img.pixels = DxImage.GetPixels();
            }
            else
            {
                auto pData = Info.pData;
                Img.format = MapPixelFormatToDXGIFormat(pData->format);
                Img.width = pData->Size.width;
                Img.height = pData->Size.height;
                Img.rowPitch = pData->rowPitch;
                Img.slicePitch = pData->slicePitch;
                Img.pixels = pData->pPixels;
            }

            Utils::TCString< wchar_t, Config::Resource::MAX_NAME_LENGTH > Name;
            Name.Convert( Info.pFileName );
            DirectX::WICCodecs codec = MapImageFormatToCodec(Info.format);

            if(codec != 0)
            {
                REFGUID guid = DirectX::GetWICCodec(codec);
                ::HRESULT hr = DirectX::SaveToWICFile(Img, DirectX::WIC_FLAGS_NONE, guid,
                    Name.GetData(), nullptr);

                if (hr == S_OK)
                {
                    ret = VKE_OK;
                }
            }
            else if (Info.format == ImageFileFormats::TGA)
            {
                ::HRESULT hr = DirectX::SaveToTGAFile(Img, Name.GetData(), nullptr);
                if (hr == S_OK)
                {
                    ret = VKE_OK;
                }
            }
            else if (Info.format == ImageFileFormats::HDR)
            {
                ::HRESULT hr = DirectX::SaveToHDRFile(Img, Name.GetData());
                if (hr == S_OK)
                {
                    ret = VKE_OK;
                }
            }
            else if (Info.format == ImageFileFormats::DDS)
            {
                ::HRESULT hr = DirectX::SaveToDDSFile(Img, DirectX::DDS_FLAGS_NONE, Name.GetData());
                if (hr == S_OK)
                {
                    ret = VKE_OK;
                }
            }
            else
            {
                VKE_LOG_WARN( "Unknown format of file: " << Info.pFileName );
            }

            if (VKE_FAILED(ret))
            {
                VKE_LOG_ERR("Unable to save image to file: " << Info.pFileName);
            }
#else
#error "implement"
#endif
            return ret;
        }

        void CImageManager::_GetTextureDesc(const CImage* pImg, RenderSystem::STextureDesc* pOut) const
        {
#if VKE_USE_DIRECTXTEX
            const auto& Metadata = pImg->m_DXImage.GetMetadata();

            pOut->arrayElementCount = (uint16_t)Metadata.arraySize;
            pOut->format = MapDXGIFormatToRenderSystemFormat(Metadata.format);
            pOut->Size.width = (RenderSystem::TextureSizeType)Metadata.width;
            pOut->Size.height = (RenderSystem::TextureSizeType)Metadata.height;
            pOut->memoryUsage = RenderSystem::MemoryUsages::GPU_ACCESS | RenderSystem::MemoryUsages::TEXTURE | RenderSystem::MemoryUsages::STATIC;
            pOut->mipmapCount = (uint16_t)Metadata.mipLevels;
            pOut->multisampling = RenderSystem::SampleCounts::SAMPLE_1;
            pOut->sliceCount = (uint16_t)Metadata.depth;
            pOut->type = MapTexDimmensionToTextureType(Metadata.dimension);
            pOut->usage = RenderSystem::TextureUsages::SAMPLED | RenderSystem::TextureUsages::TRANSFER_DST;
            //pOut->Name = pImg->GetDesc().Name;
            pOut->SetDebugName( pImg->GetDesc().Name.GetData() );
            //VKE_RENDER_SYSTEM_SET_DEBUG_NAME(*pOut, pOut->Name.GetData());
#else
#error "implement"
#endif
        }

        Result CImageManager::_Resize(const ImageSize& NewSize, CImage** ppInOut)
        {
            Result ret = VKE_FAIL;
            VKE_ASSERT2( ppInOut != nullptr && *ppInOut != nullptr, "" );
            CImage* pImg = *ppInOut;
            const auto& Desc = pImg->GetDesc();
            if( Desc.Size == NewSize )
            {
                ret = VKE_OK;
                return ret;
            }
#if VKE_USE_DIRECTXTEX
            const auto& Meta = pImg->m_DXImage.GetMetadata();
            DirectX::ScratchImage NewImage;
            ::HRESULT hr = NewImage.Initialize2D(Meta.format, NewSize.width, NewSize.height, Meta.arraySize,
                Meta.mipLevels);

            if (hr == S_OK)
            {
                const bool needDecompress = IsCompressed(Meta.format);
                DirectX::ScratchImage TmpImg;
                DirectX::ScratchImage* pSrcImg = &pImg->m_DXImage;
                const DirectX::TexMetadata* pSrcMetadata = &pSrcImg->GetMetadata();

                if (needDecompress)
                {
                    hr = DirectX::Decompress(pImg->m_DXImage.GetImages(), pImg->m_DXImage.GetImageCount(), Meta,
                        DXGI_FORMAT_UNKNOWN, TmpImg);
                    if (hr == S_OK)
                    {
                        pSrcImg = &TmpImg;
                        pSrcMetadata = &TmpImg.GetMetadata();
                    }
                }

                hr = DirectX::Resize(pSrcImg->GetImages(), pSrcImg->GetImageCount(), *pSrcMetadata,
                    (size_t)NewSize.width, (size_t)NewSize.height, DirectX::TEX_FILTER_DEFAULT, NewImage);
                if (hr == S_OK)
                {
                    /*REFGUID guid = DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG);
                    DirectX::SaveToWICFile(*NewImage.GetImage(0,0,0), DirectX::WIC_FLAGS_NONE,
                        guid,
                        L"resize.png");*/

                    if (needDecompress)
                    {
                        DirectX::ScratchImage NewCompressed;
                        hr = DirectX::Compress(NewImage.GetImages(), NewImage.GetImageCount(), NewImage.GetMetadata(),
                            Meta.format, DirectX::TEX_COMPRESS_BC7_QUICK, 0.0f, NewCompressed);
                        if (hr == S_OK)
                        {
                            NewImage.Release();
                            NewImage = std::move(NewCompressed);
                        }
                    }

                    if (hr == S_OK)
                    {
                        pImg->m_DXImage.Release();
                        //hr = pImg->m_DXImage.Initialize(NewImage.GetMetadata());
                        pImg->m_DXImage = std::move(NewImage);
                        if (hr == S_OK)
                        {
                            //const auto w = pImg->m_DXImage.GetMetadata().width;
                            ret = VKE_OK;
                        }
                        else
                        {
                            VKE_LOG_ERR("Unable to reinitialize image.");
                        }
                    }
                }
                else
                {
                    VKE_LOG_ERR("Unable to Resize DirectX Tex image.");
                }
            }
            else
            {
                VKE_LOG_ERR("Unable to initialize2D new image for resize.");
            }
#else
#error "implement"
#endif
            return ret;
        }

        Result CImageManager::Resize(const ImageSize& NewSize, ImagePtr* ppInOut)
        {
            CImage* pImg = ppInOut->Get();
            return _Resize(NewSize, &pImg);
        }

        Result CImageManager::Resize(const ImageSize& NewSize, ImageHandle* pInOut)
        {
            CImage* pImg = GetImage(*pInOut).Get();
            return _Resize( NewSize, &pImg );
        }

    } // Core
} // VKE