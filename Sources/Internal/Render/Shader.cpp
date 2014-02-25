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


#include "Render/Shader.h"
#include "Render/RenderBase.h"
#include "Platform/Thread.h"
#include "Render/RenderManager.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/FileSystem.h"
#include "Platform/SystemTimer.h"
#include "Utils/Utils.h"
#include "Utils/StringFormat.h"
#include "Math/Math2D.h"

#define GET_UNIFORM(__uniformIndex__) ((Uniform*)(uniformData + uniformOffsets[__uniformIndex__]))

namespace DAVA 
{
#if defined(__DAVAENGINE_OPENGL__)
GLuint Shader::activeProgram = 0;

Shader::Shader()
    : RenderResource()
{
    DVASSERT(RenderManager::Instance()->GetRenderer() == Core::RENDERER_OPENGL_ES_2_0 || RenderManager::Instance()->GetRenderer() == Core::RENDERER_OPENGL);
    
    vertexShader = 0;
    fragmentShader = 0;
    program = 0;
    
    attributeNames = 0;
    activeAttributes = 0;
    activeUniforms = 0;

    //uniforms = 0;
	uniformData = NULL;
	uniformOffsets = NULL;
    
    for (int32 ki = 0; ki < VERTEX_FORMAT_STREAM_MAX_COUNT; ++ki)
         vertexFormatAttribIndeces[ki] = -1;

    vertexShaderData = 0;
    fragmentShaderData = 0;
    
//#if defined(__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
//    relativeFileName = "";
//#endif //#if defined(__DAVAENGINE_ANDROID__) 
}

String VertexTypeStringFromEnum(GLenum type); // Fucking XCode 4 analyzer
String VertexTypeStringFromEnum(GLenum type)
{
    if (type == GL_FLOAT)return "GL_FLOAT";
    else if (type == GL_FLOAT_VEC2)return "GL_FLOAT_VEC2";
    else if (type == GL_FLOAT_VEC3)return "GL_FLOAT_VEC3";
    else if (type == GL_FLOAT_VEC4)return "GL_FLOAT_VEC4";
    else if (type == GL_FLOAT_MAT2)return "GL_FLOAT_MAT2";
    else if (type == GL_FLOAT_MAT3)return "GL_FLOAT_MAT3";
    else if (type == GL_FLOAT_MAT4)return "GL_FLOAT_MAT4";
    return "";
}
    
FastName uniformStrings[Shader::UNIFORM_COUNT] =
    {
        "none",
        "modelViewProjectionMatrix",
        "modelViewMatrix",
		"projectionMatrix",
        "normalMatrix",
        "flatColor",
        "globalTime",
    };

FastName attributeStrings[VERTEX_FORMAT_STREAM_MAX_COUNT] =
    {
        "inPosition",
        "inNormal",
        "inColor",
        "inTexCoord0",
        "inTexCoord1",
        "inTexCoord2",
        "inTexCoord3",
        "inTangent",
        "inBinormal",
        "inJointWeight",
		"inTime"
    };
    
Shader::eUniform Shader::GetUniformByName(const FastName & name)
{
    for (int32 k = 0; k < UNIFORM_COUNT; ++k)
        if (name == uniformStrings[k])return (Shader::eUniform)k;
    return Shader::UNIFORM_NONE;
};
    
int32 Shader::GetUniformCount()
{
    return activeUniforms;
}
    
Shader::eUniformType Shader::GetUniformType(int32 index)
{
    return GET_UNIFORM(index)->type;
}
    
int32 Shader::GetUniformTypeSize(eUniformType type)
{
    switch(type)
    {
        case UT_FLOAT:
            return 4;
        case UT_FLOAT_VEC2:
            return 4 * 2;
        case UT_FLOAT_VEC3:
            return 4 * 3;
        case UT_FLOAT_VEC4:
            return 4 * 4;
        case UT_INT:
            return 4 * 1;
        case UT_INT_VEC2:
            return 4 * 2;
        case UT_INT_VEC3:
            return 4 * 3;
        case UT_INT_VEC4:
            return 4 * 4;
        case UT_BOOL:
            return 4 * 1;
        case UT_BOOL_VEC2:
            return 4 * 2;
        case UT_BOOL_VEC3:
            return 4 * 3;
        case UT_BOOL_VEC4:
            return 4 * 4;
        case UT_FLOAT_MAT2:
            return 4 * 2 * 2;
        case UT_FLOAT_MAT3:
            return 4 * 3 * 3;
        case UT_FLOAT_MAT4:
            return 4 * 4 * 4;
        case UT_SAMPLER_2D:
            return 4;
        case UT_SAMPLER_CUBE:
            return 4;

		default:
			break;
    };

	return 0;
}
    
    
const char * Shader::GetUniformTypeSLName(eUniformType type)
{
    switch(type)
    {
        case UT_FLOAT:
            return "float";
        case UT_FLOAT_VEC2:
            return "vec2";
        case UT_FLOAT_VEC3:
            return "vec3";
        case UT_FLOAT_VEC4:
            return "vec4";
        case UT_INT:
            return "int";
        case UT_INT_VEC2:
            return "ivec2";
        case UT_INT_VEC3:
            return "ivec3";
        case UT_INT_VEC4:
            return "ivec4";
        case UT_BOOL:
            return "bool";
        case UT_BOOL_VEC2:
            return "bvec2";
        case UT_BOOL_VEC3:
            return "bvec3";
        case UT_BOOL_VEC4:
            return "bvec4";
        case UT_FLOAT_MAT2:
            return "mat2";
        case UT_FLOAT_MAT3:
            return "mat3";
        case UT_FLOAT_MAT4:
            return "mat4";
        case UT_SAMPLER_2D:
            return "sampler2D";
        case UT_SAMPLER_CUBE:
            return "samplerCube";

		default:
			break;
    };

	return "";
}

    
    
const char * Shader::GetUniformName(int32 index)
{
    return GET_UNIFORM(index)->name.c_str();
}

int32 Shader::GetUniformLocationByIndex(int32 index)
{
    return GET_UNIFORM(index)->location;
}
    
int32 Shader::GetUniformArraySize(int32 index)
{
    return GET_UNIFORM(index)->size;
}

/*int32 Shader::FindUniformLocationByName(const FastName & name)
{
    for (int32 k = 0; k < activeUniforms; ++k)
    {
        if (uniforms[k].name == name)
        {
            return uniforms[k].location;
        }
    }
    return -1;
}*/

int32 Shader::FindUniformIndexByName(const FastName & name)
{
	for (int32 k = 0; k < activeUniforms; ++k)
    {
        if (GET_UNIFORM(k)->name == name)
        {
            return k;
        }
    }
	
    return -1;
}
    
int32 Shader::GetAttributeIndexByName(const FastName & name)
{
    for (int32 k = 0; k < VERTEX_FORMAT_STREAM_MAX_COUNT; ++k)
        if (name == attributeStrings[k]) return k;
    return -1;
}
    
void Shader::SetDefines(const String & _defines)
{
    vertexShaderDefines = fragmentShaderDefines = _defines;
}

void Shader::SetVertexShaderDefines(const String & _defines)
{
    vertexShaderDefines = _defines;
}
    
void Shader::SetFragmentShaderDefines(const String & _defines)
{
    fragmentShaderDefines = _defines;
}
    
void Shader::SetDefineList(const String & enableDefinesList)
{
    Vector<String> defineNameList;
    String result;
    Split(enableDefinesList, ";", defineNameList, true);
    size_t size = defineNameList.size();
    for (size_t i = 0; i < size; ++i)
    {
        result += Format("#define %s\n", defineNameList[i].c_str());
    }
    SetDefines(result);
}
    
bool Shader::LoadFromYaml(const FilePath & pathname)
{
//#if defined(__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
//    relativeFileName = pathname;
//#endif //#if defined(__DAVAENGINE_ANDROID__) 

    uint64 shaderLoadTime = SystemTimer::Instance()->AbsoluteMS();
    
    YamlParser * parser = YamlParser::Create(pathname);
    if (!parser)
        return false;
    
    YamlNode * rootNode = parser->GetRootNode();
    if (!rootNode)
    {
        SafeRelease(rootNode);
        return false;
    }
    
    const YamlNode * vertexShaderNode = rootNode->Get("vertexShader");
    if (!vertexShaderNode)
    {
        SafeRelease(parser);
        return false;
    }

    const YamlNode * glslVertexNode = vertexShaderNode->Get("glsl");
    if (!glslVertexNode)
    {
        SafeRelease(parser);
        return false;
    }
    
    const YamlNode * fragmentShaderNode = rootNode->Get("fragmentShader");
    if (!fragmentShaderNode)
    {
        SafeRelease(parser);
        return false;
    }
    
    const YamlNode * glslFragmentNode = fragmentShaderNode->Get("glsl");
    if (!glslFragmentNode)
    {
        SafeRelease(parser);
        return false;
    }

    FilePath pathOnly(pathname.GetDirectory());
    vertexShaderPath = pathOnly + glslVertexNode->AsString();
    fragmentShaderPath = pathOnly + glslFragmentNode->AsString();
    SafeRelease(parser);

    Load(vertexShaderPath, fragmentShaderPath);
    
    shaderLoadTime = SystemTimer::Instance()->AbsoluteMS() - shaderLoadTime;
    
//    Logger::FrameworkDebug("shader loaded:%s load-time: %lld ms", pathname.c_str(), shaderLoadTime);
    return true;
}
    
bool Shader::Load(const FilePath & _vertexShaderPath, const FilePath & _fragmentShaderPath)
{
    vertexShaderPath = _vertexShaderPath;
    fragmentShaderPath = _fragmentShaderPath;
    uint32 vertexShaderSize = 0, fragmentShaderSize = 0;
        
    uint8 * vertexShaderBytes = FileSystem::Instance()->ReadFileContents(vertexShaderPath, vertexShaderSize);
    vertexShaderData = new Data(vertexShaderBytes, vertexShaderSize);
    
    uint8 * fragmentShaderBytes = FileSystem::Instance()->ReadFileContents(fragmentShaderPath, fragmentShaderSize);
    fragmentShaderData = new Data(fragmentShaderBytes, fragmentShaderSize);
    
    return true;
}
    
Shader::~Shader()
{
    SafeDeleteArray(attributeNames);
    //SafeDeleteArray(uniforms);
	SafeDeleteArray(uniformOffsets);
	SafeDeleteArray(uniformData);
    
    SafeRelease(vertexShaderData);
    SafeRelease(fragmentShaderData);
    
    DeleteShaders();
}
    
void Shader::Recompile()
{
	ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &Shader::RecompileInternal));
	JobInstanceWaiter waiter(job);
	waiter.Wait();
}

