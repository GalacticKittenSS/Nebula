#include "nbpch.h"
#include "Prefab_Serializer.h"

#include "Nebula/Project/Project.h"
#include "Nebula/Scripting/ScriptEngine.h"
#include "Nebula/Renderer/Material.h"

#include "Nebula/Utils/YAML.h"

namespace Nebula
{
#define WRITE_SCRIPT_FIELD(FieldType, Type)			\
	case ScriptFieldType::FieldType: \
		out << field.GetValue<Type>(); \
		break

#define READ_SCRIPT_FIELD(FieldType, Type)			\
	case ScriptFieldType::FieldType: {				\
		Type data = scriptField["Data"].as<Type>(); \
		instance.SetValue(data); \
		break; \
	}

	PrefabSerializer::PrefabSerializer(Scene* scene) : m_Scene(scene) { }

	static void SerializeEntity(YAML::Emitter& out, Scene* scene, Entity entity)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Entity";

		UUID uuid = entity.GetUUID();
		out << YAML::Value << uuid;

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

		out << YAML::Key << "ParentChildComponent";
		out << YAML::BeginMap; // ParentChildComponent

		auto& component = scene->GetEntityNode(uuid);

		YAML::Node children;
		for (uint32_t i = 0; i < component.Children.size(); i++)
			children.push_back((uint64_t)component.Children[i]);
		children.SetStyle(YAML::EmitterStyle::Flow);

		out << YAML::Key << "PrimaryParent" << YAML::Value << component.Parent;
		out << YAML::Key << "Children" << YAML::Value << children;
		out << YAML::Key << "ChildCount" << YAML::Value << component.Children.size();

		out << YAML::EndMap; // ParentChildComponent
		
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
			auto& fields = ScriptEngine::GetScriptFieldMap(uuid);
			if (fields.size() > 0)
			{
				out << YAML::Key << "ScriptFields" << YAML::Value;
				out << YAML::BeginSeq;

				for (auto& [name, field] : fields)
				{
					if (field.Type == ScriptFieldType::None)
						continue;

					out << YAML::BeginMap;
					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);
					out << YAML::Key << "Data" << YAML::Value;

					switch (field.Type)
					{
						WRITE_SCRIPT_FIELD(Float, float);
						WRITE_SCRIPT_FIELD(Double, double);
						WRITE_SCRIPT_FIELD(Bool, bool);
						WRITE_SCRIPT_FIELD(Char, char);
						WRITE_SCRIPT_FIELD(Byte, int8_t);
						WRITE_SCRIPT_FIELD(Short, int16_t);
						WRITE_SCRIPT_FIELD(Int, int32_t);
						WRITE_SCRIPT_FIELD(Long, int64_t);
						WRITE_SCRIPT_FIELD(SByte, uint8_t);
						WRITE_SCRIPT_FIELD(UShort, uint16_t);
						WRITE_SCRIPT_FIELD(UInt, uint32_t);
						WRITE_SCRIPT_FIELD(ULong, uint64_t);
						WRITE_SCRIPT_FIELD(Vector2, glm::vec2);
						WRITE_SCRIPT_FIELD(Vector3, glm::vec3);
						WRITE_SCRIPT_FIELD(Vector4, glm::vec4);
					case ScriptFieldType::Entity:
					case ScriptFieldType::Prefab:
					case ScriptFieldType::Font:
					case ScriptFieldType::Texture:
					case ScriptFieldType::Asset:
						out << field.GetValueObject();
						break;
					}

					out << YAML::EndMap;
				}

