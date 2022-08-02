namespace Nebula
{
    public abstract class Component
    {
        public Entity Entity { get; internal set; }
    }

    public class TagComponent : Component
    {

    }

    public class ParentChildComponent : Component
    {

    }

    public class TransformComponent : Component
    {
        public Vector3 Translation
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
            }
        }
        
        public Vector3 Rotation
        {
            get
            {
                InternalCalls.TransformComponent_GetRotation(Entity.ID, out Vector3 rotation);
                return rotation;
            }
            set
            {
                InternalCalls.TransformComponent_SetRotation(Entity.ID, ref value);
            }
        }
        
        public Vector3 Scale
        {
            get
            {
                InternalCalls.TransformComponent_GetScale(Entity.ID, out Vector3 scale);
                return scale;
            }
            set
            {
                InternalCalls.TransformComponent_SetScale(Entity.ID, ref value);
            }
        }
    }

    public class WorldTransformComponent : Component
    {

    }

    public class CameraComponent : Component
    {

    }

    public class ScriptComponent : Component
    {

    }

    public class NativeScriptComponent : Component
    {

    }

    public class SpriteRendererComponent : Component
    {
        public Vector4 Colour
        {
            get 
            {
                InternalCalls.SpriteRendererComponent_GetColour(Entity.ID, out Vector4 colour);
                return colour;
            }
            set
            {
                InternalCalls.SpriteRendererComponent_SetColour(Entity.ID, ref value);
            }
        }

        public Vector2 Offset
        {
            get
            {
                InternalCalls.SpriteRendererComponent_GetOffset(Entity.ID, out Vector2 offset);
                return offset;
            }
            set
            {
                InternalCalls.SpriteRendererComponent_SetOffset(Entity.ID, ref value);
            }
        }

        public Vector2 CellSize
        {
            get
            {
                InternalCalls.SpriteRendererComponent_GetCellSize(Entity.ID, out Vector2 size);
                return size;
            }
            set
            {
                InternalCalls.SpriteRendererComponent_SetCellSize(Entity.ID, ref value);
            }
        }

        public Vector2 CellNumber
        {
            get
            {
                InternalCalls.SpriteRendererComponent_GetCellNumber(Entity.ID, out Vector2 number);
                return number;
            }
            set
            {
                InternalCalls.SpriteRendererComponent_SetCellNumber(Entity.ID, ref value);
            }
        }

        public float Tiling
        {
            get
            {
                InternalCalls.SpriteRendererComponent_GetTiling(Entity.ID, out float tiling);
                return tiling;
            }
            set
            {
                InternalCalls.SpriteRendererComponent_SetTiling(Entity.ID, value);
            }
        }
    }

    public class CircleRendererComponent : Component
    {
        public Vector4 Colour
        {
            get
            {
                InternalCalls.CircleRendererComponent_GetColour(Entity.ID, out Vector4 colour);
                return colour;
            }
            set
            {
                InternalCalls.CircleRendererComponent_SetColour(Entity.ID, ref value);
            }
        }

        public float Radius
        {
            get
            {
                InternalCalls.CircleRendererComponent_GetRadius(Entity.ID, out float radius);
                return radius;
            }
            set
            {
                InternalCalls.CircleRendererComponent_SetRadius(Entity.ID, value);
            }
        }

        public float Thickness
        {
            get
            {
                InternalCalls.CircleRendererComponent_GetThickness(Entity.ID, out float thickness);
                return thickness;
            }
            set
            {
                InternalCalls.CircleRendererComponent_SetThickness(Entity.ID, value);
            }
        }

        public float Fade
        {
            get
            {
                InternalCalls.CircleRendererComponent_GetFade(Entity.ID, out float fade);
                return fade;
            }
            set
            {
                InternalCalls.CircleRendererComponent_SetFade(Entity.ID, value);
            }
        }
    }

    public class StringRendererComponent : Component
    {
        public string Text
        {
            get
            {
                InternalCalls.StringRendererComponent_GetText(Entity.ID, out string text);
                return text;
            }
            set
            {
                InternalCalls.StringRendererComponent_SetText(Entity.ID, ref value);
            }
        }

        public Vector4 Colour
        {
            get
            {
                InternalCalls.StringRendererComponent_GetColour(Entity.ID, out Vector4 colour);
                return colour;
            }
            set
            {
                InternalCalls.StringRendererComponent_SetColour(Entity.ID, ref value);
            }
        }

        public float Resolution
        {
            get
            {
                InternalCalls.StringRendererComponent_GetResolution(Entity.ID, out float resolution);
                return resolution;
            }
            set
            {
                InternalCalls.StringRendererComponent_SetResolution(Entity.ID, value);
            }
        }

        public bool Bold
        {
            get
            {
                InternalCalls.StringRendererComponent_GetBold(Entity.ID, out bool bold);
                return bold;
            }
            set
            {
                InternalCalls.StringRendererComponent_SetBold(Entity.ID, value);
            }
        }

        public bool Italic
        {
            get
            {
                InternalCalls.StringRendererComponent_GetItalic(Entity.ID, out bool italic);
                return italic;
            }
            set
            {
                InternalCalls.StringRendererComponent_SetItalic(Entity.ID, value);
            }
        }

        public enum Fonts
        {
            Default = 0, OpenSans, Roboto
        }

        public Fonts Font
        {
            get
            {
                InternalCalls.StringRendererComponent_GetIndex(Entity.ID, out int index);
                return (Fonts)index;
            }
            set
            {
                InternalCalls.StringRendererComponent_SetIndex(Entity.ID, (int)value);
            }
        }
    }

    public class Rigidbody2DComponent : Component
    {
        public void ApplyLinearImpulse(Vector2 force)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref force);
        }
    }

    public class BoxCollider2DComponent : Component
    {

    }

    public class CircleColliderComponent : Component
    {

    }
}