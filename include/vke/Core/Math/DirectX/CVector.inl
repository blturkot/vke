#pragma once
#include "CMatrix.h"
#if VKE_USE_DIRECTX_MATH
namespace VKE
{
    namespace Math
    {
        constexpr CVector3::CVector3( float f ) :
            _Native{ f, f, f }
        {

        }

        constexpr CVector3::CVector3(float x, float y, float z) :
            _Native{ x, y, z }
        {}

        CVector3::CVector3(const CVector4& Other) :
            _Native{ Other.x, Other.y, Other.z }
        {}

        /*CVector3::CVector3(const CVector3& Other) :
            _Native{ Other._Native }
        {}*/

        /*void CVector3::operator=( const float v )
        {
            DirectX::XMStoreFloat3( &_Native, DirectX::XMVectorSet( v, v, v, 0.0f ) );
        }*/

        CVector3 CVector3::operator+( const CVector3& Right ) const
        {
            CVector3 Ret;
            DirectX::XMStoreFloat3( &Ret._Native, DirectX::XMVectorAdd( VKE_XMVEC3( *this ), VKE_XMVEC3( Right ) ) );
            return Ret;
        }

        CVector3 CVector3::operator*( const CVector3& Right ) const
        {
            CVector3 Ret;
            DirectX::XMStoreFloat3( &Ret._Native, DirectX::XMVectorMultiply( VKE_XMVEC3( *this ), VKE_XMVEC3( Right ) ) );
            return Ret;
        }

        CVector3 CVector3::operator-( const CVector3& Right ) const
        {
            CVector3 Ret;
            DirectX::XMStoreFloat3( &Ret._Native, DirectX::XMVectorSubtract( VKE_XMVEC3( *this ), VKE_XMVEC3( Right ) ) );
            return Ret;
        }

        CVector3 CVector3::operator/( const CVector3& Right ) const
        {
            CVector3 Ret;
            DirectX::XMStoreFloat3( &Ret._Native, DirectX::XMVectorDivide( VKE_XMVEC3( *this ), VKE_XMVEC3( Right ) ) );
            return Ret;
        }

        CVector3 CVector3::operator-() const
        {
            CVector3 Ret;
            DirectX::XMStoreFloat3( &Ret._Native, DirectX::XMVectorMultiply( VKE_XMVEC3( *this ), VKE_XMVEC4( CVector4::NEGATIVE_ONE ) ) );
            return Ret;
        }

        CVector3 CVector3::operator+() const
        {
            return *this;
        }

        void CVector3::operator+=( const CVector3& Right )
        {
            DirectX::XMStoreFloat3( &_Native, DirectX::XMVectorAdd( VKE_XMVEC3( *this ), VKE_XMVEC3( Right ) ) );
        }

        void CVector3::operator*=( const CVector3& Right )
        {
            DirectX::XMStoreFloat3( &_Native, DirectX::XMVectorMultiply( VKE_XMVEC3( *this ), VKE_XMVEC3( Right ) ) );
        }

        void CVector3::operator-=( const CVector3& Right )
        {
            DirectX::XMStoreFloat3( &_Native, DirectX::XMVectorSubtract( VKE_XMVEC3( *this ), VKE_XMVEC3( Right ) ) );
        }

        void CVector3::operator/=( const CVector3& Right )
        {
            DirectX::XMStoreFloat3( &_Native, DirectX::XMVectorDivide( VKE_XMVEC3( *this ), VKE_XMVEC3( Right ) ) );
        }

        bool CVector3::IsZero() const
        {
            return Equals( *this, ZERO );
        }

        void CVector3::ConvertToVector4( CVector4* pOut ) const
        {
            pOut->_Native = DirectX::XMLoadFloat3( &_Native );
        }

        void CVector3::ConvertToRadians( CVector3* pOut ) const
        {
            pOut->x = DirectX::XMConvertToRadians( x );
            pOut->y = DirectX::XMConvertToRadians( y );
            pOut->z = DirectX::XMConvertToRadians( z );
        }

