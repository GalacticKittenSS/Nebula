#pragma once

#include "Nebula/Core/API.h"
#include "Nebula/Utils/Arrays.h"
#include "Nebula/Renderer/Texture.h"

namespace Nebula {
	struct MSDFData;
	
	class Font 
	{
	public:
		Font(const std::string& name, const std::filesystem::path& filename);
		~Font();

		const MSDFData* GetMSDFData() const { return m_Data.get(); }
		inline const Ref<Texture2D> GetAtlasTexture() const { return m_AtlasTexture; }
		inline const std::filesystem::path& GetFilename() const { return m_Filename; }
		inline const std::string& GetName() const { return m_Name; }

		static Ref<Font> GetDefault();
	private:
		std::string m_Name;
		std::filesystem::path m_Filename;
		Scope<MSDFData> m_Data;
		Ref<Texture2D> m_AtlasTexture;
	};
}