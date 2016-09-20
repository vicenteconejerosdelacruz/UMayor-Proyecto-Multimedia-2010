#include "stdafx.h"
#include "Model.h"
#include "VBCreator.h"
#include "Renderer.h"

extern char FileDirectory[MAX_PATH];

bool Model::Load(const char* filePath)
{
	const char* extension = filePath + strlen(filePath) - 4;
	char* folder = (char*)extension;
	while(*folder!='\\')
		folder--;
	folder++;

	std::string fileFolder = FileDirectory;
	std::string file = folder;
	fileFolder.append(filePath,folder-filePath);

	SetCurrentDirectory(fileFolder.c_str());

	MATRIX4X4 transformation;
	transformation.SetIdentity();

	unsigned int loadingFlags = aiProcessPreset_TargetRealtime_Quality|aiProcess_ConvertToLeftHanded;

	if(!stricmp(extension,".3ds"))
	{
		transformation._22 = 0;	transformation._23 = -1;
		transformation._32 = -1; transformation._33 = 0;
	}
	else if(!stricmp(extension,".b3d"))
	{
		loadingFlags&=~aiProcess_ConvertToLeftHanded;
		loadingFlags|=aiProcess_FlipUVs;
	}

	m_aiScene = aiImportFile(file.c_str(),loadingFlags);
	if(m_aiScene==NULL || m_aiScene->mNumMeshes<=0)
		return false;
	
	SAFEDELETEARRAY(m_vb);
	SAFEDELETEARRAY(m_fb);

	m_numFb = m_numVb = m_aiScene->mNumMeshes;
	m_vb = SAFENEWARRAY(VertexBuffer[m_numVb]);
	m_fb = SAFENEWARRAY(FaceBuffer[m_numFb]);

	for(unsigned int i=0;i<m_aiScene->mNumMeshes;i++)
	{
		unsigned int vType = 0;
		if(m_aiScene->mMeshes[i]->HasPositions())
			vType|=ATT_POS;
		if(m_aiScene->mMeshes[i]->HasNormals())
			vType|=ATT_NORMAL;
		if(m_aiScene->mMeshes[i]->HasTangentsAndBitangents())
			vType|=ATT_TANGENT|ATT_BITANGENT;
		if(m_aiScene->mMeshes[i]->HasTextureCoords(0))
			vType|=ATT_UV0;
		if(m_aiScene->mMeshes[i]->HasTextureCoords(1))
			vType|=ATT_UV1;
		if(m_aiScene->mMeshes[i]->HasTextureCoords(2))
			vType|=ATT_UV2;
		if(m_aiScene->mMeshes[i]->HasTextureCoords(3))
			vType|=ATT_UV3;

		VertexBuffer* vb = &m_vb[i];
		FaceBuffer* fb = &m_fb[i];

		if(s_VBCreator.find(vType)==s_VBCreator.end())
			continue;

		s_VBCreator[vType](m_aiScene->mMeshes[i],vb,&m_AABBox,i==0,&transformation);

		if(m_aiScene->mMeshes[i]->HasFaces())
		{
			fb->m_numIndexes = m_aiScene->mMeshes[i]->mNumFaces*m_aiScene->mMeshes[i]->mFaces[0].mNumIndices;
			fb->m_indexesPerFace = m_aiScene->mMeshes[i]->mFaces[0].mNumIndices;
			fb->m_indexes = SAFENEWARRAY(unsigned int[fb->m_numIndexes]);

			unsigned int index = 0;
			for(unsigned int j=0;j<m_aiScene->mMeshes[i]->mNumFaces;j++)
			{
				bool dbreak = false;
				for(unsigned int k=0;k<m_aiScene->mMeshes[i]->mFaces[j].mNumIndices;k++)
				{
					fb->m_indexes[index] = m_aiScene->mMeshes[i]->mFaces[j].mIndices[k];
					index++;
					if(index>=fb->m_numIndexes)
					{
						dbreak = true;
						break;
					}
				}
				if(dbreak)
					break;
			}

			HRESULT res;
			void* pdata;
			res = g_d3dDevice->CreateIndexBuffer(sizeof(unsigned int)*fb->m_numIndexes,0,D3DFMT_INDEX32,D3DPOOL_MANAGED,&fb->m_faceBuffer,NULL);
			res = fb->m_faceBuffer->Lock(0,0,&pdata,0);
			memcpy(pdata,fb->m_indexes,sizeof(unsigned int)*fb->m_numIndexes);
			res = fb->m_faceBuffer->Unlock();
		}
	}

	m_textures.clear();

	for(unsigned int i=0;i<m_aiScene->mNumMaterials;i++)
	{
		aiMaterial* mat = m_aiScene->mMaterials[i];
		aiString name;
		mat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE,0),name);
		char* imageName = name.data + name.length - 1;
		while((*imageName)!='\\'&&(*imageName)!='/'&&imageName!=name.data)
		{
			imageName--;
		}
		if(imageName!=name.data)
			imageName++;

		std::string matName(name.data);
		if(name.length)
		{
			char tex[1024];
			memset(tex,0,sizeof(tex));
			strcat(tex,imageName);
			LPDIRECT3DTEXTURE9 texture;
			HRESULT res = 0;
			res = D3DXCreateTextureFromFile(g_d3dDevice,tex,&texture);
			if(res==S_OK)
			{
				m_textures[matName]=texture;
			}
		}
	}
	SetCurrentDirectory(FileDirectory);
	return true;
}

