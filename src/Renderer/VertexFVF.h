#ifndef _VERTEXFVF_H
#define _VERTEXFVF_H

#include "VertexTypes.h"

template<unsigned int _type>
struct FVFCode
{
	enum
	{
		FVF =	 0
				|has_pos<_type>::value*D3DFVF_XYZ
				|has_diffuse<_type>::value*D3DFVF_DIFFUSE
				|has_normal<_type>::value*D3DFVF_NORMAL
				|has_tangent<_type>::value*D3DFVF_NORMAL
				|has_bitangent<_type>::value*D3DFVF_NORMAL
				|has_uv0<_type>::value*D3DFVF_TEX0
				|has_uv1<_type>::value*D3DFVF_TEX1
				|has_uv2<_type>::value*D3DFVF_TEX2
				|has_uv3<_type>::value*D3DFVF_TEX3
	};
};

#endif