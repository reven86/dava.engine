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
#ifdef USE_CRC_COMPARE
#include "Utils/CRC32.h"
#endif

namespace DAVA
{
#if defined(__DAVAENGINE_OPENGL__)
	GLuint Shader::activeProgram = 0;
	
	bool IsAutobindUniform(Shader::eUniform uniformId)
	{
		return (uniformId == Shader::UNIFORM_MODEL_VIEW_PROJECTION_MATRIX ||
				uniformId == Shader::UNIFORM_MODEL_VIEW_MATRIX ||
				uniformId == Shader::UNIFORM_PROJECTION_MATRIX ||
				uniformId == Shader::UNIFORM_NORMAL_MATRIX ||
				uniformId == Shader::UNIFORM_COLOR ||
				uniformId == Shader::UNIFORM_GLOBAL_TIME);
	}
	
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
		autobindUniforms = NULL;
		autobindUniformCount = 0;
		
		for (int32 ki = 0; ki < VERTEX_FORMAT_STREAM_MAX_COUNT; ++ki)
			vertexFormatAttribIndeces[ki] = -1;
		
		vertexShaderData = 0;
		fragmentShaderData = 0;
        
        lastProjectionMatrixCache = 0;
        lastModelViewMatrixCache = 0;
        lastMVPMatrixModelViewCache = 0;
        lastMVPMatrixProjectionCache = 0;
		
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
        FastName("none"),
        FastName("modelViewProjectionMatrix"),
        FastName("modelViewMatrix"),
		FastName("projectionMatrix"),
        FastName("normalMatrix"),
        FastName("flatColor"),
        FastName("globalTime"),
    };
	
	FastName attributeStrings[VERTEX_FORMAT_STREAM_MAX_COUNT] =
    {
        FastName("inPosition"),
        FastName("inNormal"),
        FastName("inColor"),
        FastName("inTexCoord0"),
        FastName("inTexCoord1"),
        FastName("inTexCoord2"),
        FastName("inTexCoord3"),
        FastName("inTangent"),
        FastName("inBinormal"),
        FastName("inJointWeight"),
		FastName("inTime")
    };
    
	Shader::eUniform Shader::GetUniformByName(const FastName & name)
	{
		for (int32 k = 0; k < UNIFORM_COUNT; ++k)
			if (name == uniformStrings[k])return (Shader::eUniform)k;
		return Shader::UNIFORM_NONE;
	};
    
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
		