void Shader::RecompileAsync()
{
	ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &Shader::RecompileInternal));
}

void Shader::RecompileInternal(BaseObject * caller, void * param, void *callerData)
{
    DVASSERT((vertexShader == 0) && (fragmentShader == 0) && (program == 0));
    
    if (!CompileShader(&vertexShader, GL_VERTEX_SHADER, vertexShaderData->GetSize(), (GLchar*)vertexShaderData->GetPtr(), vertexShaderDefines))
    {
        Logger::Error("Failed to compile vertex shader: %s", vertexShaderPath.GetAbsolutePathname().c_str());
        return;
    }
    
    if (!CompileShader(&fragmentShader, GL_FRAGMENT_SHADER, fragmentShaderData->GetSize(), (GLchar*)fragmentShaderData->GetPtr(), fragmentShaderDefines))
    {
        Logger::Error("Failed to compile fragment shader: %s", fragmentShaderPath.GetAbsolutePathname().c_str());
        return;
    }
    
    program = glCreateProgram();
    RENDER_VERIFY(glAttachShader(program, vertexShader));
    RENDER_VERIFY(glAttachShader(program, fragmentShader));
    
    if (!LinkProgram(program))
    {
        Logger::Error("Failed to Link program for shader: %s", fragmentShaderPath.GetAbsolutePathname().c_str());

        DeleteShaders();
        return;
    }
    
    RENDER_VERIFY(glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &activeAttributes));
    
    char attributeName[512];
    attributeNames = new FastName[activeAttributes];
    for (int32 k = 0; k < activeAttributes; ++k)
    {
        GLint size;
        GLenum type;
        RENDER_VERIFY(glGetActiveAttrib(program, k, 512, 0, &size, &type, attributeName));
        attributeNames[k] = attributeName;
        
        int32 flagIndex = GetAttributeIndexByName(attributeName);
        vertexFormatAttribIndeces[flagIndex] = glGetAttribLocation(program, attributeName);
    }
    
    RENDER_VERIFY(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniforms));
	
	SafeDeleteArray(uniformOffsets);
	SafeDeleteArray(uniformData);
	
	int32 totalSize = 0;
	uniformOffsets = new uint16[activeUniforms];
	for (int32 k = 0; k < activeUniforms; ++k)
    {
        GLint size;
        GLenum type;
        RENDER_VERIFY(glGetActiveUniform(program, k, 512, 0, &size, &type, attributeName));
		
		uniformOffsets[k] = (uint16)totalSize;
		
		int32 uniformDataSize = GetUniformTypeSize((eUniformType)type);
		totalSize += sizeof(Uniform) + (uniformDataSize * size);
	}

	uniformData = new uint8[totalSize];
	for (int32 k = 0; k < activeUniforms; ++k)
    {
        GLint size = 0;
        GLenum type = 0;
        RENDER_VERIFY(glGetActiveUniform(program, k, 512, 0, &size, &type, attributeName));
		
		Uniform* uniformStruct = GET_UNIFORM(k);
		new (&uniformStruct->name) FastName(); //VI: FastName is not a POD so a constructor should be called
		
        eUniform uniform = GetUniformByName(attributeName);
        uniformStruct->name = attributeName;
        uniformStruct->location = glGetUniformLocation(program, uniformStruct->name.c_str());
        uniformStruct->id = uniform;
        uniformStruct->type = (eUniformType)type;
        uniformStruct->size = size;
		uniformStruct->cacheValueSize = GetUniformTypeSize((eUniformType)type) * size;
		uniformStruct->cacheValue = uniformData + uniformOffsets[k] + sizeof(Uniform);
		
		//VI: initialize cacheValue with value from shader
		switch(uniformStruct->type)
		{
			case UT_FLOAT:
			{
				RENDER_VERIFY(glGetUniformfv(program, uniformStruct->location, (float32*)uniformStruct->cacheValue));
				break;
			}
				
			case UT_FLOAT_VEC2:
			{
				RENDER_VERIFY(glGetUniformfv(program, uniformStruct->location, (float32*)uniformStruct->cacheValue));
				break;
			}
				
			case UT_FLOAT_VEC3:
			{
				RENDER_VERIFY(glGetUniformfv(program, uniformStruct->location, (float32*)uniformStruct->cacheValue));
				break;
			}
				
			case UT_FLOAT_VEC4:
			{
				RENDER_VERIFY(glGetUniformfv(program, uniformStruct->location, (float32*)uniformStruct->cacheValue));
				break;
			}
				
			case UT_INT:
			{
				RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)uniformStruct->cacheValue));
				break;
			}
				
			case UT_INT_VEC2:
			{
				RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)uniformStruct->cacheValue));
				break;
			}
				
			case UT_INT_VEC3:
			{
				RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)uniformStruct->cacheValue));
				break;
			}
				
			case UT_INT_VEC4:
			{
				RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)uniformStruct->cacheValue));
				break;
			}
				
			case UT_BOOL:
			{
				RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)uniformStruct->cacheValue));
				break;
			}
				
			case UT_BOOL_VEC2:
			{
				RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)uniformStruct->cacheValue));
				break;
			}
				
			case UT_BOOL_VEC3:
			{
				RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)uniformStruct->cacheValue));
				break;
			}
				
			case UT_BOOL_VEC4:
			{
				RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)uniformStruct->cacheValue));
				break;
			}
				
			//VI: Matrices are returned from the shader in column-major order so need to transpose the matrix.
			case UT_FLOAT_MAT2:
			{
				RENDER_VERIFY(glGetUniformfv(program, uniformStruct->location, (float32*)uniformStruct->cacheValue));
				Matrix2* m = (Matrix2*)uniformStruct->cacheValue;
				Matrix2 t;
				for (int i = 0; i < 2; ++i)
					for (int j = 0; j < 2; ++j)
						t._data[i][j] = m->_data[j][i];
				*m = t;

				break;
			}
				
			case UT_FLOAT_MAT3:
			{
				RENDER_VERIFY(glGetUniformfv(program, uniformStruct->location, (float32*)uniformStruct->cacheValue));
				Matrix3* m = (Matrix3*)uniformStruct->cacheValue;
				Matrix3 t;
				for (int i = 0; i < 3; ++i)
					for (int j = 0; j < 3; ++j)
						t._data[i][j] = m->_data[j][i];
				*m = t;
				
				break;
			}
				
			case UT_FLOAT_MAT4:
			{
				RENDER_VERIFY(glGetUniformfv(program, uniformStruct->location, (float32*)uniformStruct->cacheValue));
				Matrix4* m = (Matrix4*)uniformStruct->cacheValue;
				m->Transpose();
				
				break;
			}
				
			case UT_SAMPLER_2D:
			{
				RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)uniformStruct->cacheValue));
				break;
			}
				
			case UT_SAMPLER_CUBE:
			{
				RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)uniformStruct->cacheValue));
				break;
			}
		}
    }
}
    	