        void CVector3::ConvertToDegrees( CVector3* pOut ) const
        {
            pOut->x = DirectX::XMConvertToDegrees( x );
            pOut->y = DirectX::XMConvertToDegrees( y );
            pOut->z = DirectX::XMConvertToDegrees( z );
        }

        void CVector3::Normalize( CVector3* pOut ) const
        {
            DirectX::XMStoreFloat3( &pOut->_Native, DirectX::XMVector3Normalize( VKE_XMVEC3( *this ) ) );
        }

        void CVector3::Normalize()
        {
            Normalize( this );
        }

        void CVector3::Sin( CVector3* pOut ) const
        {
            DirectX::XMStoreFloat3( &pOut->_Native, DirectX::XMVectorSin( VKE_XMVEC3( *this ) ) );
        }

        void CVector3::Cos( CVector3* pOut ) const
        {
            DirectX::XMStoreFloat3( &pOut->_Native, DirectX::XMVectorCos( VKE_XMVEC3( *this ) ) );
        }

        float CVector3::Dot( const CVector3& Other ) const
        {
            auto Ret = DirectX::XMVector3Dot( VKE_XMVEC3( *this ), VKE_XMVEC3( Other ) );
            return DirectX::XMVectorGetX( Ret );
        }

        bool CVector3::Equals( const CVector3& Left, const CVector3& Right )
        {
            return DirectX::XMVector3Equal( VKE_XMVEC3( Left ), VKE_XMVEC3( Right ) );
        }

        bool CVector3::Less( const CVector3& Left, const CVector3& Right )
        {
            return DirectX::XMVector3Less( VKE_XMVEC3( Left ), VKE_XMVEC3( Right ) );
        }

        bool CVector3::Greater( const CVector3& Left, const CVector3& Right )
        {
            return DirectX::XMVector3Greater( VKE_XMVEC3( Left ), VKE_XMVEC3( Right ) );
        }

        bool CVector3::LessOrEquals( const CVector3& Left, const CVector3& Right )
        {
            return DirectX::XMVector3LessOrEqual( VKE_XMVEC3( Left ), VKE_XMVEC3( Right ) );
        }

        bool CVector3::GreaterOrEquals( const CVector3& Left, const CVector3& Right )
        {
            return DirectX::XMVector3GreaterOrEqual( VKE_XMVEC3( Left ), VKE_XMVEC3( Right ) );
        }

        void CVector3::Add( const CVector3& Left, const CVector3& Right, CVector3* pOut )
        {
            DirectX::XMStoreFloat3( &pOut->_Native,
                DirectX::XMVectorAdd( VKE_XMVEC3( Left ), VKE_XMVEC3( Right ) ) );
        }

        void CVector3::Set( const float x, const float y, const float z, CVector3* pOut )
        {
            DirectX::XMStoreFloat3( &pOut->_Native, DirectX::XMVectorSet( x, y, z, 0.0f ) );
        }

        void CVector3::Set( const float v, CVector3* pOut )
        {
            Set( v, v, v, pOut );
        }

        CVector3 CVector3::Add( const CVector3& Left, const CVector3& Right )
        {
            CVector3 Tmp;
            Add( Left, Right, &Tmp );
            return Tmp;
        }

        void CVector3::Sub( const CVector3& Left, const CVector3& Right, CVector3* pOut )
        {
            DirectX::XMStoreFloat3( &pOut->_Native,
                DirectX::XMVectorSubtract( VKE_XMVEC3( Left ), VKE_XMVEC3( Right ) ) );
        }

        CVector3 CVector3::Sub( const CVector3& Left, const CVector3& Right )
        {
            CVector3 Tmp;
            Sub( Left, Right, &Tmp );
            return Tmp;
        }

