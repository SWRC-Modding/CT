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

static const GLchar* GLSLVersion = "#version 450\n";
static const GLchar* GlobalUniformBlock = "layout(std140, binding = 0) uniform Globals{\n"
#define UNIFORM_BLOCK_MEMBER(type, name)  "\t" #type " " #name ";\n"
                                               UNIFORM_BLOCK_CONTENTS
#undef UNIFORM_BLOCK_MEMBER
                                           "};\n"
                                           "layout(binding = 0) uniform sampler2D Texture0;\n"
                                           "layout(binding = 1) uniform sampler2D Texture1;\n"
                                           "layout(binding = 2) uniform sampler2D Texture2;\n"
                                           "layout(binding = 3) uniform sampler2D Texture3;\n"
                                           "layout(binding = 4) uniform sampler2D Texture4;\n"
                                           "layout(binding = 5) uniform sampler2D Texture5;\n"
                                           "layout(binding = 6) uniform sampler2D Texture6;\n"
                                           "layout(binding = 7) uniform sampler2D Texture7;\n";

static const GLchar* VertexShaderVariables = "layout(location = 0) in vec3 InPosition;\n"
                                             "layout(location = 1) in vec3 InNormal;\n"
                                             "layout(location = 2) in vec2 InTexCoord0;\n"
                                             "layout(location = 3) in vec2 InTexCoord1;\n"
                                             "layout(location = 4) in vec2 InTexCoord2;\n"
                                             "layout(location = 5) in vec2 InTexCoord3;\n"
                                             "layout(location = 6) in vec2 InTexCoord4;\n"
                                             "layout(location = 7) in vec2 InTexCoord5;\n"
                                             "layout(location = 8) in vec2 InTexCoord6;\n"
                                             "layout(location = 9) in vec2 InTexCoord7;\n"
                                             "layout(location = 10) in vec3 InTangent;\n"
                                             "layout(location = 11) in vec3 InBinormal;\n"
                                             "out vec3 Position;\n"
                                             "out vec3 Normal;\n"
                                             "out vec2 TexCoord0;\n"
                                             "out vec2 TexCoord1;\n"
                                             "out vec2 TexCoord2;\n"
                                             "out vec2 TexCoord3;\n"
                                             "out vec2 TexCoord4;\n"
                                             "out vec2 TexCoord5;\n"
                                             "out vec2 TexCoord6;\n"
                                             "out vec2 TexCoord7;\n"
                                             "out vec3 Tangent;\n"
                                             "out vec3 Binormal;\n";

static const GLchar* FragmentShaderVariables = "in vec3 Position;\n"
                                               "in vec3 Normal;\n"
                                               "in vec2 TexCoord0;\n"
                                               "in vec2 TexCoord1;\n"
                                               "in vec2 TexCoord2;\n"
                                               "in vec2 TexCoord3;\n"
                                               "in vec2 TexCoord4;\n"
                                               "in vec2 TexCoord5;\n"
                                               "in vec2 TexCoord6;\n"
                                               "in vec2 TexCoord7;\n"
                                               "in vec3 Tangent;\n"
                                               "in vec3 Binormal;\n"
                                               "out vec4 FragColor;\n";

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

	const GLchar* CombinedSource[] = {
		GLSLVersion,
		GlobalUniformBlock,
		Type == OST_Vertex ? VertexShaderVariables : FragmentShaderVariables,
		Source
	};

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
                                                                                                           BufferSize(0),
                                                                                                           IsDynamic(InIsDynamic){}

FOpenGLIndexBuffer::~FOpenGLIndexBuffer(){
	Free();
}

void FOpenGLIndexBuffer::Cache(FIndexBuffer* IndexBuffer){
	if(!IsDynamic)
		Free();

	if(!EBO)
		glCreateBuffers(1, &EBO);

	INT NewBufferSize = IndexBuffer->GetSize();

	checkSlow(NewBufferSize > 0);

	IndexSize = IndexBuffer->GetIndexSize();

	if(IsDynamic){
		if(BufferSize < NewBufferSize){
			BufferSize = NewBufferSize * 2;
			debugf("Allocating %i byte dynamic %i-bit index buffer", BufferSize, IndexSize * 8);
			glNamedBufferData(EBO, BufferSize, NULL, GL_DYNAMIC_DRAW);
		}
	}else{
		BufferSize = NewBufferSize;
		glNamedBufferStorage(EBO, BufferSize, NULL, GL_MAP_WRITE_BIT);
	}

	void* Data = glMapNamedBuffer(EBO, GL_WRITE_ONLY);
	IndexBuffer->GetContents(Data);
	glUnmapNamedBuffer(EBO);
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

// FOpenGLVertexStream

FOpenGLVertexStream::FOpenGLVertexStream(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, bool InIsDynamic) : FOpenGLResource(InRenDev, InCacheId),
                                                                                                             VBO(GL_NONE),
                                                                                                             Stride(0),
                                                                                                             BufferSize(0),
                                                                                                             IsDynamic(InIsDynamic){}

FOpenGLVertexStream::~FOpenGLVertexStream(){
	Free();
}

void FOpenGLVertexStream::Cache(FVertexStream* VertexStream){
	if(!IsDynamic){
		Free();
		//IsDynamic = VertexStream->HintDynamic() != 0;
	}

	if(!VBO)
		glCreateBuffers(1, &VBO);

	INT NewBufferSize = VertexStream->GetSize();

	checkSlow(NewBufferSize > 0);

	Stride = VertexStream->GetStride();

	if(IsDynamic){
		if(BufferSize < NewBufferSize){
			BufferSize = NewBufferSize * 2;
			debugf("Allocating %i byte dynamic vertex buffer", BufferSize);
			glNamedBufferData(VBO, BufferSize, NULL, GL_DYNAMIC_DRAW);
		}
	}else{
		BufferSize = NewBufferSize;
		glNamedBufferStorage(VBO, BufferSize, NULL, GL_MAP_WRITE_BIT);
	}

	void* Data = glMapNamedBuffer(VBO, GL_WRITE_ONLY);
	VertexStream->GetStreamData(Data);
	glUnmapNamedBuffer(VBO);
}

void FOpenGLVertexStream::Free(){
	if(VBO){
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}
}

void FOpenGLVertexStream::Bind(unsigned int Index) const{
	checkSlow(VBO);
	glBindVertexBuffer(Index, VBO, 0, Stride);
}
