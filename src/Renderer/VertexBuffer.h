#ifndef _VERTEXBUFFER_H
#define _VERTEXBUFFER_H

#include "VertexTypes.h"
#include "VertexFVF.h"
#include <d3d9.h>

struct VertexDeclaration
{
	D3DVERTEXELEMENT9 *declaration;
	unsigned int nDeclarations;

	VertexDeclaration()
	{
		declaration = NULL;
		nDeclarations = 0;
	}
};

struct VertexBuffer
{
	LPDIRECT3DVERTEXBUFFER9		m_vertexBuffer;
	void*						m_vertexes;
	unsigned int				m_vertexSize;
	unsigned int				m_vertexType;
	unsigned int				m_numVertexes;
	unsigned int				m_nPrimitives;
	D3DPRIMITIVETYPE			primitiveType;
	unsigned int				FVFType;

	VertexBuffer()
	{
		m_vertexBuffer = NULL;
		m_vertexes = NULL;
		m_vertexSize = 0;
		m_vertexType = 0;
		m_numVertexes = 0;
		m_nPrimitives = 0;
		primitiveType = D3DPT_FORCE_DWORD;
		FVFType = 0;
	}

	~VertexBuffer()
	{
		
	}
};

#endif