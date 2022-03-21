#include "nbpch.h"
#include "OpenGL_Shader.h"

#include <fstream>
#include <glad/glad.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include <filesystem>

#include "Nebula/Utils/Time.h"

namespace Nebula {
	namespace Utils {
		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "fragment" || type == "pixel")
				return GL_FRAGMENT_SHADER;

			NB_ASSERT(false, "Unknown shader type!");
			return 0;
		}

		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
			case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
			}
			NB_ASSERT(false, "");
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
			case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
			}
			NB_ASSERT(false, "");
			return nullptr;
		}

		static const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "Resources/cache/shader/opengl";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}

		static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:    return ".cached_opengl.vert";
			case GL_FRAGMENT_SHADER:  return ".cached_opengl.frag";
			}
			NB_ASSERT(false, "");
			return "";
		}

		static const char* GLShaderStageCachedVulkanFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:    return ".cached_vulkan.vert";
			case GL_FRAGMENT_SHADER:  return ".cached_vulkan.frag";
			}
			NB_ASSERT(false, "");
			return "";
		}

		static const bool IsAmdGpu()
		{
			const char* vendor = (char*)glGetString(GL_VENDOR);
			return strstr(vendor, "ATI") != nullptr;
		}

	}

	OpenGL_Shader::OpenGL_Shader(const std::string& filepath)
		: m_FilePath(filepath)
	{
		NB_PROFILE_FUNCTION();

		Utils::CreateCacheDirectoryIfNeeded();

		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);

		{
			Timer timer;
			CompileOrGetVulkanBinaries(shaderSources);
			if (Utils::IsAmdGpu()) {
				CreateProgramForAmd();
			}
			else {
				CompileOrGetOpenGLBinaries();
				CreateProgram();
			}
			NB_WARN("Shader creation took {0} ms", timer.elapsed() * 1000);
		}

		// Extract name from filepath
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash, count);
	}

	OpenGL_Shader::OpenGL_Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(name)
	{
		NB_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;

		CompileOrGetVulkanBinaries(sources);
		if (Utils::IsAmdGpu()) {
			CreateProgramForAmd();
		}
		else {
			CompileOrGetOpenGLBinaries();
			CreateProgram();
		}
	}

	OpenGL_Shader::~OpenGL_Shader()
	{
		NB_PROFILE_FUNCTION();

		glDeleteProgram(m_RendererID);
	}

	std::string OpenGL_Shader::ReadFile(const std::string& filepath)
	{
		NB_PROFILE_FUNCTION();

		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else
			{
				NB_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			NB_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGL_Shader::PreProcess(const std::string& source)
	{
		NB_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
			NB_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			std::string type = source.substr(begin, eol - begin);
			NB_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			NB_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

			shaderSources[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	void OpenGL_Shader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();
		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					NB_ERROR(module.GetErrorMessage());
					NB_ASSERT(false, "");
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, data] : shaderData)
			Reflect(stage, data);
	}

	void OpenGL_Shader::CompileOrGetOpenGLBinaries()
	{
		auto& shaderData = m_OpenGLSPIRV;

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		const bool optimize = false;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		shaderData.clear();
		m_OpenGLSourceCode.clear();
		for (auto&& [stage, spirv] : m_VulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				m_OpenGLSourceCode[stage] = glslCompiler.compile();
				auto& source = m_OpenGLSourceCode[stage];

				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str());
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					NB_ERROR(module.GetErrorMessage());
					NB_ASSERT(false, "");
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
	}

	void OpenGL_Shader::CreateProgram()
	{
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : m_OpenGLSPIRV)
		{
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), GLsizei(spirv.size() * sizeof(uint32_t)));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			NB_ERROR("Shader linking failed ({0}):\n{1}", m_FilePath, infoLog.data());

			glDeleteProgram(program);

			for (auto id : shaderIDs)
				glDeleteShader(id);
		}

		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_RendererID = program;
	}

	static bool VerifyProgramLink(GLenum& program)
	{
		int isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			int maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<char> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(program);

			NB_ERROR("{0}", infoLog.data());
			NB_ASSERT(false, "[OpenGL] Shader link failure!");
			return false;
		}
		return true;
	}

	void OpenGL_Shader::CreateProgramForAmd()
	{
		GLuint program = glCreateProgram();

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();
		std::filesystem::path shaderFilePath = m_FilePath;
		std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + ".cached_opengl.pgr");
		std::ifstream in(cachedPath, std::ios::ate | std::ios::binary);

		if (in.is_open())
		{
			auto size = in.tellg();
			in.seekg(0);

			auto& data = std::vector<char>(size);
			uint32_t format = 0;
			in.read((char*)&format, sizeof(uint32_t));
			in.read((char*)data.data(), size);
			glProgramBinary(program, format, data.data(), (GLsizei)data.size());

			bool linked = VerifyProgramLink(program);

			if (!linked)
				return;
		}
		else
		{
			std::array<uint32_t, 2> glShadersIDs;
			CompileOpenGLBinariesForAmd(program, glShadersIDs);
			glLinkProgram(program);

			bool linked = VerifyProgramLink(program);

			if (linked)
			{
				// Save program data
				GLint formats = 0;
				glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
				NB_ASSERT(formats > 0, "Driver does not support binary format");
				Utils::CreateCacheDirectoryIfNeeded();
				GLint length = 0;
				glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &length);
				auto shaderData = std::vector<char>(length);
				uint32_t format = 0;
				glGetProgramBinary(program, length, nullptr, &format, shaderData.data());
				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					out.write((char*)&format, sizeof(uint32_t));
					out.write(shaderData.data(), shaderData.size());
					out.flush();
					out.close();
				}
			}

			for (auto& id : glShadersIDs)
				glDetachShader(program, id);
		}

		m_RendererID = program;
	}

	void OpenGL_Shader::CompileOpenGLBinariesForAmd(GLenum& program, std::array<uint32_t, 2>& glShadersIDs)
	{
		int glShaderIDIndex = 0;
		for (auto&& [stage, spirv] : m_VulkanSPIRV)
		{
			spirv_cross::CompilerGLSL glslCompiler(spirv);
			auto& source = glslCompiler.compile();

			uint32_t shader;

			shader = glCreateShader(stage);

			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			glCompileShader(shader);

			int isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				int maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<char> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(shader);

				NB_ERROR("{0}", infoLog.data());
				NB_ASSERT(false, "[OpenGL] Shader compilation failure!");
				return;
			}
			glAttachShader(program, shader);
			glShadersIDs[glShaderIDIndex++] = shader;
		}
	}

	void OpenGL_Shader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		NB_TRACE("OpenGL_Shader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_FilePath);
		NB_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		NB_TRACE("    {0} resources", resources.sampled_images.size());

		NB_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			int memberCount = (int)bufferType.member_types.size();

			NB_TRACE("  {0}", resource.name);
			NB_TRACE("    Size = {0}", bufferSize);
			NB_TRACE("    Binding = {0}", binding);
			NB_TRACE("    Members = {0}", memberCount);
		}
	}

	void OpenGL_Shader::Bind() const
	{
		NB_PROFILE_FUNCTION();

		glUseProgram(m_RendererID);
	}

	void OpenGL_Shader::Unbind() const
	{
		NB_PROFILE_FUNCTION();

		glUseProgram(0);
	}

	void OpenGL_Shader::SetBackfaceCulling(bool cull) {
		if (cull) {
			glEnable(GL_CULL_FACE);
		} else {
			glDisable(GL_CULL_FACE);
		}
	}

	void OpenGL_Shader::SetInt(const std::string& name, int value)
	{
		NB_PROFILE_FUNCTION();

		UploadUniformInt(name, value);
	}

	void OpenGL_Shader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{
		UploadUniformIntArray(name, values, count);
	}

	void OpenGL_Shader::SetFloat(const std::string& name, float value)
	{
		NB_PROFILE_FUNCTION();

		UploadUniformFloat(name, value);
	}

	void OpenGL_Shader::SetFloat2(const std::string& name, const vec2& value)
	{
		NB_PROFILE_FUNCTION();

		UploadUniformFloat2(name, value);
	}

	void OpenGL_Shader::SetFloat3(const std::string& name, const vec3& value)
	{
		NB_PROFILE_FUNCTION();

		UploadUniformFloat3(name, value);
	}

	void OpenGL_Shader::SetFloat4(const std::string& name, const vec4& value)
	{
		NB_PROFILE_FUNCTION();

		UploadUniformFloat4(name, value);
	}

	void OpenGL_Shader::SetMat4(const std::string& name, const mat4& value)
	{
		NB_PROFILE_FUNCTION();

		UploadUniformMat4(name, value);
	}

	void OpenGL_Shader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGL_Shader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGL_Shader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGL_Shader::UploadUniformFloat2(const std::string& name, const vec2& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void OpenGL_Shader::UploadUniformFloat3(const std::string& name, const vec3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGL_Shader::UploadUniformFloat4(const std::string& name, const vec4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGL_Shader::UploadUniformMat3(const std::string& name, const mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(matrix));
	}

	void OpenGL_Shader::UploadUniformMat4(const std::string& name, const mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(matrix));
	}

}