extern Vector3 g_Pos;
extern Vector3 g_Fw;

void aiColor4ToFloat4(aiColor4D& src,float* dst)
{
	dst[0] = src.r;
	dst[1] = src.g;
	dst[2] = src.b;
	dst[3] = src.a;
}

void expandFloat4(float* data)
{
	data[1] = data[2] = data[3] = data[0];
}

void Model::ApplyMaterial(unsigned int vertexType,aiMaterial* mat)
{
	LPD3DXCONSTANTTABLE vsConstants = s_VertexDeclarations[vertexType].VS.m_ShaderConstantList;
	LPD3DXCONSTANTTABLE psConstants = s_VertexDeclarations[vertexType].PS.m_ShaderConstantList;

	float LightDirection[]={0,1,-1,0};
	float LightAmbientColor[]={0.1f,0.1f,0.1f,1.0f};
	float LightDiffuseColor[]={0.8f,0.8f,0.8f,1.0f};
	float LightSpecularColor[]={0.1f,0.1f,0.1f,1.0f};
	
	if(psConstants->GetConstantByName(NULL,"LightDirection"))
		psConstants->SetVector(g_d3dDevice,"LightDirection",(D3DXVECTOR4*)LightDirection);

	if(psConstants->GetConstantByName(NULL,"LightAmbientColor"))
		psConstants->SetVector(g_d3dDevice,"LightAmbientColor",(D3DXVECTOR4*)LightAmbientColor);

	if(psConstants->GetConstantByName(NULL,"LightDiffuseColor"))
		psConstants->SetVector(g_d3dDevice,"LightDiffuseColor",(D3DXVECTOR4*)LightDiffuseColor);

	if(psConstants->GetConstantByName(NULL,"LightSpecularColor"))
		psConstants->SetVector(g_d3dDevice,"LightSpecularColor",(D3DXVECTOR4*)LightSpecularColor);

	struct	aiColor4D color4;

	float	diffuse[4] = {1.0f,1.0f,1.0f,1.0f};
	if(AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_DIFFUSE,color4))
		aiColor4ToFloat4(color4,diffuse);

	float specularPower[4] = {1.0f,1.0f,1.0f,1.0f};
	if(AI_SUCCESS == mat->Get(AI_MATKEY_SHININESS,specularPower[0]))
		expandFloat4(specularPower);

	float specularScaleFactor[4] = {1.0f,1.0f,1.0f,1.0f};
	if(AI_SUCCESS == mat->Get(AI_MATKEY_SHININESS_STRENGTH,specularScaleFactor[0]))
		expandFloat4(specularScaleFactor);

	if(psConstants->GetConstantByName(NULL,"diffuseLightColor"))
		psConstants->SetVector(g_d3dDevice,"diffuseLightColor",(D3DXVECTOR4*)diffuse);

	if(psConstants->GetConstantByName(NULL,"specularPower"))
		psConstants->SetVector(g_d3dDevice,"specularPower",(D3DXVECTOR4*)specularPower);

	if(psConstants->GetConstantByName(NULL,"specularScaleFactor"))
		psConstants->SetVector(g_d3dDevice,"specularScaleFactor",(D3DXVECTOR4*)specularScaleFactor);


	aiString name;
	mat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE,0),name);
	std::string matName(name.data);
	g_d3dDevice->SetTexture(0,m_textures[matName]);

}