        void CVector3::Mul( const CVector3& Left, const CVector3& Right, CVector3* pOut )
        {
            DirectX::XMStoreFloat3( &pOut->_Native,
                DirectX::XMVectorMultiply( VKE_XMVEC3( Left ), VKE_XMVEC3( Right ) ) );
        }

        CVector3 CVector3::Mul( const CVector3& Left, const CVector3& Right )
        {
            CVector3 Tmp;
            Mul( Left, Right, &Tmp );
            return Tmp;
        }

        void CVector3::Div( const CVector3& Left, const CVector3& Right, CVector3* pOut )
        {
            DirectX::XMStoreFloat3( &pOut->_Native,
                DirectX::XMVectorDivide( VKE_XMVEC3( Left ), VKE_XMVEC3( Right ) ) );
        }

        CVector3 CVector3::Div( const CVector3& Left, const CVector3& Right )
        {
            CVector3 Tmp;
            Div( Left, Right, &Tmp );
            return Tmp;
        }

        void CVector3::Mad( const CVector4& V1, const CVector4& V2, const CVector4& V3, CVector3* pOut )
        {
            DirectX::XMStoreFloat3( &pOut->_Native,
                DirectX::XMVectorMultiplyAdd( VKE_XMVEC4( V1 ), VKE_XMVEC4( V2 ), VKE_XMVEC4( V3 ) ) );
        }

        void CVector3::Mad( const CVector3& V1, const CVector3& V2, const CVector3& V3, CVector3* pOut )
        {
            DirectX::XMStoreFloat3( &pOut->_Native,
                DirectX::XMVectorMultiplyAdd( VKE_XMVEC3( V1 ), VKE_XMVEC3( V2 ), VKE_XMVEC3( V3 ) ) );
        }

        void CVector3::Less( const CVector3& Left, const CVector3& Right, CVector3* pOut )
        {
            DirectX::XMStoreFloat3( &pOut->_Native, DirectX::XMVectorLess( VKE_XMVEC3( Left ), VKE_XMVEC3( Right ) ) );
        }

        void CVector3::LessOrEquals( const CVector3& Left, const CVector3& Right, CVector3* pOut )
        {
            DirectX::XMStoreFloat3( &pOut->_Native, DirectX::XMVectorLessOrEqual( VKE_XMVEC3( Left ), VKE_XMVEC3( Right ) ) );
        }

        void CVector3::Greater( const CVector3& Left, const CVector3& Right, CVector3* pOut )
        {
            DirectX::XMStoreFloat3( &pOut->_Native, DirectX::XMVectorGreater( VKE_XMVEC3( Left ), VKE_XMVEC3( Right ) ) );
        }

        void CVector3::GreaterOrEquals( const CVector3& Left, const CVector3& Right, CVector3* pOut )
        {
            DirectX::XMStoreFloat3( &pOut->_Native, DirectX::XMVectorGreaterOrEqual( VKE_XMVEC3( Left ), VKE_XMVEC3( Right ) ) );
        }

        void CVector3::Cross( const CVector3& V1, const CVector3& V2, CVector3* pOut )
        {
            DirectX::XMStoreFloat3( &pOut->_Native, DirectX::XMVector3Cross( VKE_XMVEC3( V1 ), VKE_XMVEC3( V2 ) ) );
        }

        void CVector3::Dot( const CVector3& V1, const CVector3& V2, CVector3* pOut )
        {
            DirectX::XMStoreFloat3( &pOut->_Native, DirectX::XMVector3Dot( VKE_XMVEC3( V1 ), VKE_XMVEC3( V2 ) ) );
        }

        float CVector3::Dot( const CVector3& V1, const CVector3& V2 )
        {
            auto Ret = DirectX::XMVector3Dot( VKE_XMVEC3( V1 ), VKE_XMVEC3( V2 ) );
            return DirectX::XMVectorGetX( Ret );
        }

		float CVector3::Length( const CVector3& V )
		{
			auto Ret = DirectX::XMVector3Length( VKE_XMVEC3( V ) );
			return DirectX::XMVectorGetX( Ret );
		}

