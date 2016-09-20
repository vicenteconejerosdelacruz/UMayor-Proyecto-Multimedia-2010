#ifndef _FACEBUFFER_H
#define _FACEBUFFER_H


struct FaceBuffer
{
	LPDIRECT3DINDEXBUFFER9		m_faceBuffer;
	unsigned int				m_numIndexes;
	unsigned int				m_indexesPerFace;
	unsigned int*				m_indexes;
	

	FaceBuffer()
	{
		m_faceBuffer = NULL;
		m_numIndexes = 0;
		m_indexesPerFace = 0;
		m_indexes = NULL;
	}

	~FaceBuffer()
	{
		
	}
};

#endif