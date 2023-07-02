#include "nbpch.h"
#include "Material.h"

#include "Nebula/Utils/YAML.h"

namespace Nebula
{
	MaterialSerializer::MaterialSerializer(Ref<Material> mat)
		: m_Material(mat) 
	{
	}

	void MaterialSerializer::Serialize(const std::string& filepath)
	{
		NB_PROFILE_FUNCTION();

		YAML::Emitter out;
		out << YAML::BeginMap; // Root

		out << YAML::Key << "Material";
		out << YAML::Value << YAML::BeginMap; // Material

		out << YAML::Key << "Colour" << YAML::Value << m_Material->Colour;
		out << YAML::Key << "Tiling" << YAML::Value << m_Material->Tiling;

		if (m_Material->Texture)
			out << YAML::Key << "Texture" << YAML::Value << m_Material->Texture->Handle;
		
		out << YAML::EndMap; // Material
		out << YAML::EndMap; // Root

		std::ofstream fout(filepath);
		fout << out.c_str();
	}
	
	bool MaterialSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::Exception e)
		{
			NB_ERROR("[Material Serializer] Failed to load file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		auto materialNode = data["Material"];
		if (!materialNode)
			return false;

		DeserializeValue(m_Material->Colour, materialNode["Colour"]);
		DeserializeValue(m_Material->Tiling, materialNode["Tiling"]);
		
		AssetHandle handle = DeserializeValue<UUID>(materialNode["Texture"]);
		m_Material->Texture = AssetManager::GetAsset<Texture2D>(handle);
		return true;
	}

}