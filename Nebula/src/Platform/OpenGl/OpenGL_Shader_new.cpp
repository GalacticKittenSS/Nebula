#include "nbpch.h"
#include "OpenGL_Shader.h"

#include <fstream>
#include <glad/glad.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include <filesystem>

#include "Nebula/Core/Time.h"

namespace Nebula {
	namespace Utils {
		static GLenum ShaderTypeFromString(const std::string& type) {
			NB_PROFILE_FUNCTION();

			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "fragment")
				return GL_FRAGMENT_SHADER;

			NB_ASSERT(false, "Unknown Shader Type!");
			return 0;
		}
		
		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage) {
			switch (stage)
			{
				case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
				case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
			}
			NB_ASSERT(false, "Unknown Shader Type!");
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
				case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
			}
			NB_ASSERT(false, "Unknown Shader Type!");
			return nullptr;
		}

		static const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "assets/cache/shader/opengl";
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
			NB_ASSERT(false, "Unknown Shader Type!");
			return "";
		}

		static const char* GLShaderStageCachedVulkanFileExtension(uint32_t stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:    return ".cached_vulkan.vert";
				case GL_FRAGMENT_SHADER:  return ".cached_vulkan.frag";
			}
			NB_ASSERT(false, "Unknown Shader Type!");
			return "";
		}
	}

	OpenGL_Shader::OpenGL_Shader(const std::string& path) : m_FilePath(path) {
		NB_PROFILE_FUNCTION();

		Utils::CreateCacheDirectoryIfNeeded();

		std::string source = ReadFile(path);
		auto shaderSources = PreProcess(source);

		{
			Timer timer;
			CompileOrGetVulkanBinaries(shaderSources);
			CompileOrGetOpenGLBinaries();
			CreateProgram();
			NB_WARN("Shader creation took {0} ms", timer.elapsed());
		}
		
		//Extract Name from File Path
		auto lastSlash = path.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = path.rfind(".");
		auto count = lastDot == std::string::npos ? path.size() - lastSlash: lastDot - lastSlash;
		m_Name = path.substr(lastSlash, count);
	}

	OpenGL_Shader::OpenGL_Shader(const std::string& name, const std::string& vertSrc, const std::string& fragSrc): m_Name(name) {
		NB_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertSrc;
		sources[GL_FRAGMENT_SHADER] = fragSrc;

		CompileOrGetVulkanBinaries(sources);
		CompileOrGetOpenGLBinaries();
		CreateProgram();
	}

	OpenGL_Shader::~OpenGL_Shader() {
		NB_PROFILE_FUNCTION();

		glDeleteProgram(m_RendererID);
	}

	std::string OpenGL_Shader::ReadFile(const std::string& path) {
		NB_PROFILE_FUNCTION();

		std::string result;
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (in) {
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1) {
				result.resize(in.tellg());
				in.seekg(0, std::ios::beg);
				in.read(&result[0], result.size());
				in.close();
			}
			else {
				NB_ERROR("Could not read from file {0}", path);
			}
		}
		else {
			NB_ERROR("Could not load file {0}", path);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGL_Shader::PreProcess(const std::string& source) {
		NB_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> ShaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
		while (pos != std::string::npos) {
			size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
			NB_ASSERT(eol != std::string::npos, "[Shader] Syntax Error");
			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			std::string type = source.substr(begin, eol - begin);
			NB_ASSERT(Utils::ShaderTypeFromString(type), "[Shader] Invalid Shader Type Specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			NB_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

			ShaderSources[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return ShaderSources;
	}


	void OpenGL_Shader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources) {
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
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
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

	void OpenGL_Shader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		NB_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_FilePath);
		NB_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		NB_TRACE("    {0} resources", resources.sampled_images.size());

		NB_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			int memberCount = bufferType.member_types.size();

			NB_TRACE("  {0}", resource.name);
			NB_TRACE("    Size = {0}", bufferSize);
			NB_TRACE("    Binding = {0}", binding);
			NB_TRACE("    Members = {0}", memberCount);
		}
	}

	void OpenGL_Shader::SetInt(const std::string& name, const int value) {
		UploadUniformInt(name, value);
	}

	void OpenGL_Shader::SetIntArray(const std::string& name, int* values, uint32_t count) {
		UploadUniformIntArray(name, values, count);
	}
	
	void OpenGL_Shader::SetFloat(const std::string& name, const float value) {
		UploadUniformFloat(name, value);
	}
	
	void OpenGL_Shader::SetMat4(const std::string& name, const mat4& value) {
		UploadUniformMat4(name, value);
	}

	void OpenGL_Shader::SetFloat3(const std::string& name, const vec3& values) {
		UploadUniformFloat3(name, values);
	}

	void OpenGL_Shader::SetFloat4(const std::string& name, const vec4& values) {
		UploadUniformFloat4(name, values);
	}

	void OpenGL_Shader::Bind() const {
		glUseProgram(m_RendererID);
	}

	void OpenGL_Shader::Unbind() const {
		glUseProgram(0);
	}

	void OpenGL_Shader::UploadUniformInt(const std::string& name, const int value) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGL_Shader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGL_Shader::UploadUniformFloat(const std::string& name, const float value) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGL_Shader::UploadUniformFloat2(const std::string& name, const vec2& values) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, values.x, values.y);
	}

	void OpenGL_Shader::UploadUniformFloat3(const std::string& name, const vec3& values) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, values.x, values.y, values.z);
	}

	void OpenGL_Shader::UploadUniformFloat4(const std::string& name, const vec4& values) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, values.x, values.y, values.z, values.w);
	}
	
	//void OpenGL_Shader::UploadUniformMat3(const std::string& name, const mat3& matrix) {
	//	GLint location = glGetUniformLocation(m_RendererID, name.c_str());
	//	glUniformMatrix3fv(location, 1, GL_FALSE, &matrix);
	//}

	void OpenGL_Shader::UploadUniformMat4(const std::string& name, const mat4& matrix) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, &(matrix[0].x));
	}
}