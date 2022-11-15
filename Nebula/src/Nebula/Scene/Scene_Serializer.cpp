#include "nbpch.h"
#include "Scene_Serializer.h"

#include "Entity.h"
#include "Nebula/Scripting/ScriptEngine.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace YAML {
	template<>
	struct convert<Nebula::vec2>
	{
		static Node encode(const Nebula::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, Nebula::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert <Nebula::vec3>
	{
		static Node encode(const Nebula::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, Nebula::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Nebula::vec4>
	{
		static Node encode(const Nebula::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, Nebula::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
	
	template<>
	struct convert<Nebula::UUID>
	{
		static Node encode(const Nebula::UUID& uuid)
		{
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, Nebula::UUID& uuid)
		{
			uuid = node.as<uint64_t>();
			return true;
		}
	};

}

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

	YAML::Emitter& operator<<(YAML::Emitter& out, const vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const vec3& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const vec4& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w <<YAML::EndSeq;
		return out;
	}

	static std::string RigidBody2DBodyTypeToString(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Rigidbody2DComponent::BodyType::Static:    return "Static";
		case Rigidbody2DComponent::BodyType::Dynamic:   return "Dynamic";
		case Rigidbody2DComponent::BodyType::Kinematic: return "Kinematic";
		}

		NB_ASSERT(false, "Unknown body type");
		return {};
	}

	static Rigidbody2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
	{
		if (bodyTypeString == "Static")    return Rigidbody2DComponent::BodyType::Static;
		if (bodyTypeString == "Dynamic")   return Rigidbody2DComponent::BodyType::Dynamic;
		if (bodyTypeString == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;

		NB_ASSERT(false, "Unknown body type");
		return Rigidbody2DComponent::BodyType::Static;
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

					Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());
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
							WRITE_SCRIPT_FIELD(Vector2, vec2);
							WRITE_SCRIPT_FIELD(Vector3, vec3);
							WRITE_SCRIPT_FIELD(Vector4, vec4);
							WRITE_SCRIPT_FIELD(Entity, UUID);
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
				out << YAML::Key << "Texture" << YAML::Value << component.Texture->GetPath();
			
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
			out << YAML::Key << "FontIndex" << YAML::Value << component.FontTypeIndex;
			out << YAML::Key << "Bold" << YAML::Value << component.Bold;
			out << YAML::Key << "Italic" << YAML::Value << component.Italic;
			out << YAML::Key << "Resolution" << YAML::Value << component.Resolution;
			
			out << YAML::EndMap; // StringRendererComponent
		}

		if (entity.HasComponent<Rigidbody2DComponent>()) {
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent

			auto& rb2dComponent = entity.GetComponent<Rigidbody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;

			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (entity.HasComponent<BoxCollider2DComponent>()) {
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // BoxCollider2DComponent

			auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
			out << YAML::Key << "Category" << YAML::Value << (int)bc2dComponent.Category;
			out << YAML::Key << "Mask" << YAML::Value << bc2dComponent.Mask;

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
			out << YAML::Key << "Category" << YAML::Value << (int)ccComponent.Category;
			out << YAML::Key << "Mask" << YAML::Value << ccComponent.Mask;

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

	bool SceneSerializer::Deserialize(const std::string& filepath) {
		YAML::Node data;
		try {
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e) {
			NB_ERROR("[Scene Sereliazer] Failed to load file '{0}'\n     {1}", filepath, e.what());
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
				name = tagComponent["Tag"].as<std::string>();

			NB_TRACE("Deserialized Entity with ID = {0}, name = {1}", uuid, name);

			Entity deserializedEntity = m_Scene->CreateEntity(uuid, name);

			if (auto parentComponent = entity["ParentChildComponent"]) {
				auto& pcc = deserializedEntity.GetComponent<ParentChildComponent>();
				pcc.Parent = parentComponent["PrimaryParent"].as<uint64_t>();
				if (pcc.Parent)
					m_Scene->m_SceneOrder.remove(uuid);

				uint32_t count = parentComponent["ChildCount"].as<uint32_t>();
				auto children = parentComponent["Children"];
				for (uint32_t i = 0; i < count; i++)
					pcc.AddChild(children[i].as<uint64_t>());
			}

			if (auto transformComponent = entity["TransformComponent"]) {
				auto& tc = deserializedEntity.GetComponent<TransformComponent>();
				tc.Translation = transformComponent["Translation"].as<vec3>();
				tc.Rotation = transformComponent["Rotation"].as<vec3>();
				tc.Scale = transformComponent["Scale"].as<vec3>();
			}

			if (auto cameraComponent = entity["CameraComponent"])
			{
				auto& cc = deserializedEntity.AddComponent<CameraComponent>();

				auto& cameraProps = cameraComponent["Camera"];
				cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

				cc.Camera.SetPerspectiveFOV(cameraProps["PerspectiveFOV"].as<float>());
				cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
				cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

				cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
				cc.Camera.SetOrthoNearClip(cameraProps["OrthographicNear"].as<float>());
				cc.Camera.SetOrthoFarClip(cameraProps["OrthographicFar"].as<float>());

				cc.Primary = cameraComponent["Primary"].as<bool>();
				cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
			}
			
			if (auto scriptComponent = entity["ScriptComponent"])
			{
				auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
				sc.ClassName = scriptComponent["Class"].as<std::string>();

				auto scriptFields = scriptComponent["ScriptFields"];
				if (scriptFields)
				{
					if (Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName))
					{
						const auto& fields = entityClass->GetFields();
						Ref<ScriptInstance> scriptInstance = ScriptEngine::CreateScriptInstance(deserializedEntity);

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
								READ_SCRIPT_FIELD(Vector2,	vec2);
								READ_SCRIPT_FIELD(Vector3,	vec3);
								READ_SCRIPT_FIELD(Vector4,	vec4);
								READ_SCRIPT_FIELD(Entity,	UUID);
							}
						}
					}
				}
			}

			if (auto spriteRendererComponent = entity["SpriteRendererComponent"])
			{
				auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
				src.Colour = spriteRendererComponent["Colour"].as<vec4>();
				src.Tiling = spriteRendererComponent["Tiling"].as<float>();
				src.SubTextureOffset = spriteRendererComponent["Offset"].as<vec2>();
				src.SubTextureCellSize = spriteRendererComponent["CellSize"].as<vec2>();
				src.SubTextureCellNum = spriteRendererComponent["CellNum"].as<vec2>();

				if (spriteRendererComponent["Texture"])
					src.Texture = Texture2D::Create(spriteRendererComponent["Texture"].as<std::string>());
			}

			if (auto circleRendererComponent = entity["CircleRendererComponent"])
			{
				auto& crc = deserializedEntity.AddComponent<CircleRendererComponent>();
				crc.Colour = circleRendererComponent["Colour"].as<vec4>();
				crc.Thickness = circleRendererComponent["Thickness"].as<float>();
				crc.Fade = circleRendererComponent["Fade"].as<float>();
			}

			if (auto stringRendererComponent = entity["StringRendererComponent"])
			{
				auto& src = deserializedEntity.AddComponent<StringRendererComponent>();
				src.Text = stringRendererComponent["Text"].as<std::string>();
				src.Colour = stringRendererComponent["Colour"].as<vec4>();
				src.FontTypeIndex = stringRendererComponent["FontIndex"].as<int>();
				src.Bold = stringRendererComponent["Bold"].as<bool>();
				src.Italic = stringRendererComponent["Italic"].as<bool>();
				src.Resolution = stringRendererComponent["Resolution"].as<float>();

				src.InitiateFont();
			}

			if (auto rigidbody2DComponent = entity["Rigidbody2DComponent"])
			{
				auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
				rb2d.Type = RigidBody2DBodyTypeFromString(rigidbody2DComponent["BodyType"].as<std::string>());
				rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
			}

			if (auto box2DComponent = entity["Box2DComponent"])
			{
				auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();
				bc2d.Offset = box2DComponent["Offset"].as<vec2>();
				bc2d.Size = box2DComponent["Size"].as<vec2>();
				bc2d.Category = (Rigidbody2DComponent::Filters)box2DComponent["Category"].as<int>();
				bc2d.Mask = box2DComponent["Mask"].as<int>();

				bc2d.Density = box2DComponent["Density"].as<float>();
				bc2d.Friction = box2DComponent["Friction"].as<float>();
				bc2d.Restitution = box2DComponent["Restitution"].as<float>();
				bc2d.RestitutionThreshold = box2DComponent["RestitutionThreshold"].as<float>();
			}

			if (auto circleColliderComponent = entity["CircleColliderComponent"]) {
				auto& cc = deserializedEntity.AddComponent<CircleColliderComponent>();
				cc.Offset = circleColliderComponent["Offset"].as<vec2>();
				cc.Radius = circleColliderComponent["Radius"].as<float>();
				cc.Category = (Rigidbody2DComponent::Filters)circleColliderComponent["Category"].as<int>();
				cc.Mask = circleColliderComponent["Mask"].as<int>();

				cc.Density = circleColliderComponent["Density"].as<float>();
				cc.Friction = circleColliderComponent["Friction"].as<float>();
				cc.Restitution = circleColliderComponent["Restitution"].as<float>();
				cc.RestitutionThreshold = circleColliderComponent["RestitutionThreshold"].as<float>();
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

		for (auto entity : m_Scene->GetAllEntitiesWith<TransformComponent>())
			Entity{ entity, m_Scene.get() }.CalculateTransform();
		
		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath) {
		NB_ASSERT(false, "");
		return false;
	}
}