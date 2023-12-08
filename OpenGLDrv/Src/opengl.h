#pragma once

/*
 * OpenGL function prototypes for version 4.5 and above
 */

#if (defined(_WIN32) || defined(WIN32)) && !(defined(_WIN64) || defined(WIN64))
#define OPENGL_CALL __stdcall
#else
#define OPENGL_CALL
#endif

typedef unsigned int GLenum;
typedef unsigned int GLbitfield;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLboolean;
typedef signed char GLbyte;
typedef short GLshort;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned long GLulong;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
typedef char GLchar;

typedef long GLintptr;
typedef long GLsizeiptr;

#define GL_NONE                                      0
#define GL_ZERO                                      0
#define GL_ONE                                       1
#define GL_FALSE                                     0
#define GL_TRUE                                      1
#define GL_NO_ERROR                                  0
#define GL_NEVER                                     0x0200
#define GL_LESS                                      0x0201
#define GL_EQUAL                                     0x0202
#define GL_LEQUAL                                    0x0203
#define GL_GREATER                                   0x0204
#define GL_NOTEQUAL                                  0x0205
#define GL_GEQUAL                                    0x0206
#define GL_ALWAYS                                    0x0207
#define GL_SRC_COLOR                                 0x0300
#define GL_ONE_MINUS_SRC_COLOR                       0x0301
#define GL_SRC_ALPHA                                 0x0302
#define GL_ONE_MINUS_SRC_ALPHA                       0x0303
#define GL_DST_ALPHA                                 0x0304
#define GL_ONE_MINUS_DST_ALPHA                       0x0305
#define GL_DST_COLOR                                 0x0306
#define GL_ONE_MINUS_DST_COLOR                       0x0307
#define GL_SRC_ALPHA_SATURATE                        0x0308
#define GL_FRONT                                     0x0404
#define GL_BACK                                      0x0405
#define GL_FRONT_AND_BACK                            0x0408
#define GL_CULL_FACE                                 0x0B44
#define GL_DEPTH_TEST                                0x0B71
#define GL_STENCIL_TEST                              0x0B90
#define GL_BLEND                                     0x0BE2
#define GL_TEXTURE_2D                                0x0DE1
#define GL_UNSIGNED_BYTE                             0x1401
#define GL_UNSIGNED_SHORT                            0x1403
#define GL_UNSIGNED_INT                              0x1405
#define GL_FLOAT                                     0x1406
#define GL_INVERT                                    0x150A
#define GL_COLOR                                     0x1800
#define GL_DEPTH                                     0x1801
#define GL_STENCIL                                   0x1802
#define GL_POINT                                     0x1B00
#define GL_LINE                                      0x1B01
#define GL_FILL                                      0x1B02
#define GL_KEEP                                      0x1E00
#define GL_REPLACE                                   0x1E01
#define GL_INCR                                      0x1E02
#define GL_DECR                                      0x1E03
#define GL_VENDOR                                    0x1F00
#define GL_RENDERER                                  0x1F01
#define GL_VERSION                                   0x1F02
#define GL_EXTENSIONS                                0x1F03
#define GL_NEAREST                                   0x2600
#define GL_LINEAR                                    0x2601
#define GL_NEAREST_MIPMAP_NEAREST                    0x2700
#define GL_LINEAR_MIPMAP_NEAREST                     0x2701
#define GL_NEAREST_MIPMAP_LINEAR                     0x2702
#define GL_LINEAR_MIPMAP_LINEAR                      0x2703
#define GL_TEXTURE_MAG_FILTER                        0x2800
#define GL_TEXTURE_MIN_FILTER                        0x2801
#define GL_TEXTURE_WRAP_S                            0x2802
#define GL_TEXTURE_WRAP_T                            0x2803
#define GL_CLAMP                                     0x2900
#define GL_REPEAT                                    0x2901
#define GL_POLYGON_OFFSET_POINT                      0x2A01
#define GL_POLYGON_OFFSET_LINE                       0x2A02
#define GL_POLYGON_OFFSET_FILL                       0x8037
#define GL_RGB8                                      0x8051
#define GL_RGBA4                                     0x8056
#define GL_RGBA8                                     0x8058
#define GL_TEXTURE_WRAP_R                            0x8072
#define GL_BGRA                                      0x80E1
#define GL_CLAMP_TO_EDGE                             0x812F
#define GL_TEXTURE_MAX_LEVEL                         0x813D
#define GL_DEPTH_STENCIL_ATTACHMENT                  0x821A
#define GL_MAJOR_VERSION                             0x821B
#define GL_MINOR_VERSION                             0x821C
#define GL_NUM_EXTENSIONS                            0x821D
#define GL_DEBUG_OUTPUT_SYNCHRONOUS                  0x8242
#define GL_DEBUG_SOURCE_API                          0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM                0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER              0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY                  0x8249
#define GL_DEBUG_SOURCE_APPLICATION                  0x824A
#define GL_DEBUG_SOURCE_OTHER                        0x824B
#define GL_DEBUG_TYPE_ERROR                          0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR            0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR             0x824E
#define GL_DEBUG_TYPE_PORTABILITY                    0x824F
#define GL_DEBUG_TYPE_PERFORMANCE                    0x8250
#define GL_DEBUG_TYPE_OTHER                          0x8251
#define GL_DEBUG_TYPE_MARKER                         0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP                     0x8269
#define GL_DEBUG_TYPE_POP_GROUP                      0x826A
#define GL_DEBUG_SEVERITY_NOTIFICATION               0x826B
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT              0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT             0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT             0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT             0x83F3
#define GL_TEXTURE_MAX_ANISOTROPY                    0x84FE
#define GL_TEXTURE_LOD_BIAS                          0x8501
#define GL_INCR_WRAP                                 0x8507
#define GL_DECR_WRAP                                 0x8508
#define GL_TEXTURE_CUBE_MAP                          0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X               0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X               0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y               0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y               0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z               0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z               0x851A
#define GL_DEPTH24_STENCIL8                          0x88F0
#define GL_UNIFORM_BUFFER                            0x8A11
#define GL_FRAGMENT_SHADER                           0x8B30
#define GL_VERTEX_SHADER                             0x8B31
#define GL_COMPILE_STATUS                            0x8B81
#define GL_LINK_STATUS                               0x8B82
#define GL_LOWER_LEFT                                0x8CA1
#define GL_UPPER_LEFT                                0x8CA2
#define GL_FRAMEBUFFER_COMPLETE                      0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT         0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER        0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER        0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED                   0x8CDD
#define GL_COLOR_ATTACHMENT0                         0x8CE0
#define GL_COLOR_ATTACHMENT1                         0x8CE1
#define GL_COLOR_ATTACHMENT2                         0x8CE2
#define GL_COLOR_ATTACHMENT3                         0x8CE3
#define GL_COLOR_ATTACHMENT4                         0x8CE4
#define GL_COLOR_ATTACHMENT5                         0x8CE5
#define GL_COLOR_ATTACHMENT6                         0x8CE6
#define GL_COLOR_ATTACHMENT7                         0x8CE7
#define GL_COLOR_ATTACHMENT8                         0x8CE8
#define GL_COLOR_ATTACHMENT9                         0x8CE9
#define GL_COLOR_ATTACHMENT10                        0x8CEA
#define GL_COLOR_ATTACHMENT11                        0x8CEB
#define GL_COLOR_ATTACHMENT12                        0x8CEC
#define GL_COLOR_ATTACHMENT13                        0x8CED
#define GL_COLOR_ATTACHMENT14                        0x8CEE
#define GL_COLOR_ATTACHMENT15                        0x8CEF
#define GL_DEPTH_ATTACHMENT                          0x8D00
#define GL_STENCIL_ATTACHMENT                        0x8D20
#define GL_FRAMEBUFFER                               0x8D40
#define GL_RENDERBUFFER                              0x8D41
#define GL_RGB565                                    0x8D62
#define GL_GEOMETRY_SHADER                           0x8DD9
#define GL_DEBUG_SEVERITY_HIGH                       0x9146
#define GL_DEBUG_SEVERITY_MEDIUM                     0x9147
#define GL_DEBUG_SEVERITY_LOW                        0x9148
#define GL_DEBUG_OUTPUT                              0x92E0
#define GL_NEGATIVE_ONE_TO_ONE                       0x935E
#define GL_ZERO_TO_ONE                               0x935F

