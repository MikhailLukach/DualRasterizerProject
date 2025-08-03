float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WorldMatrix;

float3 gLightDirection = normalize(float3(0.577f, -0.577f, 0.577f));
float3 gLightColour = float3(1.0f, 1.0f, 1.0f);
float3 gCameraPosition : CAMERA;
static const float PI = 3.14159265359f;
static const float gLightIntensity = 7.0f;
static const float gShininess = 25.0f;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

//Sample
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap; //or mirror, clamp, border
    AddressV = Wrap; //or mirror, clamp, border
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap; //or mirror, clamp, border
    AddressV = Wrap; //or mirror, clamp, border
};

SamplerState samAntisotropic
{
    Filter = ANISOTROPIC;
    AddressU = Wrap; //or mirror, clamp, border
    AddressV = Wrap; //or mirror, clamp, border
};

RasterizerState gRasterizerState
{
    CullMode = none;
    FrontCounterClockwise = false;
};

BlendState gBlendState
{
    BlendEnable[0] = false;
    RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = 1;
    DepthFunc = less;
    StencilEnable = false;
};

//input/output Structs
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 Uv : TEXCOORD;
    float3 Tangent : TANGENT; //new
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : WORLD; //new
    float2 Uv : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT; //new
};

//Vertex Shader
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    
    //new
    output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);

    output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);

    float3x3 rotationMatrix = (float3x3) gWorldMatrix;

    output.Normal = normalize(mul(input.Normal, rotationMatrix));
    output.Tangent = normalize(mul(input.Tangent, rotationMatrix));
    //new
    
    output.Uv = input.Uv;
    return output;
}

//Pixel Shader
float4 PS(VS_OUTPUT input, SamplerState sampstate) : SV_TARGET
{
    
    // Sample textures
    float3 diffuseColor = gDiffuseMap.Sample(sampstate, input.Uv).rgb;
    float3 specularColor = gSpecularMap.Sample(sampstate, input.Uv).rgb;
    float phongExponent = gGlossinessMap.Sample(sampstate, input.Uv).r;

    // Transform normal from tangent space to world space
    //float3 tangentNormal = 2.0f * gNormalMap.Sample(sampstate, input.Uv).rgb - 1.0f; // Map [0,1] to [-1,1]
    const float3 sampledNormal = 2.f * gNormalMap.Sample(sampstate, input.Uv.xy).xyz - float3(1.f, 1.f, 1.f);
    
    float3 binormal = cross(normalize(input.Normal), normalize(input.Tangent));
    //float3 binormal = normalize(cross(input.Normal, input.Tangent));
    //float3x3 tangentSpaceAxis = float3x3(normalize(input.Tangent), binormal, normalize(input.Normal));
    const float4x4 tangentSpaceMatrix = float4x4(
        float4(input.Tangent.xyz, 0.f),
        float4(binormal.xyz, 0.f),
        float4(input.Normal.xyz, 0.f),
        float4(0.f, 0.f, 0.f, 1.f)
    );
    float3 worldNormal = normalize(mul(float4(sampledNormal, 0.0f), tangentSpaceMatrix));

    // Calculate Lambert cosine
    float3 lightDir = normalize(gLightDirection);
    float lambertCosine = saturate(dot(worldNormal, -lightDir));
    
    // Skip pixels facing away from the light
    if (lambertCosine <= 0.0f)
        return float4(0.0f, 0.0f, 0.0f, 1.0f);

    // Calculate reflection vector
    float3 reflectDir = reflect(lightDir, worldNormal);

    // Calculate Phong specular
    float3 viewDir = normalize(gCameraPosition - input.WorldPosition.xyz);
    float cosA = abs(dot(reflectDir, viewDir));
    float3 phongSpecular = specularColor * pow(cosA, phongExponent * gShininess);

    // Calculate Lambertian BRDF diffuse term
    float3 lambertDiffuse = (diffuseColor * gLightIntensity * gLightColour) / PI;
    // Combine terms based on lighting mode
    float3 finalColor;
    finalColor = lambertCosine * (lambertDiffuse + phongSpecular);
    return float4(finalColor, 1.0f); // Final color with alpha = 1.0
    //return float4(diffuseColor, 1.0f);
    //return float4(gDiffuseMap.Sample(samPoint, input.Uv));
    //return float4(1.0f, 1.0f, 1.0f, 1.0f);
    //return float4(tangentNormal.xyz, 1.f);
    //return float4(input.Normal, 1.f);
    //return lambertCosine * float4(1.0f, 1.0f, 1.0f, 1.0f);
}

float4 PSPoint(VS_OUTPUT input) : SV_TARGET
{
    return PS(input, samPoint);
}

float4 PSLinear(VS_OUTPUT input) : SV_TARGET
{
    return PS(input, samLinear);
}

float4 PSAnisotropic(VS_OUTPUT input) : SV_TARGET
{
    return PS(input, samAntisotropic);
}

//Technique
technique11 DefaultTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PSPoint()));
    }
}

technique11 LinearTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PSLinear()));
    }
}

technique11 AntisotropicTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PSAnisotropic()));
    }
}