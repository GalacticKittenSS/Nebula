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

		static const std::filesystem::path& GetProjectDirectory()
		{
			NB_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ProjectDirectory;
		}

		static std::filesystem::path GetAssetDirectory()
		{
			NB_ASSERT(s_ActiveProject);
			return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
		}

		static std::filesystem::path GetScriptModulePath()
		{
			NB_ASSERT(s_ActiveProject);
			return GetProjectDirectory() / s_ActiveProject->m_Config.ScriptModulePath;
		}
		
		static std::filesystem::path GetAssetRegistryPath()
		{
			NB_ASSERT(s_ActiveProject);
			return GetProjectDirectory() / s_ActiveProject->m_Config.AssetRegistryPath;
		}

		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{
			NB_ASSERT(s_ActiveProject);
			return GetAssetDirectory() / path;
		}

		static std::filesystem::path GetProjectFile() {
			NB_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ProjectFile;
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