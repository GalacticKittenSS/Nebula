#pragma once

#include "ProjectLayer.h"

#include "Nebula/AssetManager/AssetManagerBase.h"

#include <map>

namespace Nebula {
	struct ProjectConfig
	{
		std::string Name = "Untitled";

		std::filesystem::path StartScene;
		std::filesystem::path AssetDirectory = "Asset";
		std::filesystem::path AssetRegistryPath = "Registry.yaml";
		std::filesystem::path ScriptModulePath;

		// Scene
		glm::vec2 Gravity = { 0.0f, -9.81f };
		std::map<uint16_t, Ref<ProjectLayer>> Layers;

		// Scene Renderer
		glm::vec4 ClearColour = { 0.1f, 0.1f, 0.1f, 1.0f };
		bool ShowSky = true;
	};

	class Project
	{
	public:
		Project();
		ProjectConfig& GetConfig() { return m_Config; }

		const std::filesystem::path& GetProjectDirectory() { return m_ProjectDirectory; }
		const std::filesystem::path& GetProjectFilePath() { return m_ProjectFile; }
		std::filesystem::path GetAssetDirectory() { return GetProjectDirectory() / m_Config.AssetDirectory; }
		std::filesystem::path GetScriptModulePath() { return GetProjectDirectory() / m_Config.ScriptModulePath; }
		std::filesystem::path GetAssetRegistryPath() { return GetProjectDirectory() / m_Config.AssetRegistryPath; }
		
		static const std::filesystem::path& GetActiveProjectDirectory()
		{
			NB_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetProjectDirectory();
		}

		static const std::filesystem::path& GetActiveProjectFilePath()
		{
			NB_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetProjectFilePath();
		}

		static std::filesystem::path GetActiveAssetDirectory()
		{
			NB_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetAssetDirectory();
		}

		static std::filesystem::path GetActiveScriptModulePath()
		{
			NB_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetScriptModulePath();
		}
		
		static std::filesystem::path GetActiveAssetRegistryPath()
		{
			NB_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetAssetRegistryPath();
		}

		static std::filesystem::path GetAssetPath(const std::filesystem::path& path)
		{
			NB_ASSERT(s_ActiveProject);
			return s_ActiveProject->GetAssetDirectory() / path;
		}

		static Ref<Project> GetActive() { return s_ActiveProject; }
		static const Ref<AssetManagerBase>& GetAssetManager() { return s_ActiveProject->m_AssetManager; }

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);
	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;
		std::filesystem::path m_ProjectFile;
		Ref<AssetManagerBase> m_AssetManager;

		inline static Ref<Project> s_ActiveProject;
		friend class ProjectSerializer;
	};
}