void Shader::SetUniformValueByIndex(int32 uniformIndex, int32 value)
{
	DVASSERT(uniformIndex >= 0 && uniformIndex < activeUniforms);
	Uniform* currentUniform = GET_UNIFORM(uniformIndex);
	if(currentUniform->ValidateCache(value) == false)
	{
		RENDER_VERIFY(glUniform1i(currentUniform->location, value));
	}
}
	
void Shader::SetUniformValueByIndex(int32 uniformIndex, float32 value)
{
	DVASSERT(uniformIndex >= 0 && uniformIndex < activeUniforms);
	Uniform* currentUniform = GET_UNIFORM(uniformIndex);
	if(currentUniform->ValidateCache(value) == false)
	{
		RENDER_VERIFY(glUniform1f(currentUniform->location, value));
	}
}

void Shader::SetUniformValueByIndex(int32 uniformIndex, const Vector2 & vector)
{
	DVASSERT(uniformIndex >= 0 && uniformIndex < activeUniforms);
	Uniform* currentUniform = GET_UNIFORM(uniformIndex);
	if(currentUniform->ValidateCache(vector) == false)
	{
		RENDER_VERIFY(glUniform2fv(currentUniform->location, 1, &vector.x));
	}
}
	
void Shader::SetUniformValueByIndex(int32 uniformIndex, const Vector3 & vector)
{
	DVASSERT(uniformIndex >= 0 && uniformIndex < activeUniforms);
	Uniform* currentUniform = GET_UNIFORM(uniformIndex);
	if(currentUniform->ValidateCache(vector) == false)
	{
		RENDER_VERIFY(glUniform3fv(currentUniform->location, 1, &vector.x));
	}
}
	
