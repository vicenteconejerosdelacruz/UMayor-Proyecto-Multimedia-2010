#ifndef _VECTOR3_H
#define _VECTOR3_H

#include "Math.h"

struct Vector3
{
	union
	{
		struct
		{
			float x,y,z;
		};
		float m[3];
	};

	Vector3(){};

	Vector3(float cx,float cy,float cz)
	{
		x=cx;y=cy;z=cz;
	}

	Vector3(const Vector3& other)
	{
		x=other.x;y=other.y;z=other.z;
	}

	Vector3(float* vec)
	{
		m[0]=vec[0];
		m[1]=vec[1];
		m[2]=vec[2];
	}

	void Set(float cx,float cy,float cz)
	{
		x=cx;y=cy;z=cz;
	}


	void operator=(const Vector3& other)
	{
		x=other.x;y=other.y;z=other.z;
	}

	void operator+=(const Vector3& other)
	{
		x+=other.x;y+=other.y;z+=other.z;
	}

	void operator-=(const Vector3& other)
	{
		x-=other.x;y-=other.y;z-=other.z;
	}

	Vector3 operator+(const Vector3& v2)
	{
		return Vector3(x+v2.x,y+v2.y,z+v2.z);
	}

	Vector3 operator-(const Vector3& v2)
	{
		return Vector3(x-v2.x,y-v2.y,z-v2.z);
	}

	Vector3 operator^(const Vector3& v)
	{
		return Vector3(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);
	}

	float operator*(const Vector3& v)
	{
		return v.x*x+v.y*y+v.z*z;
	}

	Vector3 operator*(const float scale)
	{
		return Vector3(x*scale,y*scale,z*scale);
	}

	Vector3 Normalize()
	{
		float len=sqrt(x*x+y*y+z*z);
		x/=len;
		y/=len;
		z/=len;
		return *this;
	}

	void RotateAroundAxis(const Vector3& axis,float RadAngle)
	{
		//Rotate the point (x,y,z) around the vector (u,v,w) Function RotatePointAroundVector(x#,y#,z#,u#,v#,w#,a#)
		float ux=axis.x*x;
		float uy=axis.x*y;
		float uz=axis.x*z;
		float vx=axis.y*x;
		float vy=axis.y*y;
		float vz=axis.y*z;
		float wx=axis.z*x;
		float wy=axis.z*y;
		float wz=axis.z*z;
		float sa=sin(RadAngle);
		float ca=cos(RadAngle);
		x=axis.x*(ux+vy+wz)+(x*(axis.y*axis.y+axis.z*axis.z)-axis.x*(vy+wz))*ca+(-wy+vz)*sa;
		y=axis.y*(ux+vy+wz)+(y*(axis.x*axis.x+axis.z*axis.z)-axis.y*(ux+wz))*ca+(wx-uz)*sa;
		z=axis.z*(ux+vy+wz)+(z*(axis.x*axis.x+axis.y*axis.y)-axis.z*(ux+vy))*ca+(-vx+uy)*sa;
	}

	static Vector3 Up;
	static Vector3 Fw;
	static Vector3 xAxis;
	static Vector3 yAxis;
	static Vector3 zAxis;
};

#endif