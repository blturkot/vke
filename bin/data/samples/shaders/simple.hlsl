float4 VertexShaderMain( float3 position : POSITION ) : SV_POSITION
{
    float4 output = float4( position, 1.0f );
    return output;
}

float4 PixelShaderMain( float4 vtxInput : SV_POSITION ) : SV_TARGET
{
    float4 output = float4(0.6, 0.6, 0.4, 1.0f);
    return output;
}