void Shader::SetUniformColor3ByIndex(int32 uniformIndex, const Color & color)
{
	DVASSERT(uniformIndex >= 0 && uniformIndex < activeUniforms);
	Uniform* currentUniform = GET_UNIFORM(uniformIndex);
	if(currentUniform->ValidateCacheColor3(color) == false)
	{
		RENDER_VERIFY(glUniform3fv(currentUniform->location, 1, &color.r));
	}
}
	
void Shader::SetUniformColor4ByIndex(int32 uniformIndex, const Color & color)
{
	DVASSERT(uniformIndex >= 0 && uniformIndex < activeUniforms);
	Uniform* currentUniform = GET_UNIFORM(uniformIndex);
	if(currentUniform->ValidateCacheColor4(color) == false)
	{
		RENDER_VERIFY(glUniform4fv(currentUniform->location, 1, &color.r));
	}
}

void Shader::SetUniformValueByIndex(int32 uniformIndex, const Vector4 & vector)
{
	DVASSERT(uniformIndex >= 0 && uniformIndex < activeUniforms);
	Uniform* currentUniform = GET_UNIFORM(uniformIndex);
	if(currentUniform->ValidateCache(vector) == false)
	{
		RENDER_VERIFY(glUniform4fv(currentUniform->location, 1, &vector.x));
	}
}