#define GL_POINTS                                    0x0000
#define GL_LINES                                     0x0001
#define GL_LINE_LOOP                                 0x0002
#define GL_LINE_STRIP                                0x0003
#define GL_TRIANGLES                                 0x0004
#define GL_TRIANGLE_STRIP                            0x0005
#define GL_TRIANGLE_FAN                              0x0006

#define GL_DEPTH_BUFFER_BIT                          0x00000100
#define GL_STENCIL_BUFFER_BIT                        0x00000400
#define GL_COLOR_BUFFER_BIT                          0x00004000

#define GL_MAP_READ_BIT                              0x0001
#define GL_MAP_WRITE_BIT                             0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT                  0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT                 0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT                    0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT                    0x0020
#define GL_DYNAMIC_STORAGE_BIT                       0x0100

#define OPENGL_MESSAGE_CALLBACK(name) \
	void OPENGL_CALL name(GLenum Source, GLenum Type, GLuint Id, GLenum Severity, GLsizei Length, const GLchar* Message, const void* UserParam)
typedef OPENGL_MESSAGE_CALLBACK(opengl_message_callback);

// wgl

#define WGL_CONTEXT_DEBUG_BIT_ARB              0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#define WGL_CONTEXT_MAJOR_VERSION_ARB          0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB          0x2092
#define WGL_CONTEXT_FLAGS_ARB                  0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB           0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB       0x0001