		float CVector3::Distance( const CVector3& V1, const CVector3& V2 )
		{
			return Length( V1 - V2 );
		}

    } // Math
} // VKE

// CVector4
namespace VKE
{

    namespace Math
    {
        /*constexpr CVector4::CVector4( float f ) :
            _Native{ f, f, f, f }
        {

        }*/

        CVector4::CVector4( float f )
        {
#if VKE_USE_SSE
            _Native = _mm_load_ps1( &f );
#else
            *pOut = { *ptr, *ptr, *ptr, *ptr };
#endif
        }

        constexpr CVector4::CVector4( float x, float y, float z, float w ) :
            _Native{ x, y, z, w }
        {}

        constexpr CVector4::CVector4( const CVector4& Other ) :
            _Native{ Other._Native }
        {}

        constexpr CVector4::CVector4( const NativeVector4& Other ) :
            _Native{ Other }
        {}

        constexpr CVector4::CVector4( const CVector3& Other ) :
            _Native{ Other.x, Other.y, Other.z, 0.0f }
        {}

        void CVector4::operator=( const float v )
        {
            _Native = DirectX::XMVectorSet( v, v, v, v );
        }

        void CVector4::operator=( const CVector3& V )
        {
            _Native = VKE_XMVEC3( V );
        }

        CVector4 CVector4::operator+( const CVector4& Other ) const
        {
            return CVector4{ DirectX::XMVectorAdd( _Native, Other._Native ) };
        }

        CVector4 CVector4::operator-( const CVector4& Other ) const
        {
            return CVector4{ DirectX::XMVectorSubtract( _Native, Other._Native ) };
        }

        CVector4 CVector4::operator*( const CVector4& Other ) const
        {
            return CVector4{ DirectX::XMVectorMultiply( _Native, Other._Native ) };
        }

        CVector4 CVector4::operator/( const CVector4& Other ) const
        {
            return CVector4{ DirectX::XMVectorDivide( _Native, Other._Native ) };
        }

        void CVector4::operator+=( const CVector4& Other )
        {
            _Native = DirectX::XMVectorAdd( _Native, Other._Native );
        }

        void CVector4::operator-=( const CVector4& Other )
        {
            _Native = DirectX::XMVectorSubtract( _Native, Other._Native );
        }

        void CVector4::operator*=( const CVector4& Other )
        {
            _Native = DirectX::XMVectorMultiply( _Native, Other._Native );
        }

        void CVector4::operator/=( const CVector4& Other )
        {
            _Native = DirectX::XMVectorDivide( _Native, Other._Native );
        }

        CVector4 CVector4::operator+( const float v ) const
        {
            return CVector4{ DirectX::XMVectorAdd( _Native, DirectX::XMVECTOR{ v, v, v, v } ) };
        }

        CVector4 CVector4::operator-( const float v ) const
        {
            return CVector4{ DirectX::XMVectorSubtract( _Native, DirectX::XMVECTOR{ v, v, v, v } ) };
        }

        CVector4 CVector4::operator*( const float v ) const
        {
            return CVector4{ DirectX::XMVectorMultiply( _Native, DirectX::XMVECTOR{ v, v, v, v } ) };
        }

        CVector4 CVector4::operator/( const float v ) const
        {
            return CVector4{ DirectX::XMVectorDivide( _Native, DirectX::XMVECTOR{ v, v, v, v } ) };
        }

        void CVector4::operator+=( const float v )
        {
            _Native = DirectX::XMVectorAdd( _Native, DirectX::XMVECTOR{ v, v, v, v } );
        }

        void CVector4::operator-=( const float v )
        {
            _Native = DirectX::XMVectorSubtract( _Native, DirectX::XMVECTOR{ v, v, v, v } );
        }

        void CVector4::operator*=( const float v )
        {
            _Native = DirectX::XMVectorMultiply( _Native, DirectX::XMVECTOR{ v, v, v, v } );
        }