void Shader::SetUniformValueByIndex(int32 uniformIndex, const Matrix4 & matrix)
{
	DVASSERT(uniformIndex >= 0 && uniformIndex < activeUniforms);
	Uniform* currentUniform = GET_UNIFORM(uniformIndex);
	if(currentUniform->ValidateCache(matrix) == false)
	{
		RENDER_VERIFY(glUniformMatrix4fv(currentUniform->location, 1, GL_FALSE, matrix.data));
	}
}
	
void Shader::SetUniformValueByIndex(int32 uniformIndex, const Matrix3 & matrix)
{
	DVASSERT(uniformIndex >= 0 && uniformIndex < activeUniforms);
	Uniform* currentUniform = GET_UNIFORM(uniformIndex);
	if(currentUniform->ValidateCache(matrix) == false)
	{
		RENDER_VERIFY(glUniformMatrix3fv(currentUniform->location, 1, GL_FALSE, matrix.data));
	}
}

int32 Shader::GetAttributeCount()
{
    return activeAttributes;
}
    
int32 Shader::GetAttributeIndex(eVertexFormat vertexFormat)
{
    return vertexFormatAttribIndeces[CountLeadingZeros(vertexFormat)];
}

void Shader::DeleteShaders()
{
    DVASSERT(vertexShader != 0);  
    DVASSERT(fragmentShader != 0);
    DVASSERT(program != 0);
    
	DeleteShaderContainer * container = new DeleteShaderContainer();
	container->program = program;
	container->vertexShader = vertexShader;
	container->fragmentShader = fragmentShader;
	ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &Shader::DeleteShadersInternal, container));

    vertexShader = 0;
    fragmentShader = 0;
    program = 0;
}

