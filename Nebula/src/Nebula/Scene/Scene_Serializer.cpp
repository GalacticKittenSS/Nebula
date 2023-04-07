#include "nbpch.h"
#include "Scene_Serializer.h"

#include "Nebula/Utils/YAML.h"

#include "Entity.h"
#include "Nebula/Project/Project.h"
#include "Nebula/Scripting/ScriptEngine.h"

#include <fstream>

namespace Nebula {
#define WRITE_SCRIPT_FIELD(FieldType, Type)			\
	case ScriptFieldType::FieldType: \
		out << scriptInstance->GetFieldValue<Type>(name); \
		break

#define READ_SCRIPT_FIELD(FieldType, Type)			\
	case ScriptFieldType::FieldType: {				\
		Type data = scriptField["Data"].as<Type>(); \
		scriptInstance->SetFieldValue(name, data); \
		break; \
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene): m_Scene(scene) { }

	static void SerializeEntity(YAML::Emitter& out, Entity entity) {
		out << YAML::BeginMap;
		out << YAML::Key << "Entity";
		out << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>()) {
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<PropertiesComponent>()) {
			out << YAML::Key << "PropertiesComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& component = entity.GetComponent<PropertiesComponent>();
			out << YAML::Key << "IsEnabled" << YAML::Value << component.Enabled;
			out << YAML::Key << "Layer" << YAML::Value << component.Layer->Identity;

			out << YAML::EndMap; // TransformComponent
		}

		if (entity.HasComponent<ParentChildComponent>()) {
			out << YAML::Key << "ParentChildComponent";
			out << YAML::BeginMap; // ParentChildComponent

			auto& component = entity.GetComponent<ParentChildComponent>();

			YAML::Node children;
			for (uint32_t i = 0; i < component.ChildrenIDs.size(); i++)
				children.push_back((uint64_t)component[i]);
			children.SetStyle(YAML::EmitterStyle::Flow);
			
			out << YAML::Key << "PrimaryParent" << YAML::Value << component.Parent;
			out << YAML::Key << "Children" << YAML::Value << children;
			out << YAML::Key << "ChildCount" << YAML::Value << component.ChildrenIDs.size();

			out << YAML::EndMap; // ParentChildComponent
		}

		if (entity.HasComponent<TransformComponent>()) {
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& component = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << component.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << component.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << component.Scale;

			out << YAML::EndMap; // TransformComponent
		}
		
		if (entity.HasComponent<CameraComponent>()) {
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap;

			auto& camera = entity.GetComponent<CameraComponent>().Camera;
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthoNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthoFarClip();
			out << YAML::EndMap;

			auto& component = entity.GetComponent<CameraComponent>();
			out << YAML::Key << "Primary" << YAML::Value << component.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << component.FixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}

		if (entity.HasComponent<ScriptComponent>()) {
			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap; // ScriptComponent

			auto& component = entity.GetComponent<ScriptComponent>();
			out << YAML::Key << "Class" << YAML::Value << component.ClassName;

			// Fields
			if (Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(component.ClassName))
			{
				const auto& fields = entityClass->GetFields();
				if (fields.size() > 0)
				{
					out << YAML::Key << "ScriptFields" << YAML::Value;
					out << YAML::BeginSeq;

					Ref<ScriptInstance> scriptInstance = ScriptEngine::GetScriptInstance(entity);
					for (const auto& [name, field] : fields)
					{
						if (field.Type == ScriptFieldType::None)
							continue;

						out << YAML::BeginMap;
						out << YAML::Key << "Name" << YAML::Value << name;
						out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);
						out << YAML::Key << "Data" << YAML::Value;

						switch (field.Type)
						{
							WRITE_SCRIPT_FIELD(Float,	float);
							WRITE_SCRIPT_FIELD(Double,	double);
							WRITE_SCRIPT_FIELD(Bool,	bool);
							WRITE_SCRIPT_FIELD(Char,	char);
							WRITE_SCRIPT_FIELD(Byte,	int8_t);
							WRITE_SCRIPT_FIELD(Short,	int16_t);
							WRITE_SCRIPT_FIELD(Int,		int32_t);
							WRITE_SCRIPT_FIELD(Long,	int64_t);
							WRITE_SCRIPT_FIELD(SByte,	uint8_t);
							WRITE_SCRIPT_FIELD(UShort,	uint16_t);
							WRITE_SCRIPT_FIELD(UInt,	uint32_t);
							WRITE_SCRIPT_FIELD(ULong,	uint64_t);
							WRITE_SCRIPT_FIELD(Vector2, glm::vec2);
							WRITE_SCRIPT_FIELD(Vector3, glm::vec3);
							WRITE_SCRIPT_FIELD(Vector4, glm::vec4);
							WRITE_SCRIPT_FIELD(Entity,	UUID);
						}

						out << YAML::EndMap;
					}

					out << YAML::EndSeq;
				}
			}
			