        void CVector4::operator/=( const float v )
        {
            _Native = DirectX::XMVectorDivide( _Native, DirectX::XMVECTOR{ v, v, v, v } );
        }

        CVector4 CVector4::operator&( const CVector4& Other ) const
        {
            return CVector4{ DirectX::XMVectorAndInt( _Native, Other._Native ) };
        }

        void CVector4::Normalize()
        {
            _Native = DirectX::XMVector4Normalize( _Native );
        }

        bool CVector4::IsZero() const
        {
            return Equals( *this, ZERO );
        }

        void CVector4::ConvertToVector3( CVector3* pOut ) const
        {
            DirectX::XMStoreFloat3( &pOut->_Native, _Native );
        }

        void CVector4::ConvertCompareToBools( bool* pOut ) const
        {
            pOut[ 0 ] = static_cast< bool >( _Native.m128_u32[ 0 ] );
            pOut[ 1 ] = static_cast< bool >( _Native.m128_u32[ 1 ] );
            pOut[ 2 ] = static_cast< bool >( _Native.m128_u32[ 2 ] );
            pOut[ 3 ] = static_cast< bool >( _Native.m128_u32[ 3 ] );
        }

        bool CVector4::Equals( const CVector4& Left, const CVector4& Right )
        {
            return DirectX::XMVector3Equal( VKE_XMVEC4( Left ), VKE_XMVEC4( Right ) );
        }

        bool CVector4::Less( const CVector4& Left, const CVector4& Right )
        {
            return DirectX::XMVector3Less( VKE_XMVEC4( Left ), VKE_XMVEC4( Right ) );
        }

        bool CVector4::Greater( const CVector4& Left, const CVector4& Right )
        {
            return DirectX::XMVector3Greater( VKE_XMVEC4( Left ), VKE_XMVEC4( Right ) );
        }

        bool CVector4::LessEquals( const CVector4& Left, const CVector4& Right )
        {
            return DirectX::XMVector3LessOrEqual( VKE_XMVEC4( Left ), VKE_XMVEC4( Right ) );
        }

        bool CVector4::GreaterEquals( const CVector4& Left, const CVector4& Right )
        {
            return DirectX::XMVector3GreaterOrEqual( VKE_XMVEC4( Left ), VKE_XMVEC4( Right ) );
        }

        void CVector4::Add( const CVector4& Left, const CVector4& Right, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVectorAdd( VKE_XMVEC4( Left ), VKE_XMVEC4( Right ) );
        }

        void CVector4::Set( const float x, const float y, const float z, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVectorSet( x, y, z, 0.0f );
        }

        void CVector4::Set( const float v, CVector4* pOut )
        {
            Set( v, v, v, pOut );
        }

        CVector4 CVector4::Add( const CVector4& Left, const CVector4& Right )
        {
            CVector4 Tmp;
            Add( Left, Right, &Tmp );
            return Tmp;
        }

        void CVector4::Sub( const CVector4& Left, const CVector4& Right, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVectorSubtract( VKE_XMVEC4( Left ), VKE_XMVEC4( Right ) );
        }

        CVector4 CVector4::Sub( const CVector4& Left, const CVector4& Right )
        {
            CVector4 Tmp;
            Sub( Left, Right, &Tmp );
            return Tmp;
        }

        void CVector4::Mul( const CVector4& Left, const CVector4& Right, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVectorMultiply( VKE_XMVEC4( Left ), VKE_XMVEC4( Right ) );
        }

        CVector4 CVector4::Mul( const CVector4& Left, const CVector4& Right )
        {
            CVector4 Tmp;
            Mul( Left, Right, &Tmp );
            return Tmp;
        }

        void CVector4::Div( const CVector4& Left, const CVector4& Right, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVectorDivide( VKE_XMVEC4( Left ), VKE_XMVEC4( Right ) );
        }

        CVector4 CVector4::Div( const CVector4& Left, const CVector4& Right )
        {
            CVector4 Tmp;
            Div( Left, Right, &Tmp );
            return Tmp;
        }

