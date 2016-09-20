#if !defined(_CURSOR_H)
#define _CURSOR_H

struct VertexBuffer;

class Cursor
{
private:

	LPDIRECT3DTEXTURE9	m_texture;
	unsigned int m_width;
	unsigned int m_height;
	unsigned int m_x;
	unsigned int m_y;
	bool m_enable;

	struct _VS
	{
		D3DXMACRO*						m_macros;
		LPD3DXBUFFER					m_CompilatedShader;
		LPD3DXCONSTANTTABLE				m_ShaderConstantList;
		LPDIRECT3DVERTEXSHADER9			m_Shader;
		_VS()
		{
			m_macros = NULL;
			m_CompilatedShader = NULL;
			m_ShaderConstantList = NULL;
			m_Shader = NULL;
		}
	}VS;
	struct _PS
	{
		D3DXMACRO*						m_macros;
		LPD3DXBUFFER					m_CompilatedShader;
		LPD3DXCONSTANTTABLE				m_ShaderConstantList;
		LPDIRECT3DPIXELSHADER9			m_Shader;
		_PS()
		{
			m_macros = NULL;
			m_CompilatedShader = NULL;
			m_ShaderConstantList = NULL;
			m_Shader = NULL;
		}
	}PS;

public:

	Cursor();
	bool Init(unsigned int width,unsigned int height);
	void SetCursorPos(unsigned int x,unsigned int y)
	{
		m_x = x;
		m_y = y;
	}

	bool Render();
	void SetEnable(bool enable)
	{
		m_enable = enable;
	}
};

#endif