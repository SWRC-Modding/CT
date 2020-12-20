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

FOpenGLShader::FOpenGLShader(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : FOpenGLResource(InRenDev, InCacheId),
                                                                                               Program(GL_NONE),
                                                                                               NumVertexShaderSubroutines(0),
                                                                                               NumFragmentShaderSubroutines(0){}

FOpenGLShader::~FOpenGLShader(){
	if(Program)
		glDeleteProgram(Program);
}

void FOpenGLShader::Cache(FShaderGLSL* Shader){
	GLuint VertexShader = CompileShader(Shader, GL_VERTEX_SHADER);
	GLuint FragmentShader = CompileShader(Shader, GL_FRAGMENT_SHADER);

	// Set revision even if compilation is unsuccessful to avoid recompiling the invalid shader each time it is set
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

		glGetProgramStageiv(Program, GL_VERTEX_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS, &NumVertexShaderSubroutines);
		glGetProgramStageiv(Program, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS, &NumFragmentShaderSubroutines);

		check(NumVertexShaderSubroutines < MAX_SHADER_SUBROUTINES);
		check(NumFragmentShaderSubroutines < MAX_SHADER_SUBROUTINES);
	}
}

void FOpenGLShader::Bind() const{
	checkSlow(Program);
	glUseProgram(Program);
}

void FOpenGLShader::UpdateSubroutines() const{
	if(NumVertexShaderSubroutines > 0)
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, NumVertexShaderSubroutines, VertexShaderSubroutines);

	if(NumFragmentShaderSubroutines > 0)
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, NumFragmentShaderSubroutines, FragmentShaderSubroutines);
}

void FOpenGLShader::SetUniformInt(GLuint Index, GLint Value) const{
	glProgramUniform1i(Program, Index, Value);
}

void FOpenGLShader::SetUniformFloat(GLuint Index, GLfloat Value) const{
	glProgramUniform1f(Program, Index, Value);
}

void FOpenGLShader::SetUniformVec2(GLuint Index, const GLSL_vec2& Value) const{
	glProgramUniform2f(Program, Index, Value.X, Value.Y);
}

void FOpenGLShader::SetUniformVec3(GLuint Index, const GLSL_vec3& Value) const{
	glProgramUniform3f(Program, Index, Value.X, Value.Y, Value.Z);
}

void FOpenGLShader::SetUniformVec4(GLuint Index, const GLSL_vec4& Value) const{
	glProgramUniform4f(Program, Index, Value.X, Value.Y, Value.Z, Value.W);
}

GLuint FOpenGLShader::CompileShader(FShaderGLSL* Shader, GLenum Type){
	GLuint Handle = glCreateShader(Type);
	const TCHAR* ShaderText = NULL;
	const TCHAR* FileExt = NULL;

	if(Type == GL_VERTEX_SHADER){
		ShaderText = Shader->GetVertexShaderText();
		FileExt = VERTEX_SHADER_FILE_EXTENSION;
	}else if(Type == GL_FRAGMENT_SHADER){
		ShaderText = Shader->GetFragmentShaderText();
		FileExt = FRAGMENT_SHADER_FILE_EXTENSION;
	}else{
		appErrorf("Unsupported shader type (%i)", Type);
	}

	glShaderSource(Handle, 1, &ShaderText, NULL);
	glCompileShader(Handle);

	GLint Status;

	glGetShaderiv(Handle, GL_COMPILE_STATUS, &Status);

	if(!Status){
		GLchar Buffer[512];

		glGetShaderInfoLog(Handle, ARRAY_COUNT(Buffer), NULL, Buffer);
		debugf("Shader compilation failed for %s%s: %s", Shader->GetName(), FileExt, Buffer);
		glDeleteShader(Handle);
		Handle = GL_NONE;
	}

	return Handle;
}

// FOpenGLRenderTarget

FOpenGLRenderTarget::FOpenGLRenderTarget(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : FOpenGLResource(InRenDev, InCacheId),
                                                                                           Width(0),
                                                                                           Height(0),
                                                                                           FBO(GL_NONE),
                                                                                           ColorAttachment(GL_NONE){}

FOpenGLRenderTarget::~FOpenGLRenderTarget(){
	Free();
}

void FOpenGLRenderTarget::Cache(FRenderTarget* RenderTarget){
	Free();

	Width = RenderTarget->GetWidth();
	Height = RenderTarget->GetHeight();

	if(Width == 0 || Height == 0)
		return;

	glCreateFramebuffers(1, &FBO);
	glCreateTextures(GL_TEXTURE_2D, 1, &ColorAttachment);
	glTextureStorage2D(ColorAttachment, 1, RenDev->Use16bit ? GL_RGB565 : GL_RGB8, Width, Height);
	glTextureParameteri(ColorAttachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(ColorAttachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(ColorAttachment, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(ColorAttachment, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0, ColorAttachment, 0);
	glCreateRenderbuffers(1, &DepthStencilAttachment);
	glNamedRenderbufferStorage(DepthStencilAttachment, GL_DEPTH24_STENCIL8, Width, Height);
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
