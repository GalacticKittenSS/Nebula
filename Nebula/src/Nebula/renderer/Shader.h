#pragma once

#include <string>

namespace Nebula {
	class Shader {
	public:
		Shader(const std::string& vertSrc, const std::string& fragSrc);
		~Shader();

		void Bind() const;
		void Unbind() const;
	private:
		uint32_t m_RendererID;
	};
}