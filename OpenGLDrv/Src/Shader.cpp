#include "Shader.h"
#include "OpenGLRenderDevice.h"

// FOpenGLShader

FOpenGLShader::FOpenGLShader(UOpenGLRenderDevice* InRenDev) : RenDev(InRenDev),
                                                              Program(GL_NONE){}

FOpenGLShader::~FOpenGLShader(){
	Free();
}

void FOpenGLShader::Compile(const TCHAR* InShaderCode, const TCHAR* ShaderName){
	FString ShaderCode = InShaderCode;

	RenDev->ExpandShaderMacros(&ShaderCode);

	GLuint VertexShader = CompileShader(GL_VERTEX_SHADER, *ShaderCode, *(FStringTemp(ShaderName) + SHADER_FILE_EXTENSION + " - vertex shader"));
	GLuint FragmentShader = CompileShader(GL_FRAGMENT_SHADER, *ShaderCode, *(FStringTemp(ShaderName) + SHADER_FILE_EXTENSION + " - fragment shader"));

	if(!VertexShader || !FragmentShader){
		if(VertexShader)
			RenDev->glDeleteShader(VertexShader);

		if(FragmentShader)
			RenDev->glDeleteShader(FragmentShader);

		if(Program){
			RenDev->glDeleteProgram(Program);
			Program = GL_NONE;
		}

		return;
	}

	GLuint NewProgram = RenDev->glCreateProgram();

	RenDev->glAttachShader(NewProgram, VertexShader);
	RenDev->glAttachShader(NewProgram, FragmentShader);
	RenDev->glLinkProgram(NewProgram);
	RenDev->glDetachShader(NewProgram, VertexShader);
	RenDev->glDetachShader(NewProgram, FragmentShader);
	RenDev->glDeleteShader(VertexShader);
	RenDev->glDeleteShader(FragmentShader);

	GLint Status;

	RenDev->glGetProgramiv(NewProgram, GL_LINK_STATUS, &Status);

	if(Status){
		if(Program)
			RenDev->glDeleteProgram(Program);

		Program = NewProgram;
	}else{
		GLchar Buffer[512];

		RenDev->glGetProgramInfoLog(NewProgram, ARRAY_COUNT(Buffer), NULL, Buffer);
		debugf("Shader program linking failed for %s: %s", ShaderName, Buffer);
		RenDev->glDeleteProgram(NewProgram);
	}
}

void FOpenGLShader::Bind() const{
	checkSlow(this != &RenDev->ErrorShader || Program);

	if(Program)
		RenDev->glUseProgram(Program);
	else
		RenDev->ErrorShader.Bind();

}

void FOpenGLShader::Free(){
	if(RenDev->IsCurrent() && Program){
		checkSlow(RenDev->IsCurrent());
		RenDev->glDeleteProgram(Program);
		Program = GL_NONE;
	}
}

GLuint FOpenGLShader::CompileShader(GLenum Type, const TCHAR* ShaderCode, const TCHAR* ShaderName){
	GLuint Handle = RenDev->glCreateShader(Type);
	const TCHAR* ShaderVars = NULL;

	if(Type == GL_VERTEX_SHADER)
		ShaderVars = *RenDev->VertexShaderVarsText;
	else if(Type == GL_FRAGMENT_SHADER)
		ShaderVars = *RenDev->FragmentShaderVarsText;
	else
		appErrorf("Unsupported shader type (%i)", Type);

	const TCHAR* ShaderText[] = {
		ShaderVars,
		"#line 1\n",
		ShaderCode
	};

	RenDev->glShaderSource(Handle, ARRAY_COUNT(ShaderText), ShaderText, NULL);
	RenDev->glCompileShader(Handle);

	GLint Status;

	RenDev->glGetShaderiv(Handle, GL_COMPILE_STATUS, &Status);

	if(!Status){
		GLchar Buffer[512];

		RenDev->glGetShaderInfoLog(Handle, ARRAY_COUNT(Buffer), NULL, Buffer);
		debugf("Shader compilation failed for %s: %s", ShaderName, Buffer);
		RenDev->glDeleteShader(Handle);
		Handle = GL_NONE;
	}

	return Handle;
}
