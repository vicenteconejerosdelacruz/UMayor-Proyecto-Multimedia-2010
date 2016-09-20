#if !defined(_MODEL_H)
#define _MODEL_H

#include "VertexBuffer.h"
#include "FaceBuffer.h"
#include "../Math/AABBOX.h"
#include <assimp.h>
#include <aiPostProcess.h>
#include <aiScene.h>
#include <vector>
#include <map>

class Model
{
private:

	const aiScene* m_aiScene;
	VertexBuffer* m_vb;
	FaceBuffer* m_fb;
	unsigned int m_numVb;
	unsigned int m_numFb;
	std::map<std::string,LPDIRECT3DTEXTURE9>	m_textures;
	AABBOX m_AABBox;

public:

	Model()
	{
		m_aiScene = NULL;
		m_vb = NULL;
		m_fb = NULL;
		m_numVb = 0;
		m_numFb = 0;
	}

	bool Load(const char* filePath);
	bool Render();
	bool RenderAABBox();
	void ApplyMaterial(unsigned int vertexType,aiMaterial* mat);
	AABBOX& getAABBox(){return m_AABBox;}
};

#endif