        void CVector4::Less( const CVector4& Left, const CVector4& Right, CVector4* pOut )
        {
            pOut->_Native =DirectX::XMVectorLess( VKE_XMVEC4( Left ), VKE_XMVEC4( Right ) );
        }

        void CVector4::LessOrEquals( const CVector4& Left, const CVector4& Right, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVectorLessOrEqual( VKE_XMVEC4( Left ), VKE_XMVEC4( Right ) );
        }

        void CVector4::Greater( const CVector4& Left, const CVector4& Right, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVectorGreater( VKE_XMVEC4( Left ), VKE_XMVEC4( Right ) );
        }

        void CVector4::GreaterOrEquals( const CVector4& Left, const CVector4& Right, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVectorGreaterOrEqual( VKE_XMVEC4( Left ), VKE_XMVEC4( Right ) );
        }

        void CVector4::And( const CVector4& Left, const CVector4& Right, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVectorAndInt( VKE_XMVEC4( Left ), VKE_XMVEC4( Right ) );
        }

        void CVector4::Mad( const CVector4& V1, const CVector4& V2, const CVector4& V3, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVectorMultiplyAdd( VKE_XMVEC4( V1 ), VKE_XMVEC4( V2 ), VKE_XMVEC4( V3 ) );
        }

        void CVector4::ConvertToInts( int32_t* pInts ) const
        {
            pInts[0] = _Native.m128_i32[0];
            pInts[1] = _Native.m128_i32[1];
            pInts[2] = _Native.m128_i32[2];
            pInts[3] = _Native.m128_i32[3];
        }

        void CVector4::ConvertToUInts( uint32_t* pInts ) const
        {
            DirectX::XMStoreInt4( pInts, _Native );
        }

        void CVector4::Clamp( const CVector4& V, const CVector4& Min, const CVector4& Max, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVectorClamp( VKE_XMVEC4( V ), VKE_XMVEC4( Min ), VKE_XMVEC4( Max ) );
        }

        void CVector4::Saturate( const CVector4& V, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVectorSaturate( VKE_XMVEC4( V ) );
        }

        template<uint32_t DivExponent>
        void CVector4::ConvertUintToFloat( const CVector4& V, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMConvertVectorUIntToFloat( VKE_XMVEC4( V ), DivExponent );
        }

        template<uint32_t MulExponent>
        void CVector4::ConvertFloatToUInt( const CVector4& V, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMConvertVectorFloatToUInt( VKE_XMVEC4( V ), MulExponent );
        }

        void CVector4::Normalize( const CVector4& V, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVector4Normalize( VKE_XMVEC4( V ) );
        }

        void CVector4::Cross( const CVector4& V1, const CVector4& V2, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVector3Cross( VKE_XMVEC4( V1 ), VKE_XMVEC4( V2 ) );
        }

        void CVector4::Max( const CVector4& V1, const CVector4& V2, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVectorMax( VKE_XMVEC4( V1 ), VKE_XMVEC4( V2 ) );
        }

        void CVector4::Min( const CVector4& V1, const CVector4& V2, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVectorMin( VKE_XMVEC4( V1 ), VKE_XMVEC4( V2 ) );
        }

        float CVector4::Length(const CVector4& V)
        {
            auto Ret = DirectX::XMVector3Length( VKE_XMVEC4( V ) );
            return DirectX::XMVectorGetX( Ret );
        }

        float CVector4::Distance(const CVector4& V1, const CVector4& V2)
        {
            const auto v = DirectX::XMVectorSubtract( VKE_XMVEC4( V1 ), VKE_XMVEC4( V2 ) );
            auto Ret = DirectX::XMVector3Length( v );
            return DirectX::XMVectorGetX( Ret );
        }

        void CVector4::Abs(const CVector4& V, CVector4* pOut)
        {
            pOut->_Native = DirectX::XMVectorAbs( VKE_XMVEC4( V ) );
        }

