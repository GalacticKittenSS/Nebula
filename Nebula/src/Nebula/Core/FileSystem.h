#pragma once

#include "Nebula/Core/Buffer.h"

#include <filesystem>

namespace Nebula {

	class FileSystem
	{
	public:
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
	};
}