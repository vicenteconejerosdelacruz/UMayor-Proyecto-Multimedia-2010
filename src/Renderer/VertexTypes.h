#ifndef _VERTEX_TYPES_H_
#define _VERTEX_TYPES_H_

#include<string>

enum VERTEX_ATT
{
	 ATT_POS		= 1<<0
	,ATT_DIFFUSE	= 1<<1
	,ATT_NORMAL		= 1<<2
	,ATT_TANGENT	= 1<<3
	,ATT_BITANGENT	= 1<<4
	,ATT_UV0		= 1<<5
	,ATT_UV1		= 1<<6
	,ATT_UV2		= 1<<7
	,ATT_UV3		= 1<<8
};

template<unsigned int>
struct shader_code
{
	static std::string code; 
};

typedef float float3[3];
typedef float float2[2];
typedef unsigned int uint;

#define DECLARE_VERTEX_MEMBER(_type,_name)	_type	_name;\
											typedef _type _name##_##_type;\

template<int>
struct Vertex
{
};

template<>
struct Vertex<ATT_POS>
{
	DECLARE_VERTEX_MEMBER(float3,pos);
};typedef Vertex<ATT_POS> VertexPos;

template<>
struct Vertex<ATT_POS|ATT_UV0>
{
	DECLARE_VERTEX_MEMBER(float3,pos);
	DECLARE_VERTEX_MEMBER(float2,uv0);
};typedef Vertex<ATT_POS|ATT_UV0> VertexPosUv;

template<>
struct Vertex<ATT_POS|ATT_DIFFUSE>
{
	DECLARE_VERTEX_MEMBER(float3,pos);
	DECLARE_VERTEX_MEMBER(uint,diffuse);
};typedef Vertex<ATT_POS|ATT_DIFFUSE> VertexPosDiffuse;

template<>
struct Vertex<ATT_POS|ATT_NORMAL>
{
	DECLARE_VERTEX_MEMBER(float3,pos);
	DECLARE_VERTEX_MEMBER(float3,normal);
};typedef Vertex<ATT_POS|ATT_NORMAL> VertexPosNormal;

template<>
struct Vertex<ATT_POS|ATT_NORMAL|ATT_DIFFUSE>
{
	DECLARE_VERTEX_MEMBER(float3,pos);
	DECLARE_VERTEX_MEMBER(float3,normal);
	DECLARE_VERTEX_MEMBER(uint,diffuse);
};typedef Vertex<ATT_POS|ATT_NORMAL|ATT_DIFFUSE> VertexPosNormalDiffuse;

template<>
struct Vertex<ATT_POS|ATT_NORMAL|ATT_UV0>
{
	DECLARE_VERTEX_MEMBER(float3,pos);
	DECLARE_VERTEX_MEMBER(float3,normal);
	DECLARE_VERTEX_MEMBER(float2,uv0);
};typedef Vertex<ATT_POS|ATT_NORMAL|ATT_UV0> VertexPosNormalUv;

template<>
struct Vertex<ATT_POS|ATT_NORMAL|ATT_TANGENT|ATT_BITANGENT|ATT_UV0>
{
	DECLARE_VERTEX_MEMBER(float3,pos);
	DECLARE_VERTEX_MEMBER(float3,normal);
	DECLARE_VERTEX_MEMBER(float3,tangent);
	DECLARE_VERTEX_MEMBER(float3,bitangent);
	DECLARE_VERTEX_MEMBER(float2,uv0);
};typedef Vertex<ATT_POS|ATT_NORMAL|ATT_TANGENT|ATT_BITANGENT|ATT_UV0> VertexPosNormalTangentBitangentUv;

#define VERTEX_MEMBER(_type,_name)\
template<unsigned int T>\
struct has_##_name\
{\
	template<typename U>\
	static char(&test(U*, typename U::_name##_##_type* = 0))[1];\
	static char(&test(...))[2];\
	enum {value = 1 == sizeof(has_##_name::test((Vertex<T>*)0))}; \
};\

VERTEX_MEMBER(float3,pos);
VERTEX_MEMBER(uint,diffuse);
VERTEX_MEMBER(float3,normal);
VERTEX_MEMBER(float3,tangent);
VERTEX_MEMBER(float3,bitangent);
VERTEX_MEMBER(float2,uv0);
VERTEX_MEMBER(float2,uv1);
VERTEX_MEMBER(float2,uv2);
VERTEX_MEMBER(float2,uv3);
#undef VERTEX_MEMBER

#endif