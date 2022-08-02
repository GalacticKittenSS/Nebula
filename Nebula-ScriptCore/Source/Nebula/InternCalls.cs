using System;
using System.Runtime.CompilerServices;

namespace Nebula
{
    internal class InternalCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Native_Log(int level, string text);

        // ENTITY CLASS

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong entityID, Type component);

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
        internal extern static void SpriteRendererComponent_GetTiling(ulong entityID, out float tiling);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRendererComponent_SetTiling(ulong entityID, float tiling);

        // CIRCLE RENDERER COMPONENT

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_GetColour(ulong entityID, out Vector4 colour);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_SetColour(ulong entityID, ref Vector4 colour);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_GetRadius(ulong entityID, out float radius);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_SetRadius(ulong entityID, float radius);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_GetThickness(ulong entityID, out float thickness);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_SetThickness(ulong entityID, float thickness);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_GetFade(ulong entityID, out float fade);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void CircleRendererComponent_SetFade(ulong entityID, float fade);

        // CIRCLE RENDERER COMPONENT

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_GetText(ulong entityID, out string index);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetText(ulong entityID, ref string index);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_GetColour(ulong entityID, out Vector4 colour);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetColour(ulong entityID, ref Vector4 colour);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_GetResolution(ulong entityID, out float resolution);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetResolution(ulong entityID, float resolution);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_GetBold(ulong entityID, out bool bold);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetBold(ulong entityID, bool bold);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_GetItalic(ulong entityID, out bool italic);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetItalic(ulong entityID, bool italic);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_GetIndex(ulong entityID, out int index);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void StringRendererComponent_SetIndex(ulong entityID, int index);

        // RIGIDBODY 2D COMPONENT

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse, ref Vector2 point);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impulse);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyForce(ulong entityID, ref Vector2 force, ref Vector2 point);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyForceToCenter(ulong entityID, ref Vector2 force);

        // INPUT CLASS
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode keyCode);
    }
}
