#if !defined(_BOUNDINGBOX_H)
#define _BOUNDINGBOX_H

#include "Vector3.h"

class AABBOX
{
public:
	float	m_corners[6];

public:
	void SetOrigin(Vector3& origin)
	{
		m_corners[0] = origin.x;
		m_corners[1] = origin.x;
		m_corners[2] = origin.y;
		m_corners[3] = origin.y;
		m_corners[4] = origin.z;
		m_corners[5] = origin.z;
	}

	void Extend(Vector3& v)
	{
		if(v.x<m_corners[0])
			m_corners[0]=v.x;
		if(v.x>m_corners[1])
			m_corners[1]=v.x;

		if(v.y<m_corners[2])
			m_corners[2]=v.y;
		if(v.y>m_corners[3])
			m_corners[3]=v.y;

		if(v.z<m_corners[4])
			m_corners[4]=v.z;
		if(v.z>m_corners[5])
			m_corners[5]=v.z;
	}
};

#endif