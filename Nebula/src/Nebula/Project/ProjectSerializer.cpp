#include "nbpch.h"
#include "ProjectSerializer.h"

#include "Nebula/Renderer/Fonts.h"
#include "Nebula/Utils/YAML.h"

#include <fstream>

namespace Nebula {

	ProjectSerializer::ProjectSerializer(Ref<Project> project)
		: m_Project(project)
	{
	}

	bool ProjectSerializer::Serialize(const std::filesystem::path& filepath)
	{
		NB_PROFILE_FUNCTION();

		const auto& config = m_Project->GetConfig();

		YAML::Emitter out;
		out << YAML::BeginMap; // Root
		out << YAML::Key << "Project" << YAML::Value;
		
		out << YAML::BeginMap;// Project
		out << YAML::Key << "Name" << YAML::Value << config.Name;
		out << YAML::Key << "StartScene" << YAML::Value << config.StartScene.string();
		out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
		out << YAML::Key << "AssetRegistryPath" << YAML::Value << config.AssetRegistryPath.string();
		out << YAML::Key << "ScriptModulePath" << YAML::Value << config.ScriptModulePath.string();
		out << YAML::EndMap; // Project

		out << YAML::Key << "Scene" << YAML::Value;
		out << YAML::BeginMap;// Scene

		YAML::Node layers;
		for (const auto& [id, layer] : config.Layers)
			layers.push_back(layer->Name);
		layers.SetStyle(YAML::EmitterStyle::Flow);

		out << YAML::Key << "Gravity" << YAML::Value << config.Gravity;
		out << YAML::Key << "Layers" << YAML::Value << layers;
		out << YAML::EndMap; // Scene

		out << YAML::EndMap; // Root

		std::ofstream fout(filepath);
		fout << out.c_str();

		m_Project->m_AssetManager->SerializeRegistry(
			m_Project->m_ProjectDirectory / config.AssetRegistryPath);

		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		NB_PROFILE_FUNCTION();

		auto& config = m_Project->GetConfig();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::Exception e)
		{
			NB_ERROR("[Project Serializer] Failed to load file '{0}'\n     {1}", filepath.string(), e.what());
			return false;
		}

		auto projectNode = data["Project"];
		if (!projectNode)
			return false;

		DeserializeValue(config.Name, projectNode["Name"]);
		DeserializeValue(config.StartScene, projectNode["StartScene"]);
		DeserializeValue(config.AssetDirectory, projectNode["AssetDirectory"]);
		DeserializeValue(config.AssetRegistryPath, projectNode["AssetRegistryPath"]);
		DeserializeValue(config.ScriptModulePath, projectNode["ScriptModulePath"]);

		if (auto sceneNode = data["Scene"])
		{
			DeserializeValue(config.Gravity, sceneNode["Gravity"]);

			if (auto layers = sceneNode["Layers"])
			{
				int i = 0;
				for (auto& [id, layer] : config.Layers)
				{
					layer->Name = layers[i].as<std::string>();
					i++;
				}
			}
		}
		
		m_Project->m_AssetManager->DeserializeRegistry(
			m_Project->m_ProjectDirectory / config.AssetRegistryPath);

		return true;
	}
}