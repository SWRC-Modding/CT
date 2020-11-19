#include "OpenGLResource.h"

#include "../Inc/OpenGLRenderDevice.h"

FOpenGLResource::FOpenGLResource(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : RenDev(InRenDev),
                                                                                   CacheId(InCacheId),
                                                                                   HashIndex(INDEX_NONE),
                                                                                   HashNext(NULL){
	RenDev->AddResource(this);
}

FOpenGLResource::~FOpenGLResource(){
	RenDev->RemoveResource(this);
}

// FOpenGLShader

FOpenGLShader::FOpenGLShader(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, EOpenGLShaderType InType) : FOpenGLResource(InRenDev, InCacheId),
                                                                                                         Type(InType),
                                                                                                         Handle(GL_NONE){}

FOpenGLShader::~FOpenGLShader(){
	if(Handle)
		glDeleteShader(Handle);
}

void FOpenGLShader::Cache(const TCHAR* Source){
	if(!Handle)
		Handle = glCreateShader(Type == OST_Vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);

	glShaderSource(Handle, 1, &Source, NULL);
	glCompileShader(Handle);

	GLint Status;

	glGetShaderiv(Handle, GL_COMPILE_STATUS, &Status);

	if(!Status){
		GLchar Buffer[512];

		glGetShaderInfoLog(Handle, ARRAY_COUNT(Buffer), NULL, Buffer);
		appErrorf("%s shader compilation failed: %s", Type == OST_Vertex ? "Vertex" : "Fragment", Buffer);
	}
}

// FOpenGLShaderProgram

FOpenGLShaderProgram::FOpenGLShaderProgram(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : FOpenGLResource(InRenDev, InCacheId),
                                                                                             Handle(GL_NONE),
                                                                                             VertexShader(NULL),
                                                                                             FragmentShader(NULL){}

FOpenGLShaderProgram::~FOpenGLShaderProgram(){
	if(Handle)
		glDeleteProgram(Handle);
}

void FOpenGLShaderProgram::Cache(FOpenGLShader* NewVertexShader, FOpenGLShader* NewFragmentShader){
	checkSlow(NewVertexShader->Type == OST_Vertex);
	checkSlow(NewFragmentShader->Type == OST_Fragment);

	FragmentShader = NewFragmentShader;
	VertexShader = NewVertexShader;

	if(!Handle)
		Handle = glCreateProgram();

	glAttachShader(Handle, VertexShader->Handle);
	glAttachShader(Handle, FragmentShader->Handle);
	glLinkProgram(Handle);
	glDetachShader(Handle, VertexShader->Handle);
	glDetachShader(Handle, FragmentShader->Handle);

	GLint Status;

	glGetProgramiv(Handle, GL_LINK_STATUS, &Status);

	if(!Status){
		GLchar Buffer[512];

		glGetProgramInfoLog(Handle, ARRAY_COUNT(Buffer), NULL, Buffer);
		appErrorf("Shader program linking failed: %s", Buffer);
	}
}

void FOpenGLShaderProgram::Bind() const{
	checkSlow(Handle);
	glUseProgram(Handle);
}
