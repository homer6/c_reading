//
// lightmap shader without transparency
// 
// diffuse texture * lightmap texture
//
// COLUMN MAJOR MATRICES
//


// -------------------------------------------- 
// Input parameters
// -------------------------------------------- 

// transforms
float4x4		mWorld; 	// model->world tm
float4x4		mTotal; 	// model->world->view->projection tm

// texture resources (from 3dsmax)
texture 		tDiffuse;
texture 		tLightMap;

// constants
float			FRESNEL_R0 = 0.1;
//float			FRESNEL_MIN = 0;
float			FRESNEL_MIN = 0.2;
float			FRESNEL_MAX = 0.5;
float3			AMBIENT_COLOR = float3(0.25,0.25,0.4);
float3			DIFFUSE_COLOR = float3(0.4,0.4,0.45);


// -------------------------------------------- 
// Data formats
// -------------------------------------------- 

struct VS_INPUT
{
	float3 pos		 : POSITION;
	float2 tc0		 : TEXCOORD0;
	float2 tc1		 : TEXCOORD1;
};

struct VS_OUTPUT
{
	float4 pos		 : POSITION;
	float2 tc0		 : TEXCOORD0;
	float2 tc1		 : TEXCOORD1;
};

struct PS_OUTPUT
{
	float4 col		 : COLOR0;
};


// -------------------------------------------- 
// Texture samplers
// -------------------------------------------- 

sampler diffuseSampler = sampler_state
{
	Texture   = (tDiffuse);
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
};

sampler lightmapSampler = sampler_state
{
	Texture   = (tLightMap);
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
};


// -------------------------------------------- 
// Vertex shaders
// -------------------------------------------- 

VS_OUTPUT lightmapVS( const VS_INPUT v )
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	// transform vertex position
	o.pos = mul( mTotal, float4(v.pos,1) );

	o.tc0 = v.tc0;
	o.tc1 = v.tc1;
	return o;
}


// -------------------------------------------- 
// Pixel shaders
// -------------------------------------------- 

float4 lightmapPS( VS_OUTPUT p ) : COLOR
{
	float4 texc = tex2D( diffuseSampler, p.tc0 );
	float3 lmapc = tex2D( lightmapSampler, p.tc1 );
	return float4( texc.xyz*lmapc, texc.w );
}


// -------------------------------------------- 
// Techniques
// -------------------------------------------- 

technique T0
{
	pass P0
	{
		VertexShader = compile vs_1_1 lightmapVS();
		PixelShader = compile ps_1_1 lightmapPS();
	}
}