void Shader::DeleteShadersInternal(BaseObject * caller, void * param, void *callerData)
{
	DeleteShaderContainer * container = (DeleteShaderContainer*) param;
	DVASSERT(container);

	RENDER_VERIFY(glDetachShader(container->program, container->vertexShader));
	RENDER_VERIFY(glDetachShader(container->program, container->fragmentShader));
	RENDER_VERIFY(glDeleteShader(container->vertexShader));
	RENDER_VERIFY(glDeleteShader(container->fragmentShader));
	RENDER_VERIFY(glDeleteProgram(container->program));

	SafeDelete(container);
}

/* Link a program with all currently attached shaders */
GLint Shader::LinkProgram(GLuint prog)
{
    GLint status;
    
    RENDER_VERIFY(glLinkProgram(prog));
    
#ifdef __DAVAENGINE_DEBUG__
    {
		GLchar log[4096] = {0};
		GLsizei logLength = 0;

        RENDER_VERIFY(glGetProgramInfoLog(prog, 4096, &logLength, log));
		if (logLength)
		{
			Logger::FrameworkDebug("Program link log:\n%s", log);
		}
    }
#endif
	
    RENDER_VERIFY(glGetProgramiv(prog, GL_LINK_STATUS, &status));
    if (status == GL_FALSE)
        Logger::Error("Failed to link program %d", prog);

    return status;
}
    
/* Create and compile a shader from the provided source(s) */
GLint Shader::CompileShader(GLuint *shader, GLenum type, GLint count, const GLchar * sources, const String & defines)
{
    GLint status;
    //const GLchar *sources;
        
    *shader = glCreateShader(type);				// create shader
    
    if (defines.length() == 0)
    {
        RENDER_VERIFY(glShaderSource(*shader, 1, &sources, &count));	// set source code in the shader
    }else
    {
        const GLchar * multipleSources[] = 
        {
            defines.c_str(),
            sources,
        };
        const GLint multipleCounts[] = 
        {
            (GLint)defines.length(),
            count,
        };
        RENDER_VERIFY(glShaderSource(*shader, 2, multipleSources, multipleCounts));	// set source code in the shader
    }
    
    RENDER_VERIFY(glCompileShader(*shader));					// compile shader
    
#ifdef __DAVAENGINE_DEBUG__
	{
		GLchar log[4096] = {0};
		GLsizei logLength = 0;
		RENDER_VERIFY(glGetShaderInfoLog(*shader, 4096, &logLength, log));
		if (logLength)
		{
			Logger::FrameworkDebug("Shader compile log:\n%s", log);
		}
	}
#endif
    
    RENDER_VERIFY(glGetShaderiv(*shader, GL_COMPILE_STATUS, &status));
    if (status == GL_FALSE)
    {
        Logger::Error("Failed to compile shader: status == GL_FALSE\n");
    }
    return status;
}
    
