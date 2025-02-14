/*
	This is the standard vertex shader

	A vertex shader is responsible for two things:
		* Telling the GPU where the vertex (one of the three in a triangle) should be drawn on screen in a given window
			* The GPU will use this to decide which fragments (i.e. pixels) need to be shaded for a given triangle
		* Providing any data that a corresponding fragment shader will need
			* This data will be interpolated across the triangle and thus vary for each fragment of a triangle that gets shaded
*/

#include <Shaders/shaders.inc>

#if defined( EAE6320_PLATFORM_D3D )

// Constant Buffers
//=================

cbuffer g_constantBuffer_frame : register( b0 )
{
	float4x4 g_transform_worldToCamera;
	float4x4 g_transform_cameraToProjected;

	float g_elapsedSecondCount_systemTime;
	float g_elapsedSecondCount_simulationTime;
	// For float4 alignment
	float2 g_padding;
};

cbuffer g_constantBuffer_drawCall : register( b2 )
{
  float4x4 g_transform_localToWorld;
};

// Entry Point
//============

struct VertexInputData {
	float3 i_vertexPosition_local : POSITION;
	float4 i_vertexColor : COLOR;
	
};
struct VertexToFragmentData
{
	float4 o_vertexPosition_projected : SV_POSITION;
	float4 o_vertexColor : COLOR;
};

void main(

	// Input
	//======

	// The "semantics" (the keywords in all caps after the colon) are arbitrary,
	// but must match the C call to CreateInputLayout()

	// These values come from one of the VertexFormats::s3dObject that the vertex buffer was filled with in C code
	in VertexInputData in_data,
	out VertexToFragmentData out_data
	// Output
	//=======

	// An SV_POSITION value must always be output from every vertex shader
	// so that the GPU can figure out which fragments need to be shaded
	

)
{
	// Transform the local vertex into world space
	float4 vertexPosition_world;
	{
		// This will be done in a future assignment.
		// For now, however, local space is treated as if it is the same as world space.
		float4 vertexPosition_local = float4( in_data.i_vertexPosition_local, 1.0 );
		vertexPosition_world = mul(g_transform_localToWorld, vertexPosition_local);
	}
	// Calculate the position of this vertex projected onto the display
	{
		// Transform the vertex from world space into camera space
		float4 vertexPosition_camera = mul( g_transform_worldToCamera, vertexPosition_world );
		// Project the vertex from camera space into projected space
		out_data.o_vertexPosition_projected = mul( g_transform_cameraToProjected, vertexPosition_camera );
		out_data.o_vertexColor = in_data.i_vertexColor;
	}
}

#elif defined( EAE6320_PLATFORM_GL )

// Constant Buffers
//=================

layout( std140, binding = 0 ) uniform g_constantBuffer_frame
{
	mat4 g_transform_worldToCamera;
	mat4 g_transform_cameraToProjected;

	float g_elapsedSecondCount_systemTime;
	float g_elapsedSecondCount_simulationTime;
	// For vec4 alignment
	vec2 g_padding;
};

// Input
//======

// The locations assigned are arbitrary
// but must match the C calls to glVertexAttribPointer()

// These values come from one of the VertexFormats::s3dObject that the vertex buffer was filled with in C code
layout( location = 0 ) in vec3 i_vertexPosition_local;
layout( location = 1 ) in vec4 i_vertexColor;
out vec4 vertexColor;

layout( std140, binding = 2 ) uniform g_constantBuffer_drawCall
{
  mat4 g_transform_localToWorld;
};

// Output
//=======

// The vertex shader must always output a position value,
// but unlike HLSL where the value is explicit
// GLSL has an automatically-required variable named "gl_Position"

// Entry Point
//============

void main()
{
	// Transform the local vertex into world space
	vec4 vertexPosition_world;
	{
		// This will be done in a future assignment.
		// For now, however, local space is treated as if it is the same as world space.
		vec4 vertexPosition_local = vec4( i_vertexPosition_local, 1.0 );
		vertexPosition_world = g_transform_localToWorld * vertexPosition_local;
	}
	// Calculate the position of this vertex projected onto the display
	{
		// Transform the vertex from world space into camera space
		vec4 vertexPosition_camera = g_transform_worldToCamera * vertexPosition_world;
		// Project the vertex from camera space into projected space
		gl_Position = g_transform_cameraToProjected * vertexPosition_camera;
		vertexColor = i_vertexColor;
	}
}

#endif
