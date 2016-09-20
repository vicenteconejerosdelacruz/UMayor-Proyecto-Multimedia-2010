#ifndef _VBCREATOR_H
#define _VBCREATOR_H

#include <map>
#include "../Math/Matrix4x4.h"
#include "../Math/Vector3.h"

class AABOX;
typedef void(*creatorFncPointer)(aiMesh* mesh,VertexBuffer* vb,AABBOX* aabbox,bool initAABBox,MATRIX4X4* transformation);
typedef std::map<unsigned int,creatorFncPointer> vbCreatorMap;

template<unsigned int _type1,unsigned int _type2>
struct VertexCopier{ static void copyVertexData(Vertex<_type2>* v,aiMesh* mesh,unsigned int index,MATRIX4X4* transformation=NULL){};};

template<unsigned int _type2>
struct VertexCopier<ATT_POS,_type2>
{
	static void copyVertexData(Vertex<_type2>* v,aiMesh* mesh,unsigned int index,MATRIX4X4* transformation=NULL)
	{
		Vector3 pos = Vector3(mesh->mVertices[index][0],mesh->mVertices[index][1],mesh->mVertices[index][2]);
		if(transformation)
		{
			pos = transformation->Transform(pos);
		}

		v->pos[0] = pos.x;
		v->pos[1] = pos.y;
		v->pos[2] = pos.z;
	}
};

template<unsigned int _type2>
struct VertexCopier<ATT_NORMAL,_type2>
{
	static void copyVertexData(Vertex<_type2>* v,aiMesh* mesh,unsigned int index)
	{
		v->normal[0] = mesh->mNormals[index][0];
		v->normal[1] = mesh->mNormals[index][1];
		v->normal[2] = mesh->mNormals[index][2];
	}
};

template<unsigned int _type2>
struct VertexCopier<ATT_TANGENT,_type2>
{
	static void copyVertexData(Vertex<_type2>* v,aiMesh* mesh,unsigned int index)
	{
		v->tangent[0] = mesh->mTangents[index][0];
		v->tangent[1] = mesh->mTangents[index][1];
		v->tangent[2] = mesh->mTangents[index][2];
	}
};

template<unsigned int _type2>
struct VertexCopier<ATT_BITANGENT,_type2>
{
	static void copyVertexData(Vertex<_type2>* v,aiMesh* mesh,unsigned int index)
	{
		v->bitangent[0] = mesh->mBitangents[index][0];
		v->bitangent[1] = mesh->mBitangents[index][1];
		v->bitangent[2] = mesh->mBitangents[index][2];
	}
};

template<unsigned int _type2>
struct VertexCopier<ATT_UV0,_type2>
{
	static void copyVertexData(Vertex<_type2>* v,aiMesh* mesh,unsigned int index)
	{
		v->uv0[0] = mesh->mTextureCoords[0][index][0];
		v->uv0[1] = mesh->mTextureCoords[0][index][1];
	}
};

template<unsigned int _type>
static void CreateVertexBuffer(aiMesh* mesh,VertexBuffer* vb,AABBOX* aabbox=NULL,bool initAABBox=false,MATRIX4X4* PosTransformation=NULL)
{
	vb->m_vertexes = malloc(sizeof(Vertex<_type>)*mesh->mNumVertices);
	vb->m_numVertexes = mesh->mNumVertices;
	Vertex<_type> *vertex = (Vertex<_type>*)vb->m_vertexes;
	for(unsigned int i=0;i<vb->m_numVertexes;i++)
	{
		VertexCopier<_type&ATT_POS,_type		>::copyVertexData(vertex,mesh,i,PosTransformation);
		VertexCopier<_type&ATT_DIFFUSE,_type	>::copyVertexData(vertex,mesh,i);
		VertexCopier<_type&ATT_NORMAL,_type		>::copyVertexData(vertex,mesh,i);
		VertexCopier<_type&ATT_TANGENT,_type	>::copyVertexData(vertex,mesh,i);
		VertexCopier<_type&ATT_BITANGENT,_type	>::copyVertexData(vertex,mesh,i);
		VertexCopier<_type&ATT_UV0,_type		>::copyVertexData(vertex,mesh,i);
		VertexCopier<_type&ATT_UV1,_type		>::copyVertexData(vertex,mesh,i);		
		VertexCopier<_type&ATT_UV2,_type		>::copyVertexData(vertex,mesh,i);
		VertexCopier<_type&ATT_UV3,_type		>::copyVertexData(vertex,mesh,i);
		if(aabbox)
		{
			Vector3 pos(vertex->pos);
			if(i==0)
			{
				if(initAABBox)
				{
					aabbox->SetOrigin(pos);
				}
			}
			else
			{
				aabbox->Extend(pos);
			}
		}
		vertex++;
	}

	switch(mesh->mPrimitiveTypes)
	{
		case aiPrimitiveType_POINT:
			vb->primitiveType = D3DPT_POINTLIST;
			vb->m_nPrimitives = vb->m_numVertexes;
		break;
		case aiPrimitiveType_LINE:
			vb->primitiveType = D3DPT_LINELIST ;
			vb->m_nPrimitives = vb->m_numVertexes/2;
		break;
		case aiPrimitiveType_TRIANGLE:
			vb->primitiveType = D3DPT_TRIANGLELIST;
			vb->m_nPrimitives = vb->m_numVertexes/3;
		break;
		default:
		break;
	}

	HRESULT res;
	res = g_renderer->data.d3ddev->CreateVertexBuffer(sizeof(Vertex<_type>)*vb->m_numVertexes,0,FVFCode<_type>::FVF,D3DPOOL_MANAGED,&vb->m_vertexBuffer,NULL);
	void* pdata;
	res = vb->m_vertexBuffer->Lock(0,0,&pdata,0);
	memcpy(pdata,vb->m_vertexes,sizeof(Vertex<_type>)*vb->m_numVertexes);
	res = vb->m_vertexBuffer->Unlock();

	vb->FVFType = FVFCode<_type>::FVF;
	vb->m_vertexSize = sizeof(Vertex<_type>);
	vb->m_vertexType = _type;
}

template<unsigned int _type,unsigned int _size>
static void CreateDirectVertexBuffer(VertexBuffer* vb,Vertex<_type> (&vertexes)[_size],D3DPRIMITIVETYPE primitiveType,unsigned int nPrimitives)
{
	vb->m_vertexes = malloc(sizeof(vertexes));
	vb->m_numVertexes = _size;
	memcpy(vb->m_vertexes,vertexes,sizeof(vertexes));
	vb->primitiveType = primitiveType;
	vb->m_nPrimitives = nPrimitives;

	HRESULT res;
	res = g_renderer->data.d3ddev->CreateVertexBuffer(sizeof(Vertex<_type>)*vb->m_numVertexes,0,FVFCode<_type>::FVF,D3DPOOL_MANAGED,&vb->m_vertexBuffer,NULL);
	void* pdata;
	res = vb->m_vertexBuffer->Lock(0,0,&pdata,0);
	memcpy(pdata,vb->m_vertexes,sizeof(vertexes));
	res = vb->m_vertexBuffer->Unlock();

	vb->FVFType = FVFCode<_type>::FVF;
	vb->m_vertexSize = sizeof(Vertex<_type>);
	vb->m_vertexType = _type;

}

extern vbCreatorMap s_VBCreator;

#endif