				out << YAML::EndSeq;
			}

			out << YAML::EndMap; // ScriptComponent
		}
		
		auto& materialComponent = entity.GetComponent<MaterialComponent>();
		const AssetMetadata& materialMetadata = AssetManager::GetAssetMetadata(materialComponent.Material);
		if (materialMetadata.Type != AssetType::MemoryAsset) {
			out << YAML::Key << "MaterialComponent";
			out << YAML::BeginMap; // MaterialComponent

			out << YAML::Key << "Material" << YAML::Value << materialComponent.Material;
			out << YAML::EndMap; // MaterialComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>()) {
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent

			if (materialMetadata.Type == AssetType::MemoryAsset)
			{
				Ref<Material> material = AssetManager::GetAsset<Material>(materialComponent.Material);
				out << YAML::Key << "Colour" << YAML::Value << material->Colour;
				out << YAML::Key << "Tiling" << YAML::Value << material->Tiling;
			}

			auto& component = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Offset" << YAML::Value << component.SubTextureOffset;
			out << YAML::Key << "CellSize" << YAML::Value << component.SubTextureCellSize;
			out << YAML::Key << "CellNum" << YAML::Value << component.SubTextureCellNum;

			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (entity.HasComponent<CircleRendererComponent>()) {
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; // CircleRendererComponent

			auto& component = entity.GetComponent<CircleRendererComponent>();
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
			out << YAML::Key << "Font" << YAML::Value << component.FontHandle;
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
			out << YAML::Key << "Mask" << YAML::Value << bc2dComponent.Mask;

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
			out << YAML::Key << "Mask" << YAML::Value << ccComponent.Mask;

			out << YAML::EndMap; // CircleColliderComponent
		}

		out << YAML::EndMap;
	}

	static void SerializeOrder(YAML::Emitter& out, Scene* scene, Entity entity, Array<UUID>& entities)
	{
		out << entity.GetUUID();
		entities.push_back(entity.GetUUID());
		
		const auto& children = scene->GetEntityNode(entity.GetUUID());
		for (const auto& child : children.Children)
			SerializeOrder(out, scene, { child, entity }, entities);
	}

	void PrefabSerializer::Serialize(Entity entity, const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;

		Array<UUID> entities;
		
		out << YAML::Key << "Order" << YAML::Value << YAML::BeginSeq;
		SerializeOrder(out, m_Scene, entity, entities);
		out << YAML::EndSeq;

		out << YAML::Key << "Prefab" << YAML::Value << YAML::BeginSeq;
		
		for (auto& id : entities)
			SerializeEntity(out, m_Scene, { id, m_Scene });

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	static UUID DeserializeID(std::unordered_map<UUID, UUID>& idMap, const YAML::Node& node)
	{
		UUID id = DeserializeValue<uint64_t>(node);

		auto it = idMap.find(id);
		if (it == idMap.end())
			return id;

		return it->second;
	}

	Entity PrefabSerializer::Deserialize(const std::string & filepath)
	{
		YAML::Node data;
		try {
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::Exception e) {
			NB_ERROR("[Prefab Serializer] Failed to load file '{0}'\n     {1}", filepath, e.what());
			return {};
		}
		
		std::unordered_map<UUID, UUID> idMap;

		for (auto& node : data["Order"])
		{
			UUID id = node.as<uint64_t>();
			idMap[id] = UUID();
		}

		Entity returnEntity = {};

		for (auto& entity : data["Prefab"])
		{
			uint64_t uuid = DeserializeID(idMap, entity["Entity"]);

			std::string name;
			if (auto tagComponent = entity["TagComponent"])
				name = DeserializeValue<std::string>(tagComponent["Tag"], "Entity");

			NB_TRACE("Deserialized Prefab with name = {0}", name);
			Entity deserializedEntity = m_Scene->CreateEntity(uuid, name);

			if (!returnEntity)
				returnEntity = deserializedEntity;
#
			if (auto propComponent = entity["PropertiesComponent"]) {
				auto& prop = deserializedEntity.GetComponent<PropertiesComponent>();
				DeserializeValue(prop.Enabled, propComponent["IsEnabled"]);

				uint16_t layer = DeserializeValue(propComponent["Layer"], 1);
				prop.Layer = Project::GetActive()->GetConfig().Layers[layer];
			}

			if (auto parentComponent = entity["ParentChildComponent"]) {
				auto& pcc = m_Scene->m_Nodes.at(uuid);

				pcc.Parent = DeserializeID(idMap, parentComponent["PrimaryParent"]);
				if (pcc.Parent)
					m_Scene->m_SceneOrder.remove(uuid);

				uint32_t count = DeserializeValue<uint32_t>(parentComponent["ChildCount"]);
				auto children = parentComponent["Children"];
				for (uint32_t i = 0; i < count; i++)
					pcc.Children.push_back(DeserializeID(idMap, children[i]));
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
				
				if (auto scriptFields = scriptComponent["ScriptFields"])
				{
					ScriptFieldMap& fields = ScriptEngine::GetScriptFieldMap(uuid);

					for (auto scriptField : scriptFields)
					{
						std::string name = scriptField["Name"].as<std::string>();
						std::string typeString = scriptField["Type"].as<std::string>();
						ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

						ScriptField& instance = fields.GetScriptField(name, deserializedEntity);
						instance.Type = type;
						instance.Name = name;
						
						switch (type)
						{
							READ_SCRIPT_FIELD(Float, float);
							READ_SCRIPT_FIELD(Double, double);
							READ_SCRIPT_FIELD(Bool, bool);
							READ_SCRIPT_FIELD(Char, char);
							READ_SCRIPT_FIELD(Byte, int8_t);
							READ_SCRIPT_FIELD(Short, int16_t);
							READ_SCRIPT_FIELD(Int, int32_t);
							READ_SCRIPT_FIELD(Long, int64_t);
							READ_SCRIPT_FIELD(SByte, uint8_t);
							READ_SCRIPT_FIELD(UShort, uint16_t);
							READ_SCRIPT_FIELD(UInt, uint32_t);
							READ_SCRIPT_FIELD(ULong, uint64_t);
							READ_SCRIPT_FIELD(Vector2, glm::vec2);
							READ_SCRIPT_FIELD(Vector3, glm::vec3);
							READ_SCRIPT_FIELD(Vector4, glm::vec4);
							
						case ScriptFieldType::Entity:
						{
							UUID data = DeserializeID(idMap, scriptField["Data"]);
							instance.SetValueEntity(data);
							break;
						}
						case ScriptFieldType::Prefab:
						case ScriptFieldType::Font:
						case ScriptFieldType::Texture:
						case ScriptFieldType::Asset:
						{
							AssetHandle data = scriptField["Data"].as<uint64_t>();
							instance.SetValueAsset(data);
							break;
						}
						}
					}
				}
			}

			auto& mc = deserializedEntity.GetComponent<MaterialComponent>();
			if (auto materialComponent = entity["MaterialComponent"])
			{
				DeserializeValue(mc.Material, materialComponent["Material"]);
			}

			if (auto spriteRendererComponent = entity["SpriteRendererComponent"])
			{
				auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();

				if (!mc.Material)
				{
					Ref<Material> material = CreateRef<Material>();
					DeserializeValue(material->Colour, spriteRendererComponent["Colour"]);
					DeserializeValue(material->Tiling, spriteRendererComponent["Tiling"]);

					AssetHandle handle = DeserializeValue<UUID>(spriteRendererComponent["Texture"]);
					material->Texture = AssetManager::GetAsset<Texture2D>(handle);

					mc.Material = AssetManager::CreateMemoryAsset(material);
				}

				DeserializeValue(src.SubTextureOffset, spriteRendererComponent["Offset"]);
				DeserializeValue(src.SubTextureCellSize, spriteRendererComponent["CellSize"]);
				DeserializeValue(src.SubTextureCellNum, spriteRendererComponent["CellNum"]);
			}

			if (auto circleRendererComponent = entity["CircleRendererComponent"])
			{
				auto& crc = deserializedEntity.AddComponent<CircleRendererComponent>();

				if (!mc.Material)
				{
					Ref<Material> material = CreateRef<Material>();
					DeserializeValue(material->Colour, circleRendererComponent["Colour"]);
					mc.Material = AssetManager::CreateMemoryAsset(material);
				}

				DeserializeValue(crc.Thickness, circleRendererComponent["Thickness"]);
				DeserializeValue(crc.Fade, circleRendererComponent["Fade"]);
			}

			if (auto stringRendererComponent = entity["StringRendererComponent"])
			{
				auto& src = deserializedEntity.AddComponent<StringRendererComponent>();
				DeserializeValue(src.Text, stringRendererComponent["Text"]);
				DeserializeValue(src.Colour, stringRendererComponent["Colour"]);
				DeserializeValue(src.Bold, stringRendererComponent["Bold"]);
				DeserializeValue(src.Italic, stringRendererComponent["Italic"]);
				DeserializeValue(src.Kerning, stringRendererComponent["Kerning"]);
				DeserializeValue(src.LineSpacing, stringRendererComponent["LineSpacing"]);
				src.FontHandle = DeserializeValue<uint64_t>(stringRendererComponent["Font"]);
			}

			if (auto rigidbody2DComponent = entity["Rigidbody2DComponent"])
			{
				auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
				DeserializeValue(rb2d.FixedRotation, rigidbody2DComponent["FixedRotation"]);
				DeserializeValue(rb2d.Trigger, rigidbody2DComponent["IsTrigger"]);
				
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
				DeserializeValue(bc2d.Mask, box2DComponent["Mask"]);
			}

			if (auto circleColliderComponent = entity["CircleColliderComponent"]) 
			{
				auto& cc = deserializedEntity.AddComponent<CircleColliderComponent>();
				DeserializeValue(cc.Offset, circleColliderComponent["Offset"]);
				DeserializeValue(cc.Radius, circleColliderComponent["Radius"]);
				DeserializeValue(cc.Density, circleColliderComponent["Density"]);
				DeserializeValue(cc.Friction, circleColliderComponent["Friction"]);
				DeserializeValue(cc.Restitution, circleColliderComponent["Restitution"]);
				DeserializeValue(cc.RestitutionThreshold, circleColliderComponent["RestitutionThreshold"]);
				DeserializeValue(cc.Mask, circleColliderComponent["Mask"]);
			}

			deserializedEntity.CalculateTransform();

			if (deserializedEntity.HasComponent<Rigidbody2DComponent>())
				m_Scene->UpdateBox2DBody(deserializedEntity);
		}

		return returnEntity;
	}
}