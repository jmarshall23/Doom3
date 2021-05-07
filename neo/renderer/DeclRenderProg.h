// DeclRenderProg.h
//

#pragma once

struct glslUniformLocation_t {
	int		parmIndex;
	GLint	uniformIndex;
};

//
// rvmDeclRenderProg
//
class rvmDeclRenderProg : public idDecl {
public:
	virtual size_t			Size(void) const;
	virtual bool			SetDefaultText(void);
	virtual const char* DefaultDefinition(void) const;
	virtual bool			Parse(const char* text, const int textLength);
	virtual void			FreeData(void);

	void					Bind(void);

	void					BindNull(void);
private:
	void					CreateVertexShader(idStr &bracketText);
	void					CreatePixelShader(idStr& bracketText);

	idStr					ParseRenderParms(idStr& bracketText);
private:
	int						LoadGLSLShader(GLenum target, idStr& programGLSL);
	void					LoadGLSLProgram(void);
private:
	idStr					vertexShader;
	int						vertexShaderHandle;

	idStr					pixelShader;
	int						pixelShaderHandle;

	GLuint					program;

	int						tmu;

	idList<rvmDeclRenderParam*> renderParams;
	idList<glslUniformLocation_t> uniformLocations;
};