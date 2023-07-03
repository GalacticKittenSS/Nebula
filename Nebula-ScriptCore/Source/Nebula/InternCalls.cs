using System;
using System.Runtime.CompilerServices;

namespace Nebula
{
    internal class InternalCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Native_Log(int level, string text);

        #region Application
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Application_GetWindowSize(out Vector2 size);
        #endregion

        #region Mathf
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
        #endregion

        #region Time
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_DeltaTime();
        #endregion

        #region Input
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMouseButtonDown(MouseCode mouseCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetMousePos(out Vector2 pos);
        #endregion

        #region Asset
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Asset_GetHandleFromPath(string path);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Asset_GetOrCreateHandle(string path);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Asset_GetPathFromHandle(ulong handle);
        #endregion

        #region Font
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Font_GetBold(ulong handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Font_GetItalic(ulong handle);
        #endregion

        #region Material
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Material_Create();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_GetColour(ulong handle, out Vector4 colour);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetColour(ulong handle, ulong entityID, ref Vector4 colour);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Material_GetTexture(ulong handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetTexture(ulong handle, ulong entityID, ulong texture);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Material_GetTiling(ulong handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetTiling(ulong handle, ulong entityID, float tiling);
        #endregion

        #region Scene
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Scene_FindEntityByName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Scene_CreateNewEntity(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Scene_DuplicateEntity(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Scene_DestroyEntity(ulong entityID);
        #endregion

        #region Entity
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

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_GetMaterial(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_SetMaterial(ulong entityID, ulong handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_GetChild(ulong entityID, uint index);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Entity_GetChildCount(ulong entityID);
        #endregion

        #region Prefab
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Prefab_Create(ulong handle);
        #endregion

        #region TransformComponent
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
        #endregion

        #region CameraComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool CameraComponent_GetPrimary(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CameraComponent_SetPrimary(ulong entityID, bool primary);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool CameraComponent_GetFixedRatio(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CameraComponent_SetFixedRatio(ulong entityID, bool fixedRatio);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CameraComponent_ScreenToWorld(ulong entityID, Vector2 input, out Vector3 value);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CameraComponent_WorldToScreen(ulong entityID, Vector3 point, out Vector2 value);
        #endregion

        #region SpriteRendererComponent
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
        #endregion

        #region CircleRendererComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CircleRendererComponent_GetThickness(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_SetThickness(ulong entityID, float thickness);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float CircleRendererComponent_GetFade(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_SetFade(ulong entityID, float fade);
        #endregion

        #region StringRendererComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string StringRendererComponent_GetText(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetText(ulong entityID, string index);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_GetColour(ulong entityID, out Vector4 colour);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetColour(ulong entityID, ref Vector4 colour);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong StringRendererComponent_GetFontHandle(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetFontHandle(ulong entityID, ulong handle);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool StringRendererComponent_GetBold(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetBold(ulong entityID, bool bold);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool StringRendererComponent_GetItalic(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetItalic(ulong entityID, bool italic);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float StringRendererComponent_GetKerning(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetKerning(ulong entityID, float kerning);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float StringRendererComponent_GetLineSpacing(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetLineSpacing(ulong entityID, float lineSpacing);
        #endregion

        #region Rigidbody2DComponent
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Rigidbody2DComponent_GetBodyType(ulong entityID);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_SetBodyType(ulong entityID, int type);

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
        #endregion

        #region BoxCollider2DComponent
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
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static short BoxCollider2DComponent_GetMask(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void BoxCollider2DComponent_SetMask(ulong entityID, short mask);
        #endregion

        #region CircleCollider2DComponent
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

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static short CircleCollider2DComponent_GetMask(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleCollider2DComponent_SetMask(ulong entityID, short mask);
        #endregion
    }
}