#define WGL_PREFIX wgl

#define WGL_BASE_FUNCS \
	WGL_FUNC(CreateContext, HGLRC, (HDC DeviceContext)) \
	WGL_FUNC(DeleteContext, BOOL, (HGLRC OpenGLContext)) \
	WGL_FUNC(MakeCurrent, BOOL, (HDC DeviceContext, HGLRC OpenGLContext)) \
	WGL_FUNC(GetCurrentContext, HGLRC, (void)) \
	WGL_FUNC(GetProcAddress, PROC, (const char* ProcName))

#define WGL_EXT_FUNCS \
	WGL_FUNC(GetExtensionsStringARB, const char*, (HDC DeviceContext)) \
	WGL_FUNC(CreateContextAttribsARB, HGLRC, (HDC DeviceContext, HGLRC ShareContext, const int* AttribList)) \
	WGL_FUNC(GetSwapIntervalEXT, int, (void)) \
	WGL_FUNC(SwapIntervalEXT, BOOL, (int Interval))

// standard opengl

#define GL_PREFIX gl

#define GL_BASE_FUNCS \
	GL_FUNC(GetError, GLenum, (void)) \
	GL_FUNC(Enable, void, (GLenum Cap)) \
	GL_FUNC(Disable, void, (GLenum Cap)) \
	GL_FUNC(GetIntegerv, void, (GLenum Name, GLint* Params)) \
	GL_FUNC(GetString, const GLubyte*, (GLenum Name)) \
	GL_FUNC(Viewport, void, (GLint X, GLint Y, GLint Width, GLint Height)) \
	GL_FUNC(ReadPixels, void, (GLint X, GLint Y, GLint Width, GLint Height, GLenum Format, GLenum Type, void* Pixels)) \
	GL_FUNC(Clear, void, (GLbitfield Mask)) \
	GL_FUNC(ClearColor, void, (GLclampf Red, GLclampf Green, GLclampf Blue, GLclampf Alpha)) \
	GL_FUNC(ClearDepth, void, (GLclampd Depth)) \
	GL_FUNC(ClearStencil, void, (GLint Stencil)) \
	GL_FUNC(DepthFunc, void, (GLenum Func)) \
	GL_FUNC(DepthMask, void, (GLboolean Flag)) \
	GL_FUNC(StencilFunc, void, (GLenum Func, GLint Ref, GLuint Mask)) \
	GL_FUNC(StencilMask, void, (GLuint Mask)) \
	GL_FUNC(StencilOp, void, (GLenum Fail, GLenum ZFail, GLenum ZPass)) \
	GL_FUNC(BlendFunc, void, (GLenum SrcFactor, GLenum DestFactor)) \
	GL_FUNC(CullFace, void, (GLenum Mode)) \
	GL_FUNC(PolygonMode, void, (GLenum Face, GLenum Mode)) \
	GL_FUNC(PolygonOffset, void, (GLfloat Factor, GLfloat Units)) \
	GL_FUNC(DeleteTextures, void, (GLsizei Num, const GLuint* Textures)) \
	GL_FUNC(DrawArrays, void, (GLenum Mode, GLint First, GLsizei Count)) \
	GL_FUNC(DrawElements, void, (GLenum Mode, GLsizei Count, GLenum Type, const void* Indices))

