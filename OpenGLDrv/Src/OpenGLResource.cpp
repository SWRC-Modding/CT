#include "OpenGLResource.h"

#include "../Inc/OpenGLRenderDevice.h"

FOpenGLResource::FOpenGLResource(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : RenDev(InRenDev),
                                                                                   CacheId(InCacheId),
                                                                                   Revision(-1),
                                                                                   HashIndex(INDEX_NONE),
                                                                                   HashNext(NULL){
	RenDev->AddResource(this);
}

FOpenGLResource::~FOpenGLResource(){
	RenDev->RemoveResource(this);
}

// FOpenGLShader

static const GLchar* GLSLVersionString = "#version 450\n";
static const GLchar* GLSLGlobalUniforms = "layout(std140, binding = 0) uniform Globals{\n"
#define UNIFORM_BLOCK_MEMBER(type, name) "\t" #type " " #name ";\n"
                                              UNIFORM_BLOCK_CONTENTS
#undef UNIFORM_BLOCK_MEMBER
                                          "};\n";

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

	const GLchar* CombinedSource[] = {GLSLVersionString, GLSLGlobalUniforms, Source};

	glShaderSource(Handle, ARRAY_COUNT(CombinedSource), CombinedSource, NULL);
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

// FOpenGLRenderTarget

FOpenGLRenderTarget::FOpenGLRenderTarget(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : FOpenGLResource(InRenDev, InCacheId),
                                                                                           FBO(GL_NONE),
                                                                                           ColorAttachment(GL_NONE){}

FOpenGLRenderTarget::~FOpenGLRenderTarget(){
	Free();
}

void FOpenGLRenderTarget::Cache(FRenderTarget* RenderTarget){
	Free();

	glCreateFramebuffers(1, &FBO);
	glCreateTextures(GL_TEXTURE_2D, 1, &ColorAttachment);
	glTextureStorage2D(ColorAttachment, 1, RenDev->Use16bit ? GL_RGB565 : GL_RGB8, RenderTarget->GetWidth(), RenderTarget->GetHeight());
	glTextureParameteri(ColorAttachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(ColorAttachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0, ColorAttachment, 0);
	glCreateRenderbuffers(1, &DepthStencilAttachment);
	glNamedRenderbufferStorage(DepthStencilAttachment, GL_DEPTH24_STENCIL8, RenderTarget->GetWidth(), RenderTarget->GetHeight());
	glNamedFramebufferRenderbuffer(FBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, DepthStencilAttachment);

	checkSlow(glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

void FOpenGLRenderTarget::Free(){
	if(FBO){
		glDeleteFramebuffers(1, &FBO);
		FBO = GL_NONE;
	}

	if(ColorAttachment){
		glDeleteTextures(1, &ColorAttachment);
		ColorAttachment = GL_NONE;
	}

	if(DepthStencilAttachment){
		glDeleteRenderbuffers(1, &DepthStencilAttachment);
		DepthStencilAttachment = GL_NONE;
	}
}

void FOpenGLRenderTarget::Bind() const{
	checkSlow(FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

// FOpenGLIndexBuffer

FOpenGLIndexBuffer::FOpenGLIndexBuffer(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, bool InIsDynamic) : FOpenGLResource(InRenDev, InCacheId),
                                                                                                           EBO(GL_NONE),
                                                                                                           IndexSize(0),
                                                                                                           DynamicSize(0),
                                                                                                           IsDynamic(InIsDynamic){}

FOpenGLIndexBuffer::~FOpenGLIndexBuffer(){
	Free();

	// TODO: Find a better place for this
	if(RenDev->RenderInterface.DynamicIndexBuffer16 == this)
		RenDev->RenderInterface.DynamicIndexBuffer16 = NULL;

	if(RenDev->RenderInterface.DynamicIndexBuffer32 == this)
		RenDev->RenderInterface.DynamicIndexBuffer32 = NULL;
}

void FOpenGLIndexBuffer::Cache(FIndexBuffer* IndexBuffer){
	if(!IsDynamic)
		Free();

	if(!EBO)
		glCreateBuffers(1, &EBO);

	IndexSize = IndexBuffer->GetIndexSize();

	INT BufferSize = IndexBuffer->GetSize();
	void* Data = appMalloc(BufferSize);

	IndexBuffer->GetContents(Data);

	if(IsDynamic){
		if(DynamicSize < BufferSize){
			glNamedBufferData(EBO, BufferSize, Data, GL_DYNAMIC_DRAW);
			DynamicSize = BufferSize;
		}else{
			glNamedBufferSubData(EBO, 0, BufferSize, Data);
		}
	}else{
		glNamedBufferStorage(EBO, BufferSize, Data, 0);
	}

	appFree(Data);
}

void FOpenGLIndexBuffer::Free(){
	if(EBO){
		glDeleteBuffers(1, &EBO);
		EBO = 0;
	}
}

void FOpenGLIndexBuffer::Bind() const{
	checkSlow(EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
}