        CVector4 CVector4::Abs(const CVector4& V)
        {
            return CVector4{ DirectX::XMVectorAbs( VKE_XMVEC4( V ) ) };
        }

        void CVector4::Sqrt(const CVector4& V, CVector4* pOut)
        {
            pOut->_Native = DirectX::XMVectorSqrt(VKE_XMVEC4(V));
        }

        void CVector4::Dot( const CVector4& V1, const CVector4& V2, CVector4* pOut )
        {
            pOut->_Native = DirectX::XMVector4Dot( VKE_XMVEC4( V1 ), VKE_XMVEC4( V2 ) );
        }

        float CVector4::Dot(const CVector4& V1, const CVector4& V2)
        {
            auto Native = DirectX::XMVector4Dot(VKE_XMVEC4(V1), VKE_XMVEC4(V2));
            return DirectX::XMVectorGetX(Native);
        }

        int32_t CVector4::MoveMask( const CVector4& Vec )
        {
#if VKE_USE_SSE
            return _mm_movemask_ps( Vec._Native );
#else
#   error "Implement this!!!"
#endif // VKE_USE_SSE
        }

        void CVector4::Load( const float* ptr, CVector4* pOut )
        {
#if VKE_USE_SSE
            pOut->_Native = _mm_load_ps1( ptr );
#else
            *pOut = { *ptr, *ptr, *ptr, *ptr };
#endif
        }

        void CVector4::Load( const float* ptr, CVector4* pOut1, CVector4* pOut2, CVector4* pOut3 )
        {
#if VKE_USE_SSE
            pOut1->_Native = _mm_load_ps1( &ptr[ 0 ] );
            pOut2->_Native = _mm_load_ps1( &ptr[ 1 ] );
            pOut3->_Native = _mm_load_ps1( &ptr[ 2 ] );
#else
            *pOut1 = { ptr[ 0 ], ptr[ 0 ], ptr[ 0 ], ptr[ 0 ] };
            *pOut2 = { ptr[ 1 ], ptr[ 1 ], ptr[ 1 ], ptr[ 1 ] };
            *pOut3 = { ptr[ 2 ], ptr[ 2 ], ptr[ 2 ], ptr[ 2 ] };
#endif
        }

        void CVector4::Load( const float* ptr, CVector4* pOut1, CVector4* pOut2, CVector4* pOut3, CVector4* pOut4 )
        {
#if VKE_USE_SSE
            pOut1->_Native = _mm_load_ps1( &ptr[ 0 ] );
            pOut2->_Native = _mm_load_ps1( &ptr[ 1 ] );
            pOut3->_Native = _mm_load_ps1( &ptr[ 2 ] );
            pOut4->_Native = _mm_load_ps1( &ptr[ 3 ] );
#else
            *pOut1 = { ptr[ 0 ], ptr[ 0 ], ptr[ 0 ], ptr[ 0 ] };
            *pOut2 = { ptr[ 1 ], ptr[ 1 ], ptr[ 1 ], ptr[ 1 ] };
            *pOut3 = { ptr[ 2 ], ptr[ 2 ], ptr[ 2 ], ptr[ 2 ] };
            *pOut4 = { ptr[ 3 ], ptr[ 3 ], ptr[ 3 ], ptr[ 3 ] };
#endif
        }

        template<uint32_t Count>
        void CVector4::Load( const float* ptr, CVector4* pOut )
        {
#if VKE_USE_SSE
            for(uint32_t i = 0; i < Count; ++i)
            {
                pOut[ i ]._Native = _mm_load_ps1( &ptr[ i ] );
            }
#else
            for( uint32_t i = 0; i < Count; ++i )
            {
                pOut[ i ] = { ptr[ i ], ptr[ i ], ptr[ i ], ptr[i] };
            }
#endif
        }

    } // Scene
} // VKE
#endif // VKE_USE_DIRECTX_MATH