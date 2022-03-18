#pragma once

#include <string>

#include "Nebula/Maths/Maths.h"

namespace Nebula {
	class Shader {
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual const std::string& GetName() const = 0;
		virtual void SetBackfaceCulling(bool) = 0;

		static Ref<Shader> Create(const std::string& path);
		static Ref<Shader> Create(const std::string& name, const std::string& vertSrc, const std::string& fragSrc);

		virtual void SetInt(const std::string& name, const int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
		virtual void SetMat4(const std::string& name, const mat4& value) = 0;
		virtual void SetFloat(const std::string& name, const float value) = 0;
		virtual void SetFloat2(const std::string& name, const vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const vec3& values) = 0;
		virtual void SetFloat4(const std::string& name, const vec4& values) = 0;
	};

	class ShaderLibrary {
	public:
		void Add(const Ref<Shader>& shader);
		void Add(const std::string& name, const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filepath);
		Ref<Shader> Load(const std::string& filepath, const std::string& name);

		Ref<Shader> Get(const std::string& name);
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}