void Shader::Unbind()
{
    if (activeProgram != 0)
    {
        RENDER_VERIFY(glUseProgram(0));
        activeProgram = 0;
    }
}
    
void Shader::Bind()
{
    if (activeProgram != program)
    {
        RENDER_VERIFY(glUseProgram(program));
        activeProgram = program;
    }
    
    for (int32 k = 0; k < activeUniforms; ++k)
    {
		Uniform* currentUniform = GET_UNIFORM(k);
		
        switch (currentUniform->id)
        {
			case UNIFORM_MODEL_VIEW_PROJECTION_MATRIX:
            {
                const Matrix4 & modelViewProj = RenderManager::Instance()->GetUniformMatrix(RenderManager::UNIFORM_MATRIX_MODELVIEWPROJECTION);
                SetUniformValueByIndex(k, modelViewProj);
                break;
            }
			case UNIFORM_MODEL_VIEW_MATRIX:
            {
                const Matrix4 & modelView = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
                SetUniformValueByIndex(k, modelView);
                break;
            }
			case UNIFORM_PROJECTION_MATRIX:
			{
                const Matrix4 & proj = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_PROJECTION);
                SetUniformValueByIndex(k, proj);
                break;
			}
			case UNIFORM_NORMAL_MATRIX:
            {
                const Matrix3 & normalMatrix = RenderManager::Instance()->GetNormalMatrix();
                SetUniformValueByIndex(k, normalMatrix);
                break;
            }
			case UNIFORM_COLOR:
            {
                const Color & c = RenderManager::Instance()->GetColor();
                SetUniformColor4ByIndex(k, c);
                break;
            }
			case UNIFORM_GLOBAL_TIME:
            {
                float32 globalTime = SystemTimer::Instance()->GetGlobalTime();
                SetUniformValueByIndex(k, globalTime);
            };
			default:
				
            break;
		}
    }
    
}
    
void Shader::Dump()
{
    Logger::FrameworkDebug("Attributes: ");
    for (int32 k = 0; k < activeAttributes; ++k)
    {
        int32 flagIndex = GetAttributeIndexByName(attributeNames[k].c_str());
        Logger::FrameworkDebug("Attribute: %s location: %d vertexFormatIndex:%x", attributeNames[k].c_str(), vertexFormatAttribIndeces[flagIndex], flagIndex);
    }
    
    RENDER_VERIFY(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniforms));
    
    
    Logger::FrameworkDebug("Uniforms: ");
    for (int32 k = 0; k < activeUniforms; ++k)
    {
		Uniform* currentUniform = GET_UNIFORM(k);
		
        //Logger::FrameworkDebug("shader uniform: %s size: %d type: %s", attributeName, size, VertexTypeStringFromEnum(type).c_str());
//        uniformNames[k] = attributeName;
//        uniformLocations[k] = glGetUniformLocation(program, uniformNames[k].c_str());
//        uniformIDs[k] = uniform;
//        uniformTypes[k] = (eUniformType)type;
        eUniform uniform = GetUniformByName(currentUniform->name.c_str());
        Logger::FrameworkDebug("uniform: %s(%d) type: %s", currentUniform->name.c_str(), uniform, VertexTypeStringFromEnum(currentUniform->type).c_str());
    }

    
}
    
Shader * Shader::RecompileNewInstance(const String & combination)
{
    Shader * shader = new Shader();
    shader->vertexShaderData = SafeRetain(vertexShaderData);
    shader->fragmentShaderData = SafeRetain(fragmentShaderData);
    shader->SetDefineList(combination);
    
	//TODO: return "invalid shader" on error;
	shader->Recompile();
	/*if (!shader->Recompile())
    {
        SafeRelease(shader);
        return 0;
    }*/
    return shader;
}

