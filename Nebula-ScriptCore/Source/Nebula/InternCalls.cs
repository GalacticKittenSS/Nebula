using System;
using System.Runtime.CompilerServices;

namespace Nebula
{
    internal class InternalCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Native_Log(int level, string text);

        // APPLICATION CLASS
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Application_GetWindowSize(out Vector2 size);
        
        // PREFAB CLASS

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Prefab_Create(string path);

        // ENTITY CLASS

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong entityID, Type component);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_AddComponent(ulong entityID, Type component);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Entity_GetName(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_SetName(ulong entityID, string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object Entity_GetScriptInstance(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object Entity_SetScriptInstance(ulong entityID, Type script);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_FindChildByName(ulong entityID, string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static short Entity_GetLayer(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_SetLayer(ulong entityID, short layer);

        // INPUT CLASS

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMouseButtonDown(MouseCode mouseCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetMousePos(out Vector2 pos);

        // TRANSFORM COMPONENT

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetRotation(ulong entityID, out Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetRotation(ulong entityID, ref Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetScale(ulong entityID, out Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetScale(ulong entityID, ref Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetWorldTranslation(ulong entityID, out Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetWorldRotation(ulong entityID, out Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetWorldScale(ulong entityID, out Vector3 scale);

        // CAMERA COMPONENT

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool CameraComponent_GetPrimary(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CameraComponent_SetPrimary(ulong entityID, bool primary);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool CameraComponent_GetFixedRatio(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CameraComponent_SetFixedRatio(ulong entityID, bool fixedRatio);

        // SPRITE RENDERER COMPONENT

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRendererComponent_GetColour(ulong entityID, out Vector4 colour);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRendererComponent_SetColour(ulong entityID, ref Vector4 colour);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRendererComponent_GetOffset(ulong entityID, out Vector2 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRendererComponent_SetOffset(ulong entityID, ref Vector2 offset);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRendererComponent_GetCellSize(ulong entityID, out Vector2 size);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRendererComponent_SetCellSize(ulong entityID, ref Vector2 size);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRendererComponent_GetCellNumber(ulong entityID, out Vector2 number);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRendererComponent_SetCellNumber(ulong entityID, ref Vector2 number);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float SpriteRendererComponent_GetTiling(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRendererComponent_SetTiling(ulong entityID, float tiling);

        // CIRCLE RENDERER COMPONENT

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_GetColour(ulong entityID, out Vector4 colour);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_SetColour(ulong entityID, ref Vector4 colour);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CircleRendererComponent_GetRadius(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_SetRadius(ulong entityID, float radius);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CircleRendererComponent_GetThickness(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_SetThickness(ulong entityID, float thickness);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CircleRendererComponent_GetFade(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_SetFade(ulong entityID, float fade);

        // STRING RENDERER COMPONENT

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string StringRendererComponent_GetText(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetText(ulong entityID, string index);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_GetColour(ulong entityID, out Vector4 colour);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetColour(ulong entityID, ref Vector4 colour);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool StringRendererComponent_GetBold(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetBold(ulong entityID, bool bold);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool StringRendererComponent_GetItalic(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetItalic(ulong entityID, bool italic);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string StringRendererComponent_GetFontName(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetFontName(ulong entityID, string name);

        // RIGIDBODY 2D COMPONENT

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Rigidbody2DComponent_GetBodyType(ulong entityID);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_SetBodyType(ulong entityID, int type);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static short Rigidbody2DComponent_GetMask(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_SetMask(ulong entityID, short mask);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Rigidbody2DComponent_GetFixedRotation(ulong entityID);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_SetFixedRotation(ulong entityID, bool fixedRotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse, ref Vector2 point);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impulse);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_GetLinearVelocity(ulong entityID, out Vector2 impulse);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyForce(ulong entityID, ref Vector2 force, ref Vector2 point);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyForceToCenter(ulong entityID, ref Vector2 force);
        
        // BOX COLLIDER 2D COMPONENT

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider2DComponent_GetSize(ulong entityID, out Vector2 size);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider2DComponent_SetSize(ulong entityID, ref Vector2 size);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider2DComponent_GetOffset(ulong entityID, out Vector2 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider2DComponent_SetOffset(ulong entityID, ref Vector2 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float BoxCollider2DComponent_GetDensity(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider2DComponent_SetDensity(ulong entityID, float density);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float BoxCollider2DComponent_GetFriction(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider2DComponent_SetFriction(ulong entityID, float friction);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float BoxCollider2DComponent_GetRestitution(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider2DComponent_SetRestitution(ulong entityID, float restitution);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float BoxCollider2DComponent_GetThreshold(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider2DComponent_SetThreshold(ulong entityID, float threshold);
        
        // CIRCLE COLLIDER COMPONENT

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CircleCollider2DComponent_GetRadius(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleCollider2DComponent_SetRadius(ulong entityID, float radius);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleCollider2DComponent_GetOffset(ulong entityID, out Vector2 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleCollider2DComponent_SetOffset(ulong entityID, ref Vector2 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CircleCollider2DComponent_GetDensity(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleCollider2DComponent_SetDensity(ulong entityID, float density);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CircleCollider2DComponent_GetFriction(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleCollider2DComponent_SetFriction(ulong entityID, float friction);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CircleCollider2DComponent_GetRestitution(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleCollider2DComponent_SetRestitution(ulong entityID, float restitution);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CircleCollider2DComponent_GetThreshold(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleCollider2DComponent_SetThreshold(ulong entityID, float threshold);

        // SCENE

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Scene_FindEntityByName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Scene_CreateNewEntity(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Scene_DuplicateEntity(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Scene_DestroyEntity(ulong entityID);

        // MATHF

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Mathf_ToDegrees(float radians);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Mathf_ToRadians(float degrees);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Mathf_Tan(float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Mathf_Atan(float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Mathf_Cos(float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Mathf_Acos(float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Mathf_Sin(float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Mathf_Asin(float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Mathf_Sqrt(float value);

        // TIME

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_DeltaTime();
    }
}
