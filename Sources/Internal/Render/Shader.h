/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_SHADER_H__
#define __DAVAENGINE_SHADER_H__

#include "Render/RenderBase.h"
#include "Render/RenderResource.h"
#include "Render/VertexBuffer.h"
#include "Base/BaseMath.h"
#include "Base/Data.h"
#include "Base/FastName.h"
#include "FileSystem/FilePath.h"
#include "Job/JobManager.h"
#include "Job/JobWaiter.h"

#ifdef __DAVAENGINE_ANDROID__
#if !defined(GLchar)
typedef char             GLchar;
#endif //not defined GLchar
#endif //#ifdef __DAVAENGINE_ANDROID__


namespace DAVA
{
    
class Data;
    
/*	
    \brief Class to use low-level shaders
*/	
class Shader : public RenderResource
{
public:
    enum eUniform
    {
        UNIFORM_NONE = 0, 
        UNIFORM_MODEL_VIEW_PROJECTION_MATRIX,  // gl_ModelViewProjectionMatrix
        UNIFORM_MODEL_VIEW_MATRIX,
		UNIFORM_PROJECTION_MATRIX,
        UNIFORM_NORMAL_MATRIX, 
        UNIFORM_COLOR,
        UNIFORM_GLOBAL_TIME,
        UNIFORM_COUNT,
    };
    
    enum eUniformType
    {
        UT_FLOAT = GL_FLOAT,
        UT_FLOAT_VEC2 = GL_FLOAT_VEC2,
        UT_FLOAT_VEC3 = GL_FLOAT_VEC3,
        UT_FLOAT_VEC4 = GL_FLOAT_VEC4,
        UT_INT = GL_INT,
        UT_INT_VEC2 = GL_INT_VEC2,
        UT_INT_VEC3 = GL_INT_VEC3,
        UT_INT_VEC4 = GL_INT_VEC4,
        UT_BOOL = GL_BOOL,
        UT_BOOL_VEC2 = GL_BOOL_VEC2,
        UT_BOOL_VEC3 = GL_BOOL_VEC3,
        UT_BOOL_VEC4 = GL_BOOL_VEC4,
        UT_FLOAT_MAT2 = GL_FLOAT_MAT2,
        UT_FLOAT_MAT3 = GL_FLOAT_MAT3,
        UT_FLOAT_MAT4 = GL_FLOAT_MAT4,
        UT_SAMPLER_2D = GL_SAMPLER_2D,
        UT_SAMPLER_CUBE = GL_SAMPLER_CUBE,
    };

protected:
    virtual ~Shader();
public:
    Shader();
    
    Shader * Clone();
    
    // virtual void SetActiveShader(const String & string);
    void SetDefines(const String & defines);
    void SetVertexShaderDefines(const String & defines);
    void SetFragmentShaderDefines(const String & defines);
    
    // comma ';' sepated define list
    void SetDefineList(const String & enableDefinesList);
    
    bool LoadFromYaml(const FilePath & pathname);
    bool Load(const FilePath & vertexShaderPath, const FilePath & fragmentShaderPath);
    
	void Recompile();
	void RecompileAsync();
	void RecompileInternal(BaseObject * caller, void * param, void *callerData);

    Shader * RecompileNewInstance(const String & combination);
    
    void Bind();
    static void Unbind();
    int32 GetAttributeIndex(eVertexFormat vertexFormat);
    int32 GetAttributeCount();
    
    int32 GetUniformCount();
    eUniformType GetUniformType(int32 index);
    static int32 GetUniformTypeSize(eUniformType type);
    static const char * GetUniformTypeSLName(eUniformType type);
    const char * GetUniformName(int32 index);
    int32 GetUniformArraySize(int32 index);

    int32 GetUniformLocationByIndex(int32 index);
    //int32 FindUniformLocationByName(const FastName & name);
    int32 FindUniformIndexByName(const FastName & name);
    
