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
#include "Base/FastNameMap.h"
#include "FileSystem/FilePath.h"
#include "Concurrency/JobManager.h"

//#define USE_CRC_COMPARE

#ifdef __DAVAENGINE_ARM_7__
#define USE_NEON_MATRIX_COMPARE
#include <arm_neon.h>
#endif

#ifdef __DAVAENGINE_ANDROID__
#if !defined(GLchar)
typedef char             GLchar;
#endif //not defined GLchar
#endif //#ifdef __DAVAENGINE_ANDROID__

#define GET_UNIFORM(__uniformIndex__) ((Uniform*)(uniformData + uniformOffsets[__uniformIndex__]))

namespace DAVA
{

    
struct AutobindVariableData
{
    pointer_size updateSemantic;    // Use lower 1 bit, for indication of update
    const void * value;
    
    inline void SetUpdateSemantic(pointer_size _updateSemantic)
    {
        //updateSemantic = 1 | (_updateSemantic & 0xFFFFFFFE);
        updateSemantic = _updateSemantic;
    };
    
    inline void ResetRequireUpdate(pointer_size flag)
    {
        updateSemantic &= 0xFFFFFFFE;
        updateSemantic |= flag & 1;
    }
    inline pointer_size IsRequireUpdate()
    {
        return updateSemantic & 1;
    }
};
    

template<class AutobindType>
class AutobindVariable
{
public:
    inline void SetValue(uint32 _semantic, const AutobindType * _value){ if (semantic != _semantic){ semantic = _semantic; value = _value;} }
    inline void ClearSemantic() { semantic = 0; }
    inline AutobindType * GetValue() {return  value; }
    
    uint32 semantic;
    AutobindType *value;
};

class AutobindManager
{
public:
    template<class T>
    AutobindVariable<T> * AllocateVariable(const FastName & name);
    
    template<class T>
    AutobindVariable<T> * GetVariable(const FastName & name);
    