#define GL_FUNCS \
	GL_FUNC(GetStringi, const GLubyte*, (GLenum Name, GLint Index)) \
	GL_FUNC(DebugMessageCallback, void, (opengl_message_callback* Callback, void* UserParam)) \
	GL_FUNC(ClipControl, void, (GLenum Origin, GLenum Depth)) \
	GL_FUNC(CreateRenderbuffers, void, (GLsizei Num, GLuint* Buffers)) \
	GL_FUNC(DeleteRenderbuffers, void, (GLsizei Num, const GLuint* Buffers)) \
	GL_FUNC(NamedRenderbufferStorage, void, (GLuint Renderbuffer, GLenum InternalFormat, GLsizei Width, GLsizei Height)) \
	GL_FUNC(CreateBuffers, void, (GLsizei Num, GLuint* Buffers)) \
	GL_FUNC(DeleteBuffers, void, (GLsizei Num, const GLuint* Buffers)) \
	GL_FUNC(NamedBufferStorage, void, (GLuint Buffer, GLsizeiptr Size, const void* Data, GLbitfield Flags)) \
	GL_FUNC(NamedBufferSubData, void, (GLuint Buffer, GLintptr Offset, GLsizeiptr Size, const void* Data)) \
	GL_FUNC(MapNamedBufferRange, void*, (GLuint Buffer, GLintptr Offset, GLsizeiptr Length, GLbitfield Access)) \
	GL_FUNC(UnmapNamedBuffer, void, (GLuint Buffer)) \
	GL_FUNC(BindBufferBase, void, (GLenum target, GLuint Index, GLuint Buffer)) \
	GL_FUNC(VertexAttrib4f, void, (GLuint Index, GLfloat X, GLfloat Y, GLfloat Z, GLfloat W)) \
	GL_FUNC(CreateVertexArrays, void, (GLsizei Num, GLuint* Arrays)) \
	GL_FUNC(DeleteVertexArrays, void, (GLsizei Num, const GLuint* Arrays)) \
	GL_FUNC(BindVertexArray, void, (GLuint VAO)) \
	GL_FUNC(VertexArrayAttribFormat, void, (GLuint VAO, GLuint AttribIndex, GLint Size, GLenum Type, GLboolean Normalized, GLuint RelativeOffset)) \
	GL_FUNC(EnableVertexArrayAttrib, void, (GLuint VAO, GLuint Index)) \
	GL_FUNC(VertexArrayAttribBinding, void, (GLuint VAO, GLuint AttribIndex, GLuint BindingIndex)) \
	GL_FUNC(VertexArrayVertexBuffer, void, (GLuint VAO, GLuint BindingIndex, GLuint Buffer, GLintptr Offset, GLsizei Stride)) \
	GL_FUNC(VertexArrayElementBuffer, void, (GLuint VAO, GLuint Buffer)) \
	GL_FUNC(CreateSamplers, void, (GLsizei Num, GLuint* Samplers)) \
	GL_FUNC(DeleteSamplers, void, (GLsizei Num, const GLuint* Samplers)) \
	GL_FUNC(BindSamplers, void, (GLuint First, GLsizei Count, const GLuint* Samplers)) \
	GL_FUNC(SamplerParameteri, void, (GLuint Sampler, GLenum Name, GLint Param)) \
	GL_FUNC(SamplerParameterf, void, (GLuint Sampler, GLenum Name, GLfloat Param)) \
	GL_FUNC(CreateTextures, void, (GLenum Target, GLsizei Num, GLuint* Textures)) \
	GL_FUNC(BindTextureUnit, void, (GLuint Unit, GLuint Texture)) \
	GL_FUNC(TextureParameteri, void, (GLuint Texture, GLenum Name, GLint Param)) \
	GL_FUNC(TextureStorage2D, void, (GLuint Texture, GLsizei Levels, GLenum InternalFormat, GLsizei Width, GLsizei Height)) \
	GL_FUNC(TextureSubImage2D, void, (GLuint Texture, GLint Level, GLint XOffset, GLint YOffset, GLsizei Width, GLsizei Height, GLenum Format, GLenum Type, const void* Pixels)) \
	GL_FUNC(TextureSubImage3D, void, (GLuint Texture, GLint Level, GLint XOffset, GLint YOffset, GLint ZOffset, GLsizei Width, GLsizei Height, GLsizei Depth, GLenum Format, GLenum Type, const void* Pixels)) \
	GL_FUNC(GenerateTextureMipmap, void, (GLuint Texture)) \
	GL_FUNC(CreateFramebuffers, void, (GLsizei Num, GLuint* Framebuffers)) \
	GL_FUNC(DeleteFramebuffers, void, (GLsizei Num, const GLuint* Framebuffers)) \
	GL_FUNC(BindFramebuffer, void, (GLenum Target, GLuint Framebuffer)) \
	GL_FUNC(NamedFramebufferTexture, void, (GLuint Framebuffer, GLenum Attachment, GLuint Texture, GLint Level)) \
	GL_FUNC(NamedFramebufferRenderbuffer, void, (GLuint Framebuffer, GLenum Attachment, GLenum RenderbufferTarget, GLuint Renderbuffer)) \
	GL_FUNC(CheckNamedFramebufferStatus, GLenum, (GLuint Framebuffer, GLenum Target)) \
	GL_FUNC(BlitNamedFramebuffer, void, (GLuint ReadFramebuffer, GLuint DrawFramebuffer, GLint SrcX0, GLint SrcY0, GLint SrcX1, GLint SrcY1, GLint DstX0, GLint DstY0, GLint DstX1, GLint DestY1, GLbitfield Mask, GLenum Filter)) \
	GL_FUNC(ClearNamedFramebufferfv, void, (GLuint Framebuffer, GLenum Buffer, GLint Drawbuffer, GLfloat* Value)) \
	GL_FUNC(CreateShader, GLuint, (GLenum Type)) \
	GL_FUNC(DeleteShader, void, (GLuint Shader)) \
	GL_FUNC(ShaderSource, void, (GLuint Shader, GLsizei Count, const GLchar* const* String, const GLint* Length)) \
	GL_FUNC(CompileShader, void, (GLuint Shader)) \
	GL_FUNC(GetShaderiv, void, (GLuint Shader, GLenum Name, GLint* Param)) \
	GL_FUNC(GetShaderInfoLog, void, (GLuint Shader, GLsizei BufferSize, GLsizei* Length, GLchar* InfoLog)) \
	GL_FUNC(CreateProgram, GLuint, (void)) \
	GL_FUNC(DeleteProgram, void, (GLuint Program)) \
	GL_FUNC(ProgramUniform4fv, void, (GLuint Program, GLint Location, GLsizei Count, const GLfloat* Value)) \
	GL_FUNC(ProgramUniformMatrix4fv, void, (GLuint Program, GLint Location, GLsizei Count, GLboolean Transpose, const GLfloat* Value)) \
	GL_FUNC(UseProgram, void, (GLuint Program)) \
	GL_FUNC(AttachShader, void, (GLuint Program, GLuint Shader)) \
	GL_FUNC(DetachShader, void, (GLuint Program, GLuint Shader)) \
	GL_FUNC(LinkProgram, void, (GLuint Program)) \
	GL_FUNC(GetProgramiv, void, (GLuint Program, GLenum Name, GLint* Param)) \
	GL_FUNC(GetProgramInfoLog, void, (GLuint Program, GLsizei BufferSize, GLsizei* Length, GLchar* InfoLog)) \
	GL_FUNC(DrawArraysInstanced, void, (GLenum Mode, GLint First, GLsizei Count, GLsizei PrimCount)) \
	GL_FUNC(DrawElementsInstanced, void, (GLenum Mode, GLsizei Count, GLenum Type, const void* Indices, GLsizei PrimCount)) \
	GL_FUNC(DrawRangeElements, void, (GLenum Mode, GLuint Start, GLuint End, GLsizei Count, GLenum Type, const void* Indices)) \
	GL_FUNC(DrawRangeElementsBaseVertex, void, (GLenum Mode, GLuint Start, GLuint End, GLsizei Count, GLenum Type, const void* Indices, GLint BaseVertex))

#define GL_EXT_FUNCS \
	GL_FUNC(CompressedTextureImage2DEXT, void, (GLuint Texture, GLenum Target, GLint Level, GLenum InternalFormat, GLsizei Width, GLsizei Height, GLint Border, GLsizei ImageSize, const void* Data))