//    Logger::Debug("shader loaded:%s load-time: %lld ms", pathname.c_str(), shaderLoadTime);
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
	
	bool Shader::IsReady()
	{
		return (vertexShader != 0 && fragmentShader != 0 && program != 0);
	}
	
	void Shader::RecompileInternal(BaseObject * caller, void * param, void *callerData)
	{
		bool silentDelete = (param != NULL);
		
		if(silentDelete &&
		   ((vertexShader != 0) || (fragmentShader != 0) || (program != 0)))
		{
			//VI: be a man: just delete shader and recompile instead of complaining with assert
			//VI: such behavior is needed for Landscape since it uses shaders directly but doesn't own them
			DeleteShaders();
		}
		else
		{
			DVASSERT((vertexShader == 0) && (fragmentShader == 0) && (program == 0));
		}
		
		if (!CompileShader(&vertexShader, GL_VERTEX_SHADER, vertexShaderData->GetSize(), (GLchar*)vertexShaderData->GetPtr(), vertexShaderDefines))
		{
			Logger::Error("Failed to compile vertex shader: %s", vertexShaderPath.GetAbsolutePathname().c_str());
			return;
		}
		
		if (!CompileShader(&fragmentShader, GL_FRAGMENT_SHADER, fragmentShaderData->GetSize(), (GLchar*)fragmentShaderData->GetPtr(), fragmentShaderDefines))
		{
			Logger::Error("Failed to compile fragment shader: %s", fragmentShaderPath.GetAbsolutePathname().c_str());
			return ;
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
			attributeNames[k] = FastName(attributeName);
			
			int32 flagIndex = GetAttributeIndexByName(attributeNames[k]);
			vertexFormatAttribIndeces[flagIndex] = glGetAttribLocation(program, attributeName);
		}
		
		RENDER_VERIFY(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniforms));
		
		SafeDeleteArray(uniformOffsets);
		SafeDeleteArray(uniformData);
		SafeDeleteArray(autobindUniforms);
		
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
		autobindUniformCount = 0;
		for (int32 k = 0; k < activeUniforms; ++k)
		{
			GLint size = 0;
			GLenum type = 0;
			RENDER_VERIFY(glGetActiveUniform(program, k, 512, 0, &size, &type, attributeName));
			
			Uniform* uniformStruct = GET_UNIFORM(k);
			new (&uniformStruct->name) FastName(); //VI: FastName is not a POD so a constructor should be called
			
			FastName attrName(attributeName);
			eUniform uniform = GetUniformByName(attrName);
			uniformStruct->name = attrName;
			uniformStruct->location = glGetUniformLocation(program, uniformStruct->name.c_str());
			uniformStruct->id = uniform;
			uniformStruct->type = (eUniformType)type;
			uniformStruct->size = size;

            void* value = uniformData + uniformOffsets[k] + sizeof(Uniform);
            uint16 valueSize = GetUniformTypeSize((eUniformType)type) * size;
#ifdef USE_CRC_COMPARE
            uniformStruct->crc = CRC32::ForBuffer((const char*)(value), valueSize);
#else
            uniformStruct->cacheValueSize = valueSize;
			uniformStruct->cacheValue = value;
#endif
#ifdef USE_NEON_MATRIX_COMPARE
            uniformStruct->matrixCRC = vmovq_n_u32(0);
#endif
			
			if(IsAutobindUniform(uniform))
			{
				autobindUniformCount++;
			}
			
			//VI: initialize cacheValue with value from shader
			switch(uniformStruct->type)
			{
				case UT_FLOAT:
				{
					RENDER_VERIFY(glGetUniformfv(program, uniformStruct->location, (float32*)value));
					break;
				}
					
				case UT_FLOAT_VEC2:
				{
					RENDER_VERIFY(glGetUniformfv(program, uniformStruct->location, (float32*)value));
					break;
				}
					
				case UT_FLOAT_VEC3:
				{
					RENDER_VERIFY(glGetUniformfv(program, uniformStruct->location, (float32*)value));
					break;
				}
					
				case UT_FLOAT_VEC4:
				{
					RENDER_VERIFY(glGetUniformfv(program, uniformStruct->location, (float32*)value));
					break;
				}
					
				case UT_INT:
				{
					RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)value));
					break;
				}
					
				case UT_INT_VEC2:
				{
					RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)value));
					break;
				}
					
				case UT_INT_VEC3:
				{
					RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)value));
					break;
				}
					
				case UT_INT_VEC4:
				{
					RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)value));
					break;
				}
					
				case UT_BOOL:
				{
					RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)value));
					break;
				}
					
				case UT_BOOL_VEC2:
				{
					RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)value));
					break;
				}
					
				case UT_BOOL_VEC3:
				{
					RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)value));
					break;
				}
					
				case UT_BOOL_VEC4:
				{
					RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)value));
					break;
				}
					
					//VI: Matrices are returned from the shader in column-major order so need to transpose the matrix.
				case UT_FLOAT_MAT2:
				{
					RENDER_VERIFY(glGetUniformfv(program, uniformStruct->location, (float32*)value));
					Matrix2* m = (Matrix2*)value;
					Matrix2 t;
					for (int i = 0; i < 2; ++i)
						for (int j = 0; j < 2; ++j)
							t._data[i][j] = m->_data[j][i];
					*m = t;
					
					break;
				}
					
				case UT_FLOAT_MAT3:
				{
					RENDER_VERIFY(glGetUniformfv(program, uniformStruct->location, (float32*)value));
					Matrix3* m = (Matrix3*)value;
					Matrix3 t;
					for (int i = 0; i < 3; ++i)
						for (int j = 0; j < 3; ++j)
							t._data[i][j] = m->_data[j][i];
					*m = t;
					
					break;
				}
					
				case UT_FLOAT_MAT4:
				{
					RENDER_VERIFY(glGetUniformfv(program, uniformStruct->location, (float32*)value));
					Matrix4* m = (Matrix4*)value;
					m->Transpose();
					
					break;
				}
					
				case UT_SAMPLER_2D:
				{
					RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)value));
					break;
				}
					
				case UT_SAMPLER_CUBE:
				{
					RENDER_VERIFY(glGetUniformiv(program, uniformStruct->location, (int32*)value));
					break;
				}
			}
		}
		
		if(autobindUniformCount)
		{
			size_t autobindUniformIndex = 0;
			autobindUniforms = new Uniform*[autobindUniformCount];
			for (int32 k = 0; k < activeUniforms; ++k)
			{
				Uniform* currentUniform = GET_UNIFORM(k);
				if(IsAutobindUniform(currentUniform->id))
				{
					autobindUniforms[autobindUniformIndex] = currentUniform;
					autobindUniformIndex++;
				}
			}
		}		
	}
    
	bool Shader::Recompile(bool silentDelete)
	{
		ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN,
															   Message(this, &Shader::RecompileInternal, (silentDelete) ? this : NULL));
        JobInstanceWaiter waiter(job);
        waiter.Wait();
		
		return true;
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
	
	void Shader::SetUniformValueByIndex(int32 uniformIndex, eUniformType uniformType, uint32 arraySize, void * data)
	{
		DVASSERT(uniformIndex >= 0 && uniformIndex < activeUniforms);
		Uniform* currentUniform = GET_UNIFORM(uniformIndex);
#ifdef USE_CRC_COMPARE
        int32 size = GetUniformTypeSize((eUniformType)currentUniform->type) * currentUniform->size;
        if(currentUniform->ValidateCache(data, size) == false)
#else
		if(currentUniform->ValidateCache(data, currentUniform->cacheValueSize) == false)
#endif
		{
			switch(uniformType)
			{
				case Shader::UT_FLOAT:
					RENDER_VERIFY(glUniform1fv(currentUniform->location, arraySize, (float*)data));
					break;
				case Shader::UT_FLOAT_VEC2:
					RENDER_VERIFY(glUniform2fv(currentUniform->location, arraySize, (float*)data));
					break;
				case Shader::UT_FLOAT_VEC3:
					RENDER_VERIFY(glUniform3fv(currentUniform->location, arraySize, (float*)data));
					break;
				case Shader::UT_FLOAT_VEC4:
					RENDER_VERIFY(glUniform4fv(currentUniform->location, arraySize, (float*)data));
					break;
				case Shader::UT_INT:
					RENDER_VERIFY(glUniform1iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_INT_VEC2:
					RENDER_VERIFY(glUniform2iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_INT_VEC3:
					RENDER_VERIFY(glUniform3iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_INT_VEC4:
					RENDER_VERIFY(glUniform4iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_BOOL:
					RENDER_VERIFY(glUniform1iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_BOOL_VEC2:
					RENDER_VERIFY(glUniform2iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_BOOL_VEC3:
					RENDER_VERIFY(glUniform3iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_BOOL_VEC4:
					RENDER_VERIFY(glUniform4iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_FLOAT_MAT2:
					RENDER_VERIFY(glUniformMatrix2fv(currentUniform->location, arraySize, GL_FALSE, (float32*)data));
					break;
				case Shader::UT_FLOAT_MAT3:
					RENDER_VERIFY(glUniformMatrix3fv(currentUniform->location, arraySize, GL_FALSE, (float32*)data));
					break;
				case Shader::UT_FLOAT_MAT4:
					RENDER_VERIFY(glUniformMatrix4fv(currentUniform->location, arraySize, GL_FALSE, (float32*)data));
					break;
				case Shader::UT_SAMPLER_2D:
					RENDER_VERIFY(glUniform1iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_SAMPLER_CUBE:
					RENDER_VERIFY(glUniform1iv(currentUniform->location, arraySize, (int32*)data));
					break;
			}
		}
		
	}
	
	void Shader::SetUniformValueByUniform(Uniform* currentUniform, int32 value)
	{
		if(currentUniform->ValidateCache(value) == false)
		{
			RENDER_VERIFY(glUniform1i(currentUniform->location, value));
		}
	}
	
	void Shader::SetUniformValueByUniform(Uniform* currentUniform, float32 value)
	{
		if(currentUniform->ValidateCache(value) == false)
		{
			RENDER_VERIFY(glUniform1f(currentUniform->location, value));
		}
	}
	
	void Shader::SetUniformValueByUniform(Uniform* currentUniform, const Vector2 & vector)
	{
		if(currentUniform->ValidateCache(vector) == false)
		{
			RENDER_VERIFY(glUniform2fv(currentUniform->location, 1, &vector.x));
		}
	}
	
	void Shader::SetUniformValueByUniform(Uniform* currentUniform, const Vector3 & vector)
	{
		if(currentUniform->ValidateCache(vector) == false)
		{
			RENDER_VERIFY(glUniform3fv(currentUniform->location, 1, &vector.x));
		}
	}
	
	void Shader::SetUniformColor3ByUniform(Uniform* currentUniform, const Color & color)
	{
		if(currentUniform->ValidateCacheColor3(color) == false)
		{
			RENDER_VERIFY(glUniform3fv(currentUniform->location, 1, &color.r));
		}
	}
	
	void Shader::SetUniformColor4ByUniform(Uniform* currentUniform, const Color & color)
	{
		if(currentUniform->ValidateCacheColor4(color) == false)
		{
			RENDER_VERIFY(glUniform4fv(currentUniform->location, 1, &color.r));
		}
	}
	
	void Shader::SetUniformValueByUniform(Uniform* currentUniform, const Vector4 & vector)
	{
		if(currentUniform->ValidateCache(vector) == false)
		{
			RENDER_VERIFY(glUniform4fv(currentUniform->location, 1, &vector.x));
		}
	}
	
	void Shader::SetUniformValueByUniform(Uniform* currentUniform, const Matrix4 & matrix)
	{
		if(currentUniform->ValidateCache(matrix) == false)
		{
			RENDER_VERIFY(glUniformMatrix4fv(currentUniform->location, 1, GL_FALSE, matrix.data));
		}
	}
	
	void Shader::SetUniformValueByUniform(Uniform* currentUniform, const Matrix3 & matrix)
	{
		if(currentUniform->ValidateCache(matrix) == false)
		{
			RENDER_VERIFY(glUniformMatrix3fv(currentUniform->location, 1, GL_FALSE, matrix.data));
		}
	}
	
	void Shader::SetUniformValueByUniform(Uniform* currentUniform, eUniformType uniformType, uint32 arraySize, void * data)
	{
#ifdef USE_CRC_COMPARE
        int32 size = GetUniformTypeSize((eUniformType)currentUniform->type) * currentUniform->size;
        if(currentUniform->ValidateCache(data, size) == false)
#else
		if(currentUniform->ValidateCache(data, currentUniform->cacheValueSize) == false)
#endif
		{
			switch(uniformType)
			{
				case Shader::UT_FLOAT:
					RENDER_VERIFY(glUniform1fv(currentUniform->location, arraySize, (float*)data));
					break;
				case Shader::UT_FLOAT_VEC2:
					RENDER_VERIFY(glUniform2fv(currentUniform->location, arraySize, (float*)data));
					break;
				case Shader::UT_FLOAT_VEC3:
					RENDER_VERIFY(glUniform3fv(currentUniform->location, arraySize, (float*)data));
					break;
				case Shader::UT_FLOAT_VEC4:
					RENDER_VERIFY(glUniform4fv(currentUniform->location, arraySize, (float*)data));
					break;
				case Shader::UT_INT:
					RENDER_VERIFY(glUniform1iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_INT_VEC2:
					RENDER_VERIFY(glUniform2iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_INT_VEC3:
					RENDER_VERIFY(glUniform3iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_INT_VEC4:
					RENDER_VERIFY(glUniform4iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_BOOL:
					RENDER_VERIFY(glUniform1iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_BOOL_VEC2:
					RENDER_VERIFY(glUniform2iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_BOOL_VEC3:
					RENDER_VERIFY(glUniform3iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_BOOL_VEC4:
					RENDER_VERIFY(glUniform4iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_FLOAT_MAT2:
					RENDER_VERIFY(glUniformMatrix2fv(currentUniform->location, arraySize, GL_FALSE, (float32*)data));
					break;
				case Shader::UT_FLOAT_MAT3:
					RENDER_VERIFY(glUniformMatrix3fv(currentUniform->location, arraySize, GL_FALSE, (float32*)data));
					break;
				case Shader::UT_FLOAT_MAT4:
					RENDER_VERIFY(glUniformMatrix4fv(currentUniform->location, arraySize, GL_FALSE, (float32*)data));
					break;
				case Shader::UT_SAMPLER_2D:
					RENDER_VERIFY(glUniform1iv(currentUniform->location, arraySize, (int32*)data));
					break;
				case Shader::UT_SAMPLER_CUBE:
					RENDER_VERIFY(glUniform1iv(currentUniform->location, arraySize, (int32*)data));
					break;
			}
		}
		
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
            RenderManager::Instance()->GetStats().shaderBindCount++;
			RENDER_VERIFY(glUseProgram(program));
			activeProgram = program;
		}
		
		//for (int32 k = 0; k < activeUniforms; ++k)
		//{
		//	Uniform* currentUniform = GET_UNIFORM(k);
		for(uint8 k = 0; k < autobindUniformCount; ++k)
		{
			Uniform* currentUniform = autobindUniforms[k];
			
			switch (currentUniform->id)
			{
				case UNIFORM_MODEL_VIEW_PROJECTION_MATRIX:
				{
                    uint32 projectionMatrixCache = RenderManager::Instance()->GetProjectionMatrixCache();
                    uint32 modelViewMatrixCache = RenderManager::Instance()->GetModelViewMatrixCache();
                    if (modelViewMatrixCache == 0   ||
                        lastMVPMatrixModelViewCache != modelViewMatrixCache    ||
                        lastMVPMatrixProjectionCache != projectionMatrixCache)
                    {
                        const Matrix4 & modelViewProj = RenderManager::Instance()->GetUniformMatrix(RenderManager::UNIFORM_MATRIX_MODELVIEWPROJECTION);
                        RENDER_VERIFY(glUniformMatrix4fv(currentUniform->location, 1, GL_FALSE, modelViewProj.data));
                        lastMVPMatrixModelViewCache = modelViewMatrixCache;
                        lastMVPMatrixProjectionCache = projectionMatrixCache;
                    }
					break;
				}
				case UNIFORM_MODEL_VIEW_MATRIX:
				{
                    uint32 modelViewMatrixCache = RenderManager::Instance()->GetModelViewMatrixCache();
                    if (modelViewMatrixCache == 0   ||
                        lastModelViewMatrixCache != modelViewMatrixCache)
                    {
                        const Matrix4 & modelView = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
                        RENDER_VERIFY(glUniformMatrix4fv(currentUniform->location, 1, GL_FALSE, modelView.data));
                        lastModelViewMatrixCache = modelViewMatrixCache;
                    }
					break;
				}
				case UNIFORM_PROJECTION_MATRIX:
				{
                    uint32 projectionMatrixCache = RenderManager::Instance()->GetProjectionMatrixCache();
                    if (lastProjectionMatrixCache != projectionMatrixCache)
                    {
                        const Matrix4 & proj = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_PROJECTION);
                        RENDER_VERIFY(glUniformMatrix4fv(currentUniform->location, 1, GL_FALSE, proj.data));
                        lastProjectionMatrixCache = projectionMatrixCache;
                    }
					break;
				}
				case UNIFORM_NORMAL_MATRIX:
				{
					const Matrix3 & normalMatrix = RenderManager::Instance()->GetNormalMatrix();
					SetUniformValueByUniform(currentUniform, normalMatrix);
					break;
				}
				case UNIFORM_COLOR:
				{
					const Color & c = RenderManager::Instance()->GetColor();
					SetUniformColor4ByUniform(currentUniform, c);
					break;
				}
				case UNIFORM_GLOBAL_TIME:
				{
					float32 globalTime = SystemTimer::Instance()->GetGlobalTime();
					SetUniformValueByUniform(currentUniform, globalTime);
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
			int32 flagIndex = GetAttributeIndexByName(attributeNames[k]);
			Logger::FrameworkDebug("Attribute: %s location: %d vertexFormatIndex:%x", attributeNames[k].c_str(), vertexFormatAttribIndeces[flagIndex], flagIndex);
		}
		
		RENDER_VERIFY(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniforms));
		
		
		Logger::FrameworkDebug("Uniforms: ");
		for (int32 k = 0; k < activeUniforms; ++k)
		{
			Uniform* currentUniform = GET_UNIFORM(k);
			
        //Logger::Debug("shader uniform: %s size: %d type: %s", attributeName, size, VertexTypeStringFromEnum(type).c_str());
			//        uniformNames[k] = attributeName;
			//        uniformLocations[k] = glGetUniformLocation(program, uniformNames[k].c_str());
			//        uniformIDs[k] = uniform;
			//        uniformTypes[k] = (eUniformType)type;
			eUniform uniform = GetUniformByName(currentUniform->name);
			Logger::FrameworkDebug("uniform: %s(%d) type: %s", currentUniform->name.c_str(), uniform, VertexTypeStringFromEnum(currentUniform->type).c_str());
		}
	}
    
	Shader * Shader::CompileShader(Data * vertexShaderData, Data * fragmentShaderData, const FastNameSet & definesSet)
	{
		Shader * shader = new Shader();
		shader->vertexShaderData = SafeRetain(vertexShaderData);
		shader->fragmentShaderData = SafeRetain(fragmentShaderData);
		
		String result;
		FastNameSet::iterator end = definesSet.end();
		for (FastNameSet::iterator it = definesSet.begin(); it != end; ++it)
		{
			const FastName & fname = it->first;
			result += Format("#define %s\n", fname.c_str());
		}
		shader->SetDefines(result);
		
		
		shader->Recompile();
		return shader;
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
#ifdef USE_CRC_COMPARE
        return ValidateCache(&value, sizeof(int32));
#else
		bool result = (*(int32*)cacheValue) == value;
		
		if(!result)
		{
			DVASSERT(sizeof(value) == cacheValueSize);
			memcpy(cacheValue, &value, cacheValueSize);
		}
		
		return result;
#endif
	}
	
	bool Shader::Uniform::ValidateCache(float32 value)
	{
#ifdef USE_CRC_COMPARE
        return ValidateCache(&value, sizeof(float32));
#else
		bool result = FLOAT_EQUAL(*((float32*)cacheValue), value);
		
		if(!result)
		{
			DVASSERT(sizeof(value) == cacheValueSize);
			memcpy(cacheValue, &value, cacheValueSize);
		}
		
		return result;
#endif
	}
	
	bool Shader::Uniform::ValidateCache(const Vector2 & value)
	{
#ifdef USE_CRC_COMPARE
        return ValidateCache(&value.data, sizeof(float32) * 2);
#else
		Vector2& cachedVector = *(Vector2*)cacheValue;
		bool result = (value == cachedVector);
		
		if(!result)
		{
			DVASSERT(sizeof(value) == cacheValueSize);
			memcpy(cacheValue, value.data, cacheValueSize);
		}
		
		return result;
#endif
	}
	
	bool Shader::Uniform::ValidateCache(const Vector3 & value)
	{
#ifdef USE_CRC_COMPARE
        return ValidateCache(&value.data, sizeof(float32) * 3);
#else
		Vector3& cachedVector = *(Vector3*)cacheValue;
		bool result = (value == cachedVector);
		
		if(!result)
		{
			DVASSERT(sizeof(value) == cacheValueSize);
			memcpy(cacheValue, value.data, cacheValueSize);
		}
		
		return result;
#endif
	}
	
	bool Shader::Uniform::ValidateCacheColor3(const Color & value)
	{
#ifdef USE_CRC_COMPARE
        return ValidateCache(&value.color, sizeof(float32) * 3);
#else
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
#endif
	}
	
	bool Shader::Uniform::ValidateCacheColor4(const Color & value)
	{
#ifdef USE_CRC_COMPARE
        return ValidateCache(&value.color, sizeof(float32) * 4);
#else
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
#endif
	}
	
	bool Shader::Uniform::ValidateCache(const Vector4 & value)
	{
#ifdef USE_CRC_COMPARE
        return ValidateCache(&value.data, sizeof(float32) * 4);
#else
		Vector4& cachedVector = *(Vector4*)cacheValue;
		bool result = (value == cachedVector);
		
		if(!result)
		{
			DVASSERT(sizeof(value) == cacheValueSize);
			memcpy(cacheValue, value.data, cacheValueSize);
		}
		
		return result;
#endif
	}
	
	bool Shader::Uniform::ValidateCache(const Matrix4 & value)
	{
#ifdef USE_CRC_COMPARE
#ifdef USE_NEON_MATRIX_COMPARE
        uint64 a0;// = (uint32)value._00 << 32 | (uint32)value._11;
        memcpy(&a0, &value._00, sizeof(float32));
        a0 = a0 << 32;
        memcpy(&a0, &value._11, sizeof(float32));
        uint64 a1;// = (uint32)value._22 << 32 | (uint32)value._33;
        memcpy(&a1, &value._22, sizeof(float32));
        a1 = a1 << 32;
        memcpy(&a1, &value._23, sizeof(float32));
        uint64 a2;// = (uint32)value._03 << 32 | (uint32)value._13;
        memcpy(&a2, &value._03, sizeof(float32));
        a2 = a2 << 32;
        memcpy(&a1, &value._13, sizeof(float32));
        uint64 a3;// = (uint32)value._20 << 32 | (uint32)value._33;
        memcpy(&a3, &value._23, sizeof(float32));
        a3 = a3 << 32;
        memcpy(&a3, &value._32, sizeof(float32));
        
        static poly8x8_t lhs1 = vcreate_p8(0xB1E6B0926655004F);
        poly8x8_t rhs1 = vcreate_p8(a0);
        poly8x8_t rhs2 = vcreate_p8(a1);
        poly8x8_t rhs3 = vcreate_p8(a2);
        poly8x8_t rhs4 = vcreate_p8(a3);
        
        // first 16 bit multiply
        poly16x8_t l1m1 = vmull_p8(lhs1, rhs1);
        poly16x8_t l1m2 = vmull_p8(lhs1, rhs2);
        poly16x4x2_t l1z1 = vuzp_p16(vget_low_p16(l1m2), vget_high_p16(l1m2));
        uint16x4_t l1e1 = veor_u16(vreinterpret_u16_p16(l1z1.val[0]), vreinterpret_u16_p16(l1z1.val[1]));
        
        uint32x4_t l1s1 = vshll_n_u16(l1e1, 8);
        uint32x4_t l1f1 = veorq_u32(l1s1, vreinterpretq_u32_p16(l1m1));
        
        // second 16 bit multiply
        poly16x8_t l1m3 = vmull_p8(lhs1, rhs3);
        poly16x8_t l1m4 = vmull_p8(lhs1, rhs4);
        poly16x4x2_t l1z2 = vuzp_p16(vget_low_p16(l1m4), vget_high_p16(l1m4));
        uint16x4_t l1e2 = veor_u16(vreinterpret_u16_p16(l1z2.val[0]), vreinterpret_u16_p16(l1z2.val[1]));
        uint32x4_t l1s2 = vshll_n_u16(l1e2, 8);
        uint32x4_t l1f2 = veorq_u32(l1s2, vreinterpretq_u32_p16(l1m3));

        // now combine the two 16 bit multiplies
        uint32x2_t l1e3 = veor_u32(vget_low_u32(l1f2), vget_high_u32(l1f2));
        uint64x2_t l1s3 = vshll_n_u32(l1e3, 16);
        uint64x2_t l1e4 = veorq_u64(l1s3, vreinterpretq_u64_u32(l1f1));
        // l1e4 = "vmull_p32"(lhs1, rhs1);

        uint32x4_t crc = vreinterpretq_u32_u64(l1e4);
        uint32x4_t cmp = vceqq_u32(crc, matrixCRC);
        
        uint32_t res = 0xffffffff;
        res &= vgetq_lane_u32(cmp, 0);
        res &= vgetq_lane_u32(cmp, 1);
        res &= vgetq_lane_u32(cmp, 2);
        res &= vgetq_lane_u32(cmp, 3);

        if (res != 0xffffffff)
        {
            matrixCRC = cmp;
            return false;
        }
        
        return true;
#else   //#ifdef USE_NEON_MATRIX_COMPARE
        return ValidateCache(&value.data, sizeof(float32) * 16);
#endif  //#ifdef USE_NEON_MATRIX_COMPARE
        
#else   //#ifdef USE_CRC_COMPARE
        Matrix4& cachedVector = *(Matrix4*)cacheValue;
        bool result = (value == cachedVector);

        if (!result)
        {
            DVASSERT(sizeof(value) == cacheValueSize);
            memcpy(cacheValue, value.data, cacheValueSize);
        }

        return result;
#endif
	}
	
	bool Shader::Uniform::ValidateCache(const Matrix3 & value)
	{
#ifdef USE_CRC_COMPARE
        uint32 crc32 = CRC32::ForBuffer((const char*)value.data, sizeof(float32) * 9);
        bool result = crc == crc32;
        if (!result)
        {
            crc = crc32;
        }
#else
		Matrix3& cachedVector = *(Matrix3*)cacheValue;
		bool result = (value == cachedVector);
		
		if(!result)
		{
			DVASSERT(sizeof(value) == cacheValueSize);
			memcpy(cacheValue, value.data, cacheValueSize);
		}
#endif
		return result;
	}
	
	bool Shader::Uniform::ValidateCache(const void* value, uint16 valueSize)
	{
#ifdef USE_CRC_COMPARE
        uint32 crc32 = CRC32::ForBuffer((const char*)value, valueSize);
        bool result = crc == crc32;
        if (!result)
        {
            crc = crc32;
        }
#else
		DVASSERT(valueSize >= cacheValueSize);
		
		bool result = false;
		if(cacheValueSize == valueSize)
		{
			result = (memcmp(value, cacheValue, cacheValueSize) == 0);
			if(!result)
			{
				memcpy(cacheValue, value, cacheValueSize);
			}
		}
#endif
		return result;
	}
	
#endif
	
	
	
}