    HashMap<FastName, uint32> manager;
    Vector<uint32> bytes;
};
    
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
        UNIFORM_MODEL_VIEW_TRANSLATE,
        UNIFORM_MODEL_SCALE,
        UNIFORM_COUNT,
    };
    enum eUpdateFreq
    {
        UPDATE_ALWAYS = 0,
        UPDATE_ONCE = 1,
        UPDATE_PER_FRAME = 2,
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
    
#ifdef USE_NEON_MATRIX_COMPARE
#pragma pack(push)
#pragma pack(4)
#endif
    struct Uniform
    {
        eShaderSemantic shaderSemantic;
        //eUpdateFreq     updateFreq;
        FastName        name;
        GLint           location;
        GLint           size;
        eUniformType    type;
        pointer_size    updateSemantic;
#ifdef USE_CRC_COMPARE
        uint32          crc;
#else
        void*			cacheValue;
		uint16			cacheValueSize;
#endif
        
#ifdef USE_NEON_MATRIX_COMPARE
        uint32x4_t      matrixCRC;
#endif
        
		bool ValidateCache(int32 value);
		bool ValidateCache(float32 value);
		bool ValidateCache(const Vector2 & value);
		bool ValidateCache(const Vector3 & value);
		bool ValidateCacheColor3(const Color & value);
		bool ValidateCacheColor4(const Color & value);
		bool ValidateCache(const Vector4 & value);
		bool ValidateCache(const Matrix4 & value);
		bool ValidateCache(const Matrix3 & value);
		bool ValidateCache(const void* value, uint16 valueSize);
    };
#ifdef USE_NEON_MATRIX_COMPARE
#pragma pack(pop)
#endif

protected:
    virtual ~Shader();
public:
    Shader();
    
    
    Shader * Clone();
    
    void SetDefines(const String & _defines);

    static Shader * CreateShader(const FastName & assetName,
                                  Data * vertexShaderData,
                                  Data * fragmentShaderData,
                                  uint8 * vertexShaderDataStart,
                                  uint32 vertexShaderDataSize,
                                  uint8 * fragmentShaderDataStart,
                                  uint32 fragmentShaderDataSize,
                                  const FastNameSet & definesSet);
    
    void Reload(    Data * vertexShaderData,
                    Data * fragmentShaderData,
                    uint8 * vertexShaderDataStart,
                    uint32 vertexShaderDataSize,
                    uint8 * fragmentShaderDataStart,
                    uint32 fragmentShaderDataSize);

    
    
    const FastName & GetAssetName() { return assetName; };
    
    bool Recompile(bool silentDelete = false);
	bool IsReady();
    
    void ClearLastBindedCaches();

    void Bind();
    void BindDynamicParameters();

    static void Unbind();
    
	static bool IsAutobindUniform(eShaderSemantic uniformId);

    inline int32 GetAttributeIndex(eVertexFormat vertexFormat);
    inline int32 GetAttributeCount();    
    
    inline int32 GetUniformCount();
    inline Uniform * GetUniform(int32 index);
    inline eUniformType GetUniformType(int32 index);
    inline const char * GetUniformName(int32 index);
    inline int32 GetUniformArraySize(int32 index);

    static int32 GetUniformTypeSize(eUniformType type);
    static const char * GetUniformTypeSLName(eUniformType type);

    int32 GetUniformLocationByIndex(int32 index);
    //int32 FindUniformLocationByName(const FastName & name);
    int32 FindUniformIndexByName(const FastName & name);
    
    void SetUniformValueByIndex(int32 uniformIndex, eUniformType uniformType, uint32 arraySize, void * data);
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
	
    void SetUniformValueByUniform(Uniform* uniform, eUniformType uniformType, uint32 arraySize, void * data);
	void SetUniformValueByUniform(Uniform* uniform, int32 value);
    void SetUniformValueByUniform(Uniform* uniform, float32 value);
    void SetUniformValueByUniform(Uniform* uniform, const Vector2 & vector);
    void SetUniformValueByUniform(Uniform* uniform, const Vector3 & vector);
    void SetUniformColor3ByUniform(Uniform* uniform, const Color & color);
    void SetUniformColor4ByUniform(Uniform* uniform, const Color & color);
    void SetUniformValueByUniform(Uniform* uniform, const Vector4 & vector);
    void SetUniformValueByUniform(Uniform* uniform, const Matrix4 & matrix);
	void SetUniformValueByUniform(Uniform* uniform, const Matrix3 & matrix);
    
    void Dump();
    
    /**
        This function return vertex format required by shader
     */
    uint32 GetRequiredVertexFormat(){return requiredVertexFormat;}

#if defined(__DAVAENGINE_ANDROID__)
	virtual void Lost();
	virtual void Invalidate();
#endif //#if defined(__DAVAENGINE_ANDROID__)

private:
    
    void ReleaseShaderData(bool deleteShader = true);
    

    uint32 requiredVertexFormat;

#if defined(__DAVAENGINE_DIRECTX9__)
#elif defined(__DAVAENGINE_OPENGL__)
    String shaderDefines;
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;
    
    FastName *attributeNames;
    GLint activeAttributes;
    GLint activeUniforms;    
	
	uint16* uniformOffsets;
	uint8* uniformData;
	Uniform** autobindUniforms;
	uint8 autobindUniformCount;
    
    int32 vertexFormatAttribIndeces[VERTEX_FORMAT_STREAM_MAX_COUNT];
    
    GLint CompileShader(GLuint *shader, GLenum type, GLint count, const GLchar * sources, const String & defines);
    GLint LinkProgram(GLuint prog);

	void RecompileInternal(bool silentDelete);
    
	void DeleteShaders();
	struct DeleteShaderContainer
	{
		GLuint program;
		GLuint vertexShader;
		GLuint fragmentShader;
	};
	void DeleteShadersInternal(GLuint program, GLuint vertexShader, GLuint fragmentShader);

    eShaderSemantic GetShaderSemanticByName(const FastName &name);
    int32 GetAttributeIndexByName(const FastName &name);
    
    static GLuint activeProgram;    
    Data * vertexShaderData;
    Data * fragmentShaderData;
    FilePath vertexShaderPath, fragmentShaderPath;
    FastName assetName;
#endif
        
    uint8 * vertexShaderDataStart;
    uint8 * fragmentShaderDataStart;
    uint32 vertexShaderDataSize;
    uint32 fragmentShaderDataSize;
};

//
inline int32 Shader::GetAttributeCount()
{
    return activeAttributes;
}

inline int32 Shader::GetAttributeIndex(eVertexFormat vertexFormat)
{
    return vertexFormatAttribIndeces[FastLog2(vertexFormat)];
}
    
inline int32 Shader::GetUniformCount()
{
    return activeUniforms;
}

inline Shader::eUniformType Shader::GetUniformType(int32 index)
{
    return GET_UNIFORM(index)->type;
}

inline Shader::Uniform * Shader::GetUniform(int32 index)
{
    return GET_UNIFORM(index);
}

inline const char * Shader::GetUniformName(int32 index)
{
    return GET_UNIFORM(index)->name.c_str();
}

inline int32 Shader::GetUniformLocationByIndex(int32 index)
{
    return GET_UNIFORM(index)->location;
}

inline int32 Shader::GetUniformArraySize(int32 index)
{
    return GET_UNIFORM(index)->size;
}
    
};


#endif // __DAVAENGINE_SHADER_H__