    /*void SetUniformValue(int32 uniformLocation, int32 value);
    void SetUniformValue(int32 uniformLocation, float32 value);
    void SetUniformValue(int32 uniformLocation, int32 count, int32 * value);
    void SetUniformValue(int32 uniformLocation, int32 count, float32 * value);
    void SetUniformValue(int32 uniformLocation, const Vector2 & vector);
    void SetUniformValue(int32 uniformLocation, const Vector3 & vector);
    void SetUniformColor3(int32 uniformLocation, const Color & color);
    void SetUniformColor4(int32 uniformLocation, const Color & color);
    void SetUniformValue(int32 uniformLocation, const Vector4 & vector);
    void SetUniformValue(int32 uniformLocation, const Matrix4 & matrix);*/

	void SetUniformValueByIndex(int32 uniformIndex, int32 value);
    void SetUniformValueByIndex(int32 uniformIndex, float32 value);
    //void SetUniformValueByIndex(int32 uniformIndex, int32 count, int32 * value);
    //void SetUniformValueByIndex(int32 uniformIndex, int32 count, float32 * value);
    void SetUniformValueByIndex(int32 uniformIndex, const Vector2 & vector);
    void SetUniformValueByIndex(int32 uniformIndex, const Vector3 & vector);
    void SetUniformColor3ByIndex(int32 uniformIndex, const Color & color);
    void SetUniformColor4ByIndex(int32 uniformIndex, const Color & color);
    void SetUniformValueByIndex(int32 uniformIndex, const Vector4 & vector);
    void SetUniformValueByIndex(int32 uniformIndex, const Matrix4 & matrix);
	void SetUniformValueByIndex(int32 uniformIndex, const Matrix3 & matrix);
    
    void Dump();
    
    /**
        This function return vertex format required by shader
     */
    //virtual uint32 GetVertexFormat();
    //virtual uint32 GetAttributeIndex(eVertexFormat fmt);
    
//#if defined(__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
//	virtual void SaveToSystemMemory();
//	virtual void Lost();
//	virtual void Invalidate();
//    String relativeFileName;
//#endif //#if defined(__DAVAENGINE_ANDROID__) 

#if defined(__DAVAENGINE_ANDROID__)
	virtual void Lost();
	virtual void Invalidate();
#endif //#if defined(__DAVAENGINE_ANDROID__)

    
private:
#if defined(__DAVAENGINE_DIRECTX9__)
    
    
#elif defined(__DAVAENGINE_OPENGL__)
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;
    
    FastName *attributeNames;
    GLint activeAttributes;
    GLint activeUniforms;
    
    
//    eUniform *uniformIDs;
//    String * uniformNames;
//    GLint * uniformLocations;
//    GLint * uniformSizes;
//    eUniformType * uniformTypes;
    
    
    struct Uniform
    {
        eUniform        id;
        FastName        name;
        GLint           location;
        GLint           size;
        eUniformType    type;
		void*			cacheValue;
		uint16			cacheValueSize;
		
		bool ValidateCache(int32 value);
		bool ValidateCache(float32 value);
		bool ValidateCache(const Vector2 & value);
		bool ValidateCache(const Vector3 & value);
		bool ValidateCacheColor3(const Color & value);
		bool ValidateCacheColor4(const Color & value);
		bool ValidateCache(const Vector4 & value);
		bool ValidateCache(const Matrix4 & value);
		bool ValidateCache(const Matrix3 & value);
	};
	
	uint16* uniformOffsets;
	uint8* uniformData;
    
    int32 vertexFormatAttribIndeces[VERTEX_FORMAT_STREAM_MAX_COUNT];
    
    GLint CompileShader(GLuint *shader, GLenum type, GLint count, const GLchar * sources, const String & defines);
    GLint LinkProgram(GLuint prog);
    
	void DeleteShaders();
	struct DeleteShaderContainer
	{
		GLuint program;
		GLuint vertexShader;
		GLuint fragmentShader;
	};
	void DeleteShadersInternal(BaseObject * caller, void * param, void *callerData);

    eUniform GetUniformByName(const FastName &name);
    int32 GetAttributeIndexByName(const FastName &name);
    
    static GLuint activeProgram;
    String vertexShaderDefines;
    String fragmentShaderDefines;
    
    Data * vertexShaderData;
    Data * fragmentShaderData;
    FilePath vertexShaderPath, fragmentShaderPath;
/*  uint8 * vertexShaderBytes;
    uint32 vertexShaderSize;
    uint8 * fragmentShaderBytes;
    uint32 fragmentShaderSize;*/
#endif
};
};

#endif // __DAVAENGINE_SHADER_H__
