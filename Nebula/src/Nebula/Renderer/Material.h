#pragma once

#include "Texture.h"
#include "Nebula/Maths/Maths.h"

#include "Nebula/AssetManager/AssetManager.h"

namespace Nebula
{
	class Material : public Asset
	{
	public:
		glm::vec4 Colour{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture = nullptr;
		float Tiling = 1.0f;

		Material() = default;
		Material(glm::vec4 col, Ref<Texture2D> texture = nullptr, float tiling = 1.0f)
			: Colour(col), Texture(texture), Tiling(tiling) { }

		static Material Get(Ref<Material>& mat) 
		{
			if (!mat)
				return {};

			return *mat.get(); 
		}

		static AssetType GetStaticType() { return AssetType::Material; }
		virtual AssetType GetType() const { return GetStaticType(); }
	};

	class MaterialSerializer
	{
	public:
		MaterialSerializer(Ref<Material> mat);

		void Serialize(const std::string& filepath);
		bool Deserialize(const std::string& filepath);
	private:
		Ref<Material> m_Material;
	};
}