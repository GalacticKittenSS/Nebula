#include "nbpch.h"
#include "Project.h"

#include "ProjectSerializer.h"

namespace Nebula {
	Project::Project()
	{
		ProjectLayer::Options layers[] = {
				ProjectLayer::A, ProjectLayer::B, ProjectLayer::C, ProjectLayer::D,
				ProjectLayer::E, ProjectLayer::F, ProjectLayer::G, ProjectLayer::H,
				ProjectLayer::I, ProjectLayer::J, ProjectLayer::K, ProjectLayer::L,
				ProjectLayer::M, ProjectLayer::N, ProjectLayer::O, ProjectLayer::P
		};

		uint32_t i = 0;
		for (ProjectLayer::Options l : layers)
		{
			Ref<ProjectLayer> Layer = CreateRef<ProjectLayer>();
			Layer->Identity = l;
			Layer->Name = "Layer " + std::to_string(i);

			m_Config.Layers[l] = Layer;
			i++;
		}
	}

	Ref<Project> Project::New()
	{
		s_ActiveProject = CreateRef<Project>();
		return s_ActiveProject;
	}

	Ref<Project> Project::Load(const std::filesystem::path& path) 
	{
		Ref<Project> project = CreateRef<Project>();
		project->m_AssetManager = CreateRef<AssetManagerBase>();
		project->m_ProjectFile = path;
		project->m_ProjectDirectory = path.parent_path();
		
		ProjectSerializer serializer(project);
		if (serializer.Deserialize(path))
		{
			s_ActiveProject = project;
			return s_ActiveProject;
		}
		
		return nullptr;
	}

	bool Project::SaveActive(const std::filesystem::path& path)
	{
		NB_ASSERT(!path.empty());

		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Serialize(path))
		{
			s_ActiveProject->m_ProjectDirectory = path.parent_path();
			return true;
		}

		return false;
	}
}