bool Model::Render()
{
	if(!m_vb)
		return false;
		
	//for(unsigned int i=0;i<m_textures.size();i++)
	//{
	//	g_d3dDevice->SetTexture(i,m_textures[i]);
	//}

	for(unsigned int i=0;i<m_numVb;i++)
	{
		VertexBuffer* vb = &m_vb[i];
		FaceBuffer* fb = &m_fb[i];

		HRESULT res;

		res = g_d3dDevice->SetVertexDeclaration(s_VertexDeclarations[vb->m_vertexType].m_vertexDeclaration);
		res = g_d3dDevice->SetVertexShader(s_VertexDeclarations[vb->m_vertexType].VS.m_Shader);
		res = g_d3dDevice->SetPixelShader(s_VertexDeclarations[vb->m_vertexType].PS.m_Shader);
		res = g_d3dDevice->SetStreamSource(0,vb->m_vertexBuffer,0,vb->m_vertexSize);
		
		LPD3DXCONSTANTTABLE constants = s_VertexDeclarations[vb->m_vertexType].VS.m_ShaderConstantList;

		if(constants->GetConstantByName(NULL,"MatWorldViewProj"))
			constants->SetMatrix(g_d3dDevice,"MatWorldViewProj",(D3DXMATRIX*)g_renderer->data.WorldViewProjectionTransformation.m);
		if(constants->GetConstantByName(NULL,"MatWorld"))
			constants->SetMatrix(g_d3dDevice,"MatWorld",(D3DXMATRIX*)g_renderer->data.WorldTransformation.m);
		if(constants->GetConstantByName(NULL,"VecEye"))
			constants->SetVector(g_d3dDevice,"VecEye",(D3DXVECTOR4*)g_Pos.m);

		ApplyMaterial(vb->m_vertexType,m_aiScene->mMaterials[m_aiScene->mMeshes[i]->mMaterialIndex]);
		//ApplyMaterial(s_VertexDeclarations[vb->m_vertexType].VS.m_ShaderConstantList,s_VertexDeclarations[vb->m_vertexType].PS.m_ShaderConstantList,m_aiScene->mMaterials[m_aiScene->mMeshes[i]->mMaterialIndex]);

		if(fb->m_faceBuffer)
		{
			res = g_d3dDevice->SetIndices(fb->m_faceBuffer);
			res = g_d3dDevice->DrawIndexedPrimitive(vb->primitiveType,0,0,vb->m_numVertexes,0,fb->m_numIndexes/fb->m_indexesPerFace);
		}
		else
		{
			g_d3dDevice->SetIndices(NULL);
			g_d3dDevice->DrawPrimitive(vb->primitiveType,0,vb->m_nPrimitives);
		}
	}

	for(unsigned int i=0;i<9;i++)
	{
		g_d3dDevice->SetTexture(i,NULL);
	}

	//RenderAABBox();

	return true;
}

bool Model::RenderAABBox()
{
	static unsigned int AABOXIndex[]=
	{
		 0,1
		,1,2
		,2,3
		,0,3
		,0,4
		,4,7
		,7,3
		,1,5
		,5,6
		,6,2
		,4,5
		,7,6
	};

	static Vertex<ATT_POS|ATT_DIFFUSE> vertexes[8];

	for(unsigned int i=0;i<8;i++)
	{
		vertexes[i].diffuse=0xFFFF0000;
	}

	vertexes[0].pos[0]=m_AABBox.m_corners[0];
	vertexes[1].pos[0]=m_AABBox.m_corners[0];
	vertexes[4].pos[0]=m_AABBox.m_corners[0];
	vertexes[5].pos[0]=m_AABBox.m_corners[0];

	vertexes[2].pos[0]=m_AABBox.m_corners[1];
	vertexes[3].pos[0]=m_AABBox.m_corners[1];
	vertexes[6].pos[0]=m_AABBox.m_corners[1];
	vertexes[7].pos[0]=m_AABBox.m_corners[1];

	vertexes[1].pos[1]=m_AABBox.m_corners[2];
	vertexes[2].pos[1]=m_AABBox.m_corners[2];
	vertexes[5].pos[1]=m_AABBox.m_corners[2];
	vertexes[6].pos[1]=m_AABBox.m_corners[2];

	vertexes[0].pos[1]=m_AABBox.m_corners[3];
	vertexes[3].pos[1]=m_AABBox.m_corners[3];
	vertexes[4].pos[1]=m_AABBox.m_corners[3];
	vertexes[7].pos[1]=m_AABBox.m_corners[3];

	vertexes[0].pos[2]=m_AABBox.m_corners[4];
	vertexes[1].pos[2]=m_AABBox.m_corners[4];
	vertexes[2].pos[2]=m_AABBox.m_corners[4];
	vertexes[3].pos[2]=m_AABBox.m_corners[4];

	vertexes[4].pos[2]=m_AABBox.m_corners[5];
	vertexes[5].pos[2]=m_AABBox.m_corners[5];
	vertexes[6].pos[2]=m_AABBox.m_corners[5];
	vertexes[7].pos[2]=m_AABBox.m_corners[5];

	HRESULT res;

	res = g_d3dDevice->SetVertexDeclaration(s_VertexDeclarations[ATT_POS|ATT_DIFFUSE].m_vertexDeclaration);
	res = g_d3dDevice->SetVertexShader(s_VertexDeclarations[ATT_POS|ATT_DIFFUSE].VS.m_Shader);
	res = g_d3dDevice->SetPixelShader(s_VertexDeclarations[ATT_POS|ATT_DIFFUSE].PS.m_Shader);
	
	LPD3DXCONSTANTTABLE constants = s_VertexDeclarations[ATT_POS|ATT_DIFFUSE].VS.m_ShaderConstantList;

	if(constants->GetConstantByName(NULL,"MatWorldViewProj"))
		constants->SetMatrix(g_d3dDevice,"MatWorldViewProj",(D3DXMATRIX*)g_renderer->data.WorldViewProjectionTransformation.m);

	g_d3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST,0,sizeof(vertexes)/sizeof(vertexes[0]),sizeof(AABOXIndex)/(2*sizeof(AABOXIndex[0])),&AABOXIndex,D3DFMT_INDEX32,&vertexes,sizeof(vertexes[0]));

	return true;
}