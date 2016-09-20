#ifndef _DEF_H
#define _DEF_H

#define SAFEDELETE(x) if(x){delete x;}
#define SAFEDELETEARRAY(x) if(x){delete[] x;}
#define SAFENEW(x) new x
#define SAFENEWARRAY(x) new x
#define SAFERELEASE(x) if(x){x->Release();}

#define INCIFNOTNULL(x) if(x)x++

template<typename T>
void LIMITTO(T& value,const T down, const T up)
{
	if(value>up)
		value=up;
	else if(value<down)
		value=down;
}

template<typename T>
bool ISINSIDE(T& value,const T down, const T up)
{
	if(value>=down && value<=up)
		return true;
	return false;
}

#define WIIMOTE_SCREEN_THRESHOLD_X	100
#define WIIMOTE_SCREEN_THRESHOLD_Y	100

#define RESOLUTION_X		g_renderer->GetResolutionX()
#define RESOLUTION_Y		g_renderer->GetResolutionY()

#endif