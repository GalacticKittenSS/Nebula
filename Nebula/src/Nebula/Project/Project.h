#pragma once

#include "Nebula/AssetManager/AssetManager.h"

namespace Nebula {
	struct ProjectConfig
	{
		std::string Name = "Untitled";

		std::filesystem::path AssetDirectory;
		std::filesystem::path ScriptModulePath;

		std::filesystem::path StartScene;
	};

	class Project
	{
	public:
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

		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{
			NB_ASSERT(s_ActiveProject);
			return GetAssetDirectory() / path;
		}

		static const Ref<AssetManager>& GetAssetManager() { return s_ActiveProject->m_AssetManager; }

		ProjectConfig& GetConfig() { return m_Config; }

		static Ref<Project> GetActive() { return s_ActiveProject; }

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);
	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;
		Ref<AssetManager> m_AssetManager;

		inline static Ref<Project> s_ActiveProject;
	};
}