#if defined(__DAVAENGINE_ANDROID__)
void Shader::Lost()
{
	RenderResource::Lost();
	
	//DeleteShaders();

	//YZ: shader always deleted when app paused on android
	vertexShader = 0;
	fragmentShader = 0;
	if (program == activeProgram)
		activeProgram = 0;
	program = 0;
	activeAttributes = 0;
	activeUniforms = 0;
}

void Shader::Invalidate()
{
	RenderResource::Invalidate();
	Recompile();
}
#endif //#if defined(__DAVAENGINE_ANDROID__)

bool Shader::Uniform::ValidateCache(int32 value)
{
	bool result = (*(int32*)cacheValue) == value;
	
	if(!result)
	{
		DVASSERT(sizeof(value) == cacheValueSize);
		memcpy(cacheValue, &value, cacheValueSize);
	}
	
	return result;
}
	
bool Shader::Uniform::ValidateCache(float32 value)
{
	bool result = FLOAT_EQUAL(*((float32*)cacheValue), value);
	
	if(!result)
	{
		DVASSERT(sizeof(value) == cacheValueSize);
		memcpy(cacheValue, &value, cacheValueSize);
	}
	
	return result;
}

bool Shader::Uniform::ValidateCache(const Vector2 & value)
{
	Vector2& cachedVector = *(Vector2*)cacheValue;
	bool result = (value == cachedVector);
	
	if(!result)
	{
		DVASSERT(sizeof(value) == cacheValueSize);
		memcpy(cacheValue, value.data, cacheValueSize);
	}
	
	return result;
}

bool Shader::Uniform::ValidateCache(const Vector3 & value)
{
	Vector3& cachedVector = *(Vector3*)cacheValue;
	bool result = (value == cachedVector);
	
	if(!result)
	{
		DVASSERT(sizeof(value) == cacheValueSize);
		memcpy(cacheValue, value.data, cacheValueSize);
	}
	
	return result;
}
	
bool Shader::Uniform::ValidateCacheColor3(const Color & value)
{
	Color& cachedColor = *(Color*)cacheValue;
	bool result = (FLOAT_EQUAL(cachedColor.r, value.r) &&
				   FLOAT_EQUAL(cachedColor.g, value.g) &&
				   FLOAT_EQUAL(cachedColor.b, value.b));
	
	if(!result)
	{
		DVASSERT(sizeof(value) - sizeof(float32) == cacheValueSize);
		memcpy(cacheValue, &value, cacheValueSize);
	}
	
	return result;
}
	
bool Shader::Uniform::ValidateCacheColor4(const Color & value)
{
	Color& cachedColor = *(Color*)cacheValue;
	bool result = (FLOAT_EQUAL(cachedColor.r, value.r) &&
				   FLOAT_EQUAL(cachedColor.g, value.g) &&
				   FLOAT_EQUAL(cachedColor.b, value.b) &&
				   FLOAT_EQUAL(cachedColor.a, value.a));
	
	if(!result)
	{
		DVASSERT(sizeof(value) == cacheValueSize);
		memcpy(cacheValue, &value, cacheValueSize);
	}
	
	return result;
}
	
bool Shader::Uniform::ValidateCache(const Vector4 & value)
{
	Vector4& cachedVector = *(Vector4*)cacheValue;
	bool result = (value == cachedVector);
	
	if(!result)
	{
		DVASSERT(sizeof(value) == cacheValueSize);
		memcpy(cacheValue, value.data, cacheValueSize);
	}
	
	return result;
}
	
bool Shader::Uniform::ValidateCache(const Matrix4 & value)
{
	Matrix4& cachedVector = *(Matrix4*)cacheValue;
	bool result = (value == cachedVector);
	
	if(!result)
	{
		DVASSERT(sizeof(value) == cacheValueSize);
		memcpy(cacheValue, value.data, cacheValueSize);
	}
	
	return result;
}
	
bool Shader::Uniform::ValidateCache(const Matrix3 & value)
{
	Matrix3& cachedVector = *(Matrix3*)cacheValue;
	bool result = (value == cachedVector);
	
	if(!result)
	{
		DVASSERT(sizeof(value) == cacheValueSize);
		memcpy(cacheValue, value.data, cacheValueSize);
	}
	
	return result;
}

#endif



}



