#include "nbpch.h"
#include "ProjectSerializer.h"

#include "Nebula/Renderer/Fonts.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Nebula {

	ProjectSerializer::ProjectSerializer(Ref<Project> project)
		: m_Project(project)
	{
	}

	bool ProjectSerializer::Serialize(const std::filesystem::path& filepath)
	{
		const auto& config = m_Project->GetConfig();

		YAML::Emitter out;
		out << YAML::BeginMap; // Root
		out << YAML::Key << "Project" << YAML::Value;
		
		out << YAML::BeginMap;// Project
		out << YAML::Key << "Name" << YAML::Value << config.Name;
		out << YAML::Key << "StartScene" << YAML::Value << config.StartScene.string();
		out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
		out << YAML::Key << "ScriptModulePath" << YAML::Value << config.ScriptModulePath.string();
		out << YAML::EndMap; // Project
		
		out << YAML::Key << "Assets" << YAML::Value;
		out << YAML::BeginSeq; // Assets

		const auto& assets = m_Project->GetAssetManager()->GetAssets();
		for (const auto& [handle, asset] : assets)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Handle" << handle;
			out << YAML::Key << "Path" << asset->RelativePath.string();
			out << YAML::EndMap;
		}

		out << YAML::EndSeq; // Assets

		out << YAML::EndMap; // Root

		std::ofstream fout(filepath);
		fout << out.c_str();

		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
	{
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

		config.Name = projectNode["Name"].as<std::string>();
		config.StartScene = projectNode["StartScene"].as<std::string>();
		config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
		config.ScriptModulePath = projectNode["ScriptModulePath"].as<std::string>();

		for (auto asset : data["Assets"])
		{
			uint64_t handle = asset["Handle"].as<uint64_t>();
			std::string relativePath = asset["Path"].as<std::string>();

			std::filesystem::path path = filepath.parent_path() / config.AssetDirectory / relativePath;
				
			m_Project->m_AssetManager->ImportAsset(handle, path, relativePath);
		}

		return true;
	}
}