			out << YAML::EndMap; // ScriptComponent
		}
		
		if (entity.HasComponent<SpriteRendererComponent>()) {
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent

			auto& component = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Colour" << YAML::Value << component.Colour;

			if (component.Texture)
			{
				Ref<Texture2D> texture = Project::GetAssetManager()->GetAssetData<Texture2D>(component.Texture);
				std::string relativePath = std::filesystem::relative(texture->GetPath(), Project::GetAssetDirectory()).string();
				out << YAML::Key << "Texture" << YAML::Value << relativePath;
			}
			
			out << YAML::Key << "Tiling" << YAML::Value << component.Tiling;
			out << YAML::Key << "Offset" << YAML::Value << component.SubTextureOffset;
			out << YAML::Key << "CellSize" << YAML::Value << component.SubTextureCellSize;
			out << YAML::Key << "CellNum" << YAML::Value << component.SubTextureCellNum;

			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (entity.HasComponent<CircleRendererComponent>()) {
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; // CircleRendererComponent

			auto& component = entity.GetComponent<CircleRendererComponent>();
			out << YAML::Key << "Colour" << YAML::Value << component.Colour;
			out << YAML::Key << "Thickness" << YAML::Value << component.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << component.Fade;
			
			out << YAML::EndMap; // CircleRendererComponent
		}
		
		if (entity.HasComponent<StringRendererComponent>()) {
			out << YAML::Key << "StringRendererComponent";
			out << YAML::BeginMap; // StringRendererComponent

			auto& component = entity.GetComponent<StringRendererComponent>();
			out << YAML::Key << "Text" << YAML::Value << component.Text;
			out << YAML::Key << "Colour" << YAML::Value << component.Colour;
			
			if (!component.FontHandle && component.FamilyName == "")
				out << YAML::Key << "Font" << YAML::Value << component.FamilyName;
			else
			{
				Ref<Font> font = component.GetFont();
				if (font)
					out << YAML::Key << "Font" << YAML::Value << font->GetName();
			}
			
			out << YAML::Key << "Bold" << YAML::Value << component.Bold;
			out << YAML::Key << "Italic" << YAML::Value << component.Italic;
			out << YAML::Key << "Kerning" << YAML::Value << component.Kerning;
			out << YAML::Key << "LineSpacing" << YAML::Value << component.LineSpacing;
			
			out << YAML::EndMap; // StringRendererComponent
		}

		if (entity.HasComponent<Rigidbody2DComponent>()) {
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent

			auto& rb2dComponent = entity.GetComponent<Rigidbody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;
			out << YAML::Key << "IsTrigger" << YAML::Value << rb2dComponent.Trigger;
			out << YAML::Key << "Mask" << YAML::Value << rb2dComponent.Mask;

			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (entity.HasComponent<BoxCollider2DComponent>()) {
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // BoxCollider2DComponent

			auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
			out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;

			out << YAML::EndMap; // BoxCollider2DComponent
		}

		if (entity.HasComponent<CircleColliderComponent>()) {
			out << YAML::Key << "CircleColliderComponent";
			out << YAML::BeginMap; // CircleColliderComponent

			auto& ccComponent = entity.GetComponent<CircleColliderComponent>();
			out << YAML::Key << "Offset" << YAML::Value << ccComponent.Offset;
			out << YAML::Key << "Radius" << YAML::Value << ccComponent.Radius;
			out << YAML::Key << "Density" << YAML::Value << ccComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << ccComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << ccComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << ccComponent.RestitutionThreshold;

			out << YAML::EndMap; // CircleColliderComponent
		}

		out << YAML::EndMap;
	}

	void SceneSerializer::Serialize(const std::string& filepath) {
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		
		YAML::Node order;
		for (const UUID& id : m_Scene->m_SceneOrder)
			order.push_back((uint64_t)id);
		order.SetStyle(YAML::EmitterStyle::Flow);

		out << YAML::Key << "Order";
		out << YAML::BeginMap;
		out << YAML::Key << "Size" << YAML::Value << m_Scene->m_SceneOrder.size();
		out << YAML::Key << "Data" << YAML::Value << order;
		out << YAML::EndMap;

		YAML::Node layers;
		for (const auto& [id, layer] :m_Scene->m_Layers)
			layers.push_back(layer->Name);
		layers.SetStyle(YAML::EmitterStyle::Flow);

		out << YAML::Key << "Layers" << YAML::Value << layers;
		
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		auto view = m_Scene->GetAllEntitiesWith<IDComponent>();
		for (auto entt : view) {
			Entity entity = { view.get<IDComponent>(entt).ID, m_Scene.get() };
			if (!entity)
				return; 

			SerializeEntity(out, entity);
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath) {
		NB_ASSERT(false, "");
	}

	template<typename T>
	static void DeserializeValue(T& var, const YAML::Node& val)
	{
		if (!val)
			return;

		var = val.as<T>();
	}


	template<typename T>
	static T DeserializeValue(const YAML::Node& val, T default = (T)0)
	{
		if (!val)
			return default;

		return val.as<T>();
	}

	bool SceneSerializer::Deserialize(const std::string& filepath) {
		YAML::Node data;
		try {
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::Exception e) {
			NB_ERROR("[Scene Serializer] Failed to load file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		NB_TRACE("Deserializing Scene '{0}'", sceneName);

		auto entities = data["Entities"];
		if (!entities)
			return true;

		for (auto entity : entities) {
			uint64_t uuid = entity["Entity"].as<uint64_t>();

			std::string name;
			auto tagComponent = entity["TagComponent"];
			if (tagComponent)
				name = DeserializeValue<std::string>(tagComponent["Tag"], "Entity");

			NB_TRACE("Deserialized Entity with ID = {0}, name = {1}", uuid, name);

			Entity deserializedEntity = m_Scene->CreateEntity(uuid, name);

			if (auto propComponent = entity["PropertiesComponent"]) {
				auto& prop = deserializedEntity.GetComponent<PropertiesComponent>();
				DeserializeValue(prop.Enabled, propComponent["IsEnabled"]);
				
				uint16_t layer = DeserializeValue(propComponent["Layer"], 1);
				prop.Layer = m_Scene->m_Layers[layer];
			}

			if (auto parentComponent = entity["ParentChildComponent"]) {
				auto& pcc = deserializedEntity.GetComponent<ParentChildComponent>();

				DeserializeValue(pcc.Parent, parentComponent["PrimaryParent"]);
				if (pcc.Parent)
					m_Scene->m_SceneOrder.remove(uuid);

				uint32_t count = DeserializeValue<uint32_t>(parentComponent["ChildCount"]);
				auto children = parentComponent["Children"];
				for (uint32_t i = 0; i < count; i++)
					pcc.AddChild(children[i].as<uint64_t>());
			}

			if (auto transformComponent = entity["TransformComponent"]) {
				auto& tc = deserializedEntity.GetComponent<TransformComponent>();
				DeserializeValue(tc.Translation, transformComponent["Translation"]);
				DeserializeValue(tc.Rotation, transformComponent["Rotation"]);
				DeserializeValue(tc.Scale, transformComponent["Scale"]);
			}

			if (auto cameraComponent = entity["CameraComponent"])
			{
				auto& cc = deserializedEntity.AddComponent<CameraComponent>();
				DeserializeValue(cc.Primary, cameraComponent["Primary"]);
				DeserializeValue(cc.FixedAspectRatio, cameraComponent["FixedAspectRatio"]);

				if (auto cameraProps = cameraComponent["Camera"])
				{
					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

					cc.Camera.SetPerspectiveFOV(cameraProps["PerspectiveFOV"].as<float>());
					cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthoNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Camera.SetOrthoFarClip(cameraProps["OrthographicFar"].as<float>());
				}
			}
			
			if (auto scriptComponent = entity["ScriptComponent"])
			{
				auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
				DeserializeValue(sc.ClassName, scriptComponent["Class"]);

				Ref<ScriptInstance> scriptInstance = ScriptEngine::CreateScriptInstance(deserializedEntity);
				if (auto scriptFields = scriptComponent["ScriptFields"])
				{
					if (Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName))
					{
						const auto& fields = entityClass->GetFields();
						for (auto scriptField : scriptFields)
						{
							std::string name = scriptField["Name"].as<std::string>();
							std::string typeString = scriptField["Type"].as<std::string>();
							ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

							auto it = fields.find(name);
							if (it == fields.end())
								continue; 
						
							switch (type)
							{
								READ_SCRIPT_FIELD(Float,	float);
								READ_SCRIPT_FIELD(Double,	double);
								READ_SCRIPT_FIELD(Bool,		bool);
								READ_SCRIPT_FIELD(Char,		char);
								READ_SCRIPT_FIELD(Byte,		int8_t);
								READ_SCRIPT_FIELD(Short,	int16_t);
								READ_SCRIPT_FIELD(Int,		int32_t);
								READ_SCRIPT_FIELD(Long,		int64_t);
								READ_SCRIPT_FIELD(SByte,	uint8_t);
								READ_SCRIPT_FIELD(UShort,	uint16_t);
								READ_SCRIPT_FIELD(UInt,		uint32_t);
								READ_SCRIPT_FIELD(ULong,	uint64_t);
								READ_SCRIPT_FIELD(Vector2,	glm::vec2);
								READ_SCRIPT_FIELD(Vector3,	glm::vec3);
								READ_SCRIPT_FIELD(Vector4,	glm::vec4);
								READ_SCRIPT_FIELD(Entity,	UUID);
							}
						}
					}
				}
			}

			if (auto spriteRendererComponent = entity["SpriteRendererComponent"])
			{
				auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
				DeserializeValue(src.Colour, spriteRendererComponent["Colour"]);
				DeserializeValue(src.Tiling, spriteRendererComponent["Tiling"]);
				DeserializeValue(src.SubTextureOffset, spriteRendererComponent["Offset"]);
				DeserializeValue(src.SubTextureCellSize, spriteRendererComponent["CellSize"]);
				DeserializeValue(src.SubTextureCellNum, spriteRendererComponent["CellNum"]);

				if (spriteRendererComponent["Texture"])
				{
					std::string texturePath = spriteRendererComponent["Texture"].as<std::string>();
					auto path = Project::GetAssetFileSystemPath(texturePath);
					src.Texture = Project::GetAssetManager()->ImportAsset(path);
				}
			}

			if (auto circleRendererComponent = entity["CircleRendererComponent"])
			{
				auto& crc = deserializedEntity.AddComponent<CircleRendererComponent>();
				DeserializeValue(crc.Colour, circleRendererComponent["Colour"]);
				DeserializeValue(crc.Thickness, circleRendererComponent["Thickness"]);
				DeserializeValue(crc.Fade, circleRendererComponent["Fade"]);
			}

			if (auto stringRendererComponent = entity["StringRendererComponent"])
			{
				auto& src = deserializedEntity.AddComponent<StringRendererComponent>();
				DeserializeValue(src.Text, stringRendererComponent["Text"]);
				DeserializeValue(src.Colour, stringRendererComponent["Colour"]);
				
				if (auto font = stringRendererComponent["Font"])
				{
					std::string fontName = font.as<std::string>();
					FontFamily family = FontManager::GetFamily(fontName);
					
					if (family.Name == "Unknown")
					{
						auto handles = Project::GetAssetManager()->GetAllAssetsWithType(AssetType::Font);
						for (AssetHandle handle : handles)
						{
							Ref<Font> asset = Project::GetAssetManager()->GetAssetData<Font>(handle);
							if (!asset ||
								asset->GetName() != fontName)
								continue;

							src.FontHandle = handle;
							break;
						}

						if (!src.FontHandle)
						{
							std::filesystem::path fontPath = Project::GetAssetFileSystemPath(fontName).string();
							src.FontHandle = Project::GetAssetManager()->ImportAsset(fontPath);
						}
					}
					else
					{
						src.FamilyName = family.Name;
					}
				}
				
				DeserializeValue(src.Bold, stringRendererComponent["Bold"]);
				DeserializeValue(src.Italic, stringRendererComponent["Italic"]);
				DeserializeValue(src.Kerning, stringRendererComponent["Kerning"]);
				DeserializeValue(src.LineSpacing, stringRendererComponent["LineSpacing"]);
			}

			if (auto rigidbody2DComponent = entity["Rigidbody2DComponent"])
			{
				auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
				DeserializeValue(rb2d.FixedRotation, rigidbody2DComponent["FixedRotation"]);
				DeserializeValue(rb2d.Trigger, rigidbody2DComponent["IsTrigger"]);
				DeserializeValue(rb2d.Mask, rigidbody2DComponent["Mask"]);
				
				rb2d.Type = RigidBody2DBodyTypeFromString(
					DeserializeValue<std::string>(rigidbody2DComponent["BodyType"], "Dynamic"));
			}

			if (auto box2DComponent = entity["BoxCollider2DComponent"])
			{
				auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();
				DeserializeValue(bc2d.Offset, box2DComponent["Offset"]);
				DeserializeValue(bc2d.Size, box2DComponent["Size"]);
				DeserializeValue(bc2d.Density, box2DComponent["Density"]);
				DeserializeValue(bc2d.Friction, box2DComponent["Friction"]);
				DeserializeValue(bc2d.Restitution, box2DComponent["Restitution"]);
				DeserializeValue(bc2d.RestitutionThreshold, box2DComponent["RestitutionThreshold"]);
			}

			if (auto circleColliderComponent = entity["CircleColliderComponent"]) {
				auto& cc = deserializedEntity.AddComponent<CircleColliderComponent>();
				DeserializeValue(cc.Offset, circleColliderComponent["Offset"]);
				DeserializeValue(cc.Radius, circleColliderComponent["Radius"]);
				DeserializeValue(cc.Density, circleColliderComponent["Density"]);
				DeserializeValue(cc.Friction, circleColliderComponent["Friction"]);
				DeserializeValue(cc.Restitution, circleColliderComponent["Restitution"]);
				DeserializeValue(cc.RestitutionThreshold, circleColliderComponent["RestitutionThreshold"]);
			}
		}

		if (auto order = data["Order"])
		{
			uint32_t count = order["Size"].as<uint32_t>();
			auto scene_order = order["Data"];

			m_Scene->m_SceneOrder.clear();
			for (uint32_t i = 0; i < count; i++)
				m_Scene->m_SceneOrder.push_back(scene_order[i].as<uint64_t>());
		}


		if (auto layers = data["Layers"])
		{
			int i = 0;
			for (auto& [id, layer] : m_Scene->m_Layers)
			{
				layer->Name = layers[i].as<std::string>();
				i++;
			}
		}

		for (auto entity : m_Scene->GetAllEntitiesWith<TransformComponent>())
			Entity{ entity, m_Scene.get() }.CalculateTransform();
		
		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath) {
		NB_ASSERT(false, "");
		return false;
	}
}