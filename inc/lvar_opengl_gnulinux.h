#pragma once

#include <GL/gl.h>
#include <GL/glx.h>

#ifndef PFNGLPOLYGONMODEPROC
typedef void (APIENTRY *PFNGLPOLYGONMODEPROC)(GLenum face, GLenum mode);
#endif

extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
extern PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
extern PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLUNIFORM4FPROC glUniform4f;
extern PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
extern PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
extern PFNGLBINDBUFFERBASEPROC glBindBufferBase;
extern PFNGLPOLYGONMODEPROC myGlPolygonMode;

#define glPolygonMode myGlPolygonMode

inline void* getGLProcAddress(const char* name)
{
  return reinterpret_cast<void*>(glXGetProcAddress((const GLubyte*)name));
}

// --------------------------------------------------------
// OpenGL function pointers
// --------------------------------------------------------
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
PFNGLBINDBUFFERBASEPROC glBindBufferBase;
PFNGLPOLYGONMODEPROC myGlPolygonMode;

void init_opengl_ptrs()
{
  glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)getGLProcAddress("glGetUniformLocation");
  glUniform1i = (PFNGLUNIFORM1IPROC)getGLProcAddress("glUniform1i");
  glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)getGLProcAddress("glGenVertexArrays");
  glGenBuffers = (PFNGLGENBUFFERSPROC)getGLProcAddress("glGenBuffers");
  glBindBuffer = (PFNGLBINDBUFFERPROC)getGLProcAddress("glBindBuffer");
  glBufferData = (PFNGLBUFFERDATAPROC)getGLProcAddress("glBufferData");
  glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)getGLProcAddress("glBindVertexArray");
  glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)getGLProcAddress("glEnableVertexAttribArray");
  glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)getGLProcAddress("glVertexAttribPointer");
  glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)getGLProcAddress("glDeleteBuffers");
  glGetShaderiv = (PFNGLGETSHADERIVPROC)getGLProcAddress("glGetShaderiv");
  glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)getGLProcAddress("glGetShaderInfoLog");
  glCreateShader = (PFNGLCREATESHADERPROC)getGLProcAddress("glCreateShader");
  glShaderSource = (PFNGLSHADERSOURCEPROC)getGLProcAddress("glShaderSource");
  glCompileShader = (PFNGLCOMPILESHADERPROC)getGLProcAddress("glCompileShader");
  glDeleteShader = (PFNGLDELETESHADERPROC)getGLProcAddress("glDeleteShader");
  glCreateProgram = (PFNGLCREATEPROGRAMPROC)getGLProcAddress("glCreateProgram");
  glAttachShader = (PFNGLATTACHSHADERPROC)getGLProcAddress("glAttachShader");
  glLinkProgram = (PFNGLLINKPROGRAMPROC)getGLProcAddress("glLinkProgram");
  glGetProgramiv = (PFNGLGETPROGRAMIVPROC)getGLProcAddress("glGetProgramiv");
  glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)getGLProcAddress("glGetProgramInfoLog");
  glUseProgram = (PFNGLUSEPROGRAMPROC)getGLProcAddress("glUseProgram");
  glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)getGLProcAddress("glGenerateMipmap");
  glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC) glXGetProcAddress(reinterpret_cast<const GLubyte*>("glXSwapIntervalEXT"));
  glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) glXGetProcAddress(reinterpret_cast<const GLubyte*>("glDeleteVertexArrays"));
  glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) glXGetProcAddress(reinterpret_cast<const GLubyte*>("glUniformMatrix4fv"));
  glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)getGLProcAddress("glDrawArraysInstanced");
  glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)getGLProcAddress("glVertexAttribDivisor");
  glBufferSubData = (PFNGLBUFFERSUBDATAPROC)getGLProcAddress("glBufferSubData");
  glDeleteProgram = (PFNGLDELETEPROGRAMPROC)getGLProcAddress("glDeleteProgram");
  glUniform1f = (PFNGLUNIFORM1FPROC)getGLProcAddress("glUniform1f");
  glUniform3f = (PFNGLUNIFORM3FPROC)getGLProcAddress("glUniform3f");
  glUniform4f = (PFNGLUNIFORM4FPROC)getGLProcAddress("glUniform4f");
  glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)getGLProcAddress("glGetUniformBlockIndex");
  glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)getGLProcAddress("glUniformBlockBinding");
  glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)getGLProcAddress("glBindBufferBase");
  myGlPolygonMode = (PFNGLPOLYGONMODEPROC)getGLProcAddress("glPolygonMode");
}
