#include "nbpch.h"
#include "OpenGL_Shader.h"

#include <fstream>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Nebula {
	static GLenum ShaderTypeFromString(const std::string& type) {
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment")
			return GL_FRAGMENT_SHADER;

		NB_ASSERT(false, "Unknown Shader Type!");
		return 0;
	}

	OpenGL_Shader::OpenGL_Shader(const std::string& path) {
		std::string source = ReadFile(path);
		auto shaderSources = PreProcess(source);
		
		bool compiled = Compile(shaderSources);

		if (!compiled) {
			NB_WARN("Could Not Compile Shaders");
		}

		//Extract Name from File Path
		auto lastSlash = path.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = path.rfind(".");
		auto count = lastDot == std::string::npos ? path.size() - lastSlash: lastDot - lastSlash;
		m_Name = path.substr(lastSlash, count);
	}

	OpenGL_Shader::OpenGL_Shader(const std::string& name, const std::string& vertSrc, const std::string& fragSrc): m_Name(name) {
		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertSrc;
		sources[GL_FRAGMENT_SHADER] = fragSrc;

		bool compiled = Compile(sources);
		
		if (!compiled) {
			NB_WARN("Could Not Compile Shaders");
		}
	}

	OpenGL_Shader::~OpenGL_Shader() {
		glDeleteProgram(m_RendererID);
	}

	std::string OpenGL_Shader::ReadFile(const std::string& path) {
		std::string result;
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (in) {
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else {
			NB_ERROR("Could not load file {0}", path);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGL_Shader::PreProcess(const std::string& source) {
		std::unordered_map<GLenum, std::string> ShaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos) {
			size_t eol = source.find_first_of("\r\n", pos);
			NB_ASSERT(eol != std::string::npos, "[Shader] Syntax Error");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			NB_ASSERT(ShaderTypeFromString(type), "[Shader] Invalid Shader Type Specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			ShaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		return ShaderSources;
	}

	bool OpenGL_Shader::Compile(std::unordered_map<GLenum, std::string> sources) {
		int isCompiled = 0; //Does the Frag/Vertex Shader Compile? 
		int isLinked = 0;	//Does the Program Link?

		GLuint program = glCreateProgram();

		NB_ASSERT(sources.size() <= 2, "More than 2 Shaders Given! Only 2 are Supported");
		std::array<GLenum, 2> glShaderIDs;
		int glShaderIndex = 0;

		for (auto& kv : sources) {
			GLenum type = kv.first;
			const std::string& source = kv.second;

			GLuint shader = glCreateShader(type);

			//Get Char from std::string
			const char* sourceC = source.c_str();
			
			//Gen and Bind Shader
			glShaderSource(shader, 1, &sourceC, 0);

			//Compile shader and get Status
			glCompileShader(shader);

			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled); 
			
			if (isCompiled == GL_FALSE) {
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(shader);

				NB_ERROR("{0}", infoLog.data());
				NB_ASSERT(false, "Shader Compilation Failure!");
				return false;
			}

			//Attach Shader to Program
			glAttachShader(program, shader);
			glShaderIDs[glShaderIndex++] = shader;
		}

		m_RendererID = program;

		//Link Program and get status
		glLinkProgram(m_RendererID);
		glGetProgramiv(m_RendererID, GL_LINK_STATUS, &isLinked);

		if (isLinked == GL_FALSE) {
			GLint maxLength = 0;
			glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(m_RendererID);

			for (auto shader : glShaderIDs) {
				glDeleteShader(shader);
			}

			NB_ERROR("{0}", infoLog.data());
			NB_ASSERT(false, "Program Shader Link Failure!");
			return false;
		}

		for (auto shader : glShaderIDs) {
			glDetachShader(m_RendererID, shader);
		}

		return true;
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

	void OpenGL_Shader::UploadUniformFloat(const std::string& name, const float value) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGL_Shader::UploadUniformFloat2(const std::string& name, const glm::vec2& values) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, values.x, values.y);
	}

	void OpenGL_Shader::UploadUniformFloat3(const std::string& name, const glm::vec3& values) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, values.x, values.y, values.z);
	}

	void OpenGL_Shader::UploadUniformFloat4(const std::string& name, const glm::vec4& values) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, values.x, values.y, values.z, values.w);
	}
	
	void OpenGL_Shader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGL_Shader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}
}