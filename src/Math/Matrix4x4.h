#ifndef _MATRIX4x4_H
#define _MATRIX4x4_H

#include "Vector3.h"

struct MATRIX4X4
{
	union
	{
		struct
		{
			float _11,_12,_13,_14;
			float _21,_22,_23,_24;
			float _31,_32,_33,_34;
			float _41,_42,_43,_44;
		};
		float m[4][4];
		float m16[16];
	};

	MATRIX4X4(){};

	void SetIdentity()
	{
		for(unsigned int i=0;i<16;i++)
			m16[i]=0.0f;

		_11=1.0f;
		_22=1.0f;
		_33=1.0f;
		_44=1.0f;
	}

	void SetView(Vector3& Pos,Vector3& Fw,Vector3& Up)
	{
		Vector3 zaxis = Fw.Normalize();
		Vector3 xaxis = Up^zaxis;
		xaxis.Normalize();
		Vector3 yaxis = zaxis^xaxis;

		_11 = xaxis.x;		_12 = yaxis.x;		_13 = zaxis.x;		_14 = 0.0f;
		_21 = xaxis.y;		_22 = yaxis.y;		_23 = zaxis.y;		_24 = 0.0f;
		_31 = xaxis.z;		_32 = yaxis.z;		_33 = zaxis.z;		_34 = 0.0f;
		_41	= -(Pos*xaxis);	_42 = -(Pos*yaxis);	_43 = -(Pos*zaxis);	_44 = 1.0f;
	}

	Vector3 Transform(Vector3& v)
	{
		Vector3 res;
		res.x = v.x*_11 + v.y*_21 + v.z*_31 + _41;
		res.y = v.x*_12 + v.y*_22 + v.z*_32 + _42;
		res.z = v.x*_13 + v.y*_23 + v.z*_33 + _43;
		return res;
	}

	Vector3 GetFw(){return Vector3(_13,_23,_33);}
	Vector3 GetUp(){return Vector3(_12,_22,_32);}
	Vector3 GetRight(){return Vector3(_11,_21,_31);}
};

#endif