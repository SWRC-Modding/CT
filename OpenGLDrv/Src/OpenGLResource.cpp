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

static const GLchar* GlobalUniforms = "layout(std140, binding = 0) uniform Globals{\n"
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

static const GLchar* VertexShaderVariables = "layout(location = 0)  in vec3 InPosition;\n"
                                             "layout(location = 1)  in vec3 InNormal;\n"
                                             "layout(location = 2)  in vec4 InDiffuse;\n"
                                             "layout(location = 3)  in vec4 InSpecular;\n"
                                             "layout(location = 4)  in vec4 InTexCoord0;\n"
                                             "layout(location = 5)  in vec4 InTexCoord1;\n"
                                             "layout(location = 6)  in vec4 InTexCoord2;\n"
                                             "layout(location = 7)  in vec4 InTexCoord3;\n"
                                             "layout(location = 8)  in vec4 InTexCoord4;\n"
                                             "layout(location = 9)  in vec4 InTexCoord5;\n"
                                             "layout(location = 10) in vec4 InTexCoord6;\n"
                                             "layout(location = 11) in vec4 InTexCoord7;\n"
                                             "layout(location = 12) in vec3 InTangent;\n"
                                             "layout(location = 13) in vec3 InBinormal;\n"
                                             "out vec3 Position;\n"
                                             "out vec3 Normal;\n"
                                             "out vec4 Diffuse;\n"
                                             "out vec4 Specular;\n"
                                             "out vec4 TexCoord0;\n"
                                             "out vec4 TexCoord1;\n"
                                             "out vec4 TexCoord2;\n"
                                             "out vec4 TexCoord3;\n"
                                             "out vec4 TexCoord4;\n"
                                             "out vec4 TexCoord5;\n"
                                             "out vec4 TexCoord6;\n"
                                             "out vec4 TexCoord7;\n"
                                             "out vec3 Tangent;\n"
                                             "out vec3 Binormal;\n\n";

static const GLchar* FragmentShaderVariables = "in vec3 Position;\n"
                                               "in vec3 Normal;\n"
                                               "in vec4 Diffuse;\n"
                                               "in vec4 Specular;\n"
                                               "in vec4 TexCoord0;\n"
                                               "in vec4 TexCoord1;\n"
                                               "in vec4 TexCoord2;\n"
                                               "in vec4 TexCoord3;\n"
                                               "in vec4 TexCoord4;\n"
                                               "in vec4 TexCoord5;\n"
                                               "in vec4 TexCoord6;\n"
                                               "in vec4 TexCoord7;\n"
                                               "in vec3 Tangent;\n"
                                               "in vec3 Binormal;\n"
                                               "out vec4 FragColor;\n\n";

FOpenGLShader::FOpenGLShader(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : FOpenGLResource(InRenDev, InCacheId),
                                                                                               Program(GL_NONE){}

FOpenGLShader::~FOpenGLShader(){
	if(Program)
		glDeleteProgram(Program);
}

void FOpenGLShader::Cache(FShaderGLSL* Shader){
	GLuint VertexShader = CompileShader(Shader, GL_VERTEX_SHADER);
	GLuint FragmentShader = CompileShader(Shader, GL_FRAGMENT_SHADER);

	// Set revision even if compilation if unsuccessful to avoid recompiling the invalid shader each time it is set
	Revision = Shader->GetRevision();

	if(!VertexShader || !FragmentShader){
		if(VertexShader)
			glDeleteShader(VertexShader);

		if(FragmentShader)
			glDeleteShader(FragmentShader);

		return;
	}

	GLuint NewProgram = glCreateProgram();

	glAttachShader(NewProgram, VertexShader);
	glAttachShader(NewProgram, FragmentShader);
	glLinkProgram(NewProgram);
	glDetachShader(NewProgram, VertexShader);
	glDetachShader(NewProgram, FragmentShader);
	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	GLint Status;

	glGetProgramiv(NewProgram, GL_LINK_STATUS, &Status);

	if(!Status){
		GLchar Buffer[512];

		glGetProgramInfoLog(NewProgram, ARRAY_COUNT(Buffer), NULL, Buffer);
		debugf("Shader program linking failed for %s: %s", Shader->GetName(), Buffer);
		glDeleteProgram(NewProgram);
	}else{
		if(Program)
			glDeleteProgram(Program);

		Program = NewProgram;
	}
}

void FOpenGLShader::Bind() const{
	checkSlow(Program);
	glUseProgram(Program);
}

GLuint FOpenGLShader::CompileShader(FShaderGLSL* Shader, GLenum Type){
	GLuint Handle = glCreateShader(Type);
	const TCHAR* Variables = NULL;
	FString EntryPoint;
	FString Main;
	const TCHAR* Text = NULL;
	const TCHAR* Ext = NULL;

	if(Type == GL_VERTEX_SHADER){
		Variables = VertexShaderVariables;
		EntryPoint = Shader->GetVertexShaderEntryPointName();
		Main = Shader->GetVertexShaderMain();
		Text = Shader->GetVertexShaderText();
		Ext = VERTEX_SHADER_EXTENSION;
	}else if(Type == GL_FRAGMENT_SHADER){
		Variables = FragmentShaderVariables;
		EntryPoint = Shader->GetFragmentShaderEntryPointName();
		Main = Shader->GetFragmentShaderMain();
		Text = Shader->GetFragmentShaderText();
		Ext = FRAGMENT_SHADER_EXTENSION;
	}else{
		appErrorf("Unsupported shader type (%i)", Type);
	}

	const GLchar* CombinedSource[] = {
		"#version 450\n\n",
		GlobalUniforms,
		Variables,
		*(FStringTemp("vec4 ", true) + EntryPoint + "();\n\n"), // Forward declaration so that default main can call the shader specific main
		*Main,
		"#line 1\n",
		Text
	};

	glShaderSource(Handle, ARRAY_COUNT(CombinedSource), CombinedSource, NULL);
	glCompileShader(Handle);

	GLint Status;

	glGetShaderiv(Handle, GL_COMPILE_STATUS, &Status);

	if(!Status){
		GLchar Buffer[512];

		glGetShaderInfoLog(Handle, ARRAY_COUNT(Buffer), NULL, Buffer);
		debugf("Shader compilation failed for %s%s: %s", Shader->GetName(), Ext, Buffer);
		glDeleteShader(Handle);
		Handle = GL_NONE;
	}

	return Handle;
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

	Revision = RenderTarget->GetRevision();
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
	void* Data = RenDev->GetScratchBuffer(NewBufferSize);

	IndexBuffer->GetContents(Data);

	if(IsDynamic){
		if(BufferSize < NewBufferSize){
			BufferSize = NewBufferSize * 2;
			glNamedBufferData(EBO, BufferSize, NULL, GL_DYNAMIC_DRAW);
		}

		glNamedBufferSubData(EBO, 0, NewBufferSize, Data);
	}else{
		BufferSize = NewBufferSize;
		glNamedBufferStorage(EBO, NewBufferSize, Data, 0);
	}

	IndexSize = IndexBuffer->GetIndexSize();
	Revision = IndexBuffer->GetRevision();
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
		IsDynamic = VertexStream->HintDynamic() != 0;
	}

	if(!VBO)
		glCreateBuffers(1, &VBO);

	INT NewBufferSize = VertexStream->GetSize();
	void* Data = RenDev->GetScratchBuffer(NewBufferSize);

	VertexStream->GetStreamData(Data);

	if(IsDynamic){
		if(BufferSize < NewBufferSize){
			BufferSize = NewBufferSize * 2;
			glNamedBufferData(VBO, BufferSize, NULL, GL_DYNAMIC_DRAW);
		}

		glNamedBufferSubData(VBO, 0, NewBufferSize, Data);
	}else{
		BufferSize = NewBufferSize;
		glNamedBufferStorage(VBO, BufferSize, Data, 0);
	}

	Stride = VertexStream->GetStride();
	Revision = VertexStream->GetRevision();
}

void FOpenGLVertexStream::Free(){
	if(VBO){
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}
}

void FOpenGLVertexStream::Bind(GLuint BindingIndex) const{
	checkSlow(VBO);
	glBindVertexBuffer(BindingIndex, VBO, 0, Stride);
}

// FOpenGLTexture

FOpenGLTexture::FOpenGLTexture(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : FOpenGLResource(InRenDev, InCacheId),
                                                                                 Handle(GL_NONE){}

FOpenGLTexture::~FOpenGLTexture(){
	if(Handle)
		glDeleteTextures(1, &Handle);
}

void FOpenGLTexture::Cache(FTexture* Texture){
	if(Handle)
		glDeleteTextures(1, &Handle);

	INT Width = Texture->GetWidth();
	INT Height = Texture->GetHeight();

	if(Width == 0 || Height == 0){
		static FSolidColorTexture ErrorTexture(FColor(255, 0, 255));

		Texture = &ErrorTexture;
		Width = Texture->GetWidth();
		Height = Texture->GetHeight();
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &Handle);

	ETextureFormat SrcFormat = Texture->GetFormat();
	INT Size = GetBytesPerPixel(SrcFormat, Width * Height);

	if(IsDXTC(SrcFormat)){
		GLenum GLFormat;

		if(SrcFormat == TEXF_DXT1)
			GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		else if(SrcFormat == TEXF_DXT3)
			GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		else
			GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

		void* Data = Texture->GetRawTextureData(0);

		checkSlow(Data);

		glCompressedTextureImage2DEXT(Handle, GL_TEXTURE_2D, 0, GLFormat, Width, Height, 0, Size, Data);
		Texture->UnloadRawTextureData(0);
	}else{
		void* Data = Texture->GetRawTextureData(0);

		if(!Data){
			Data = RenDev->GetScratchBuffer(Size);
			Texture->GetTextureData(0, Data, 0, SrcFormat);
		}

		glTextureStorage2D(Handle, 1, GL_RGBA8, Width, Height);
		glTextureSubImage2D(Handle, 0, 0, 0, Width, Height, GL_BGRA, GL_UNSIGNED_BYTE, Data);
		Texture->UnloadRawTextureData(0);
	}

	glGenerateTextureMipmap(Handle);
	glTextureParameteri(Handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(Handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	Revision = Texture->GetRevision();
}

void FOpenGLTexture::Bind(GLuint TextureUnit){
	checkSlow(Handle);
	glBindTextureUnit(TextureUnit, Handle);
}
