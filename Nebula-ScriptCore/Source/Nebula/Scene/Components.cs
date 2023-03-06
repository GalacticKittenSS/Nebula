namespace Nebula
{
    public abstract class Component
    {
        public Entity Entity { get; internal set; }
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

        public Vector3 WorldTranslation
        {
            get
            {
                InternalCalls.TransformComponent_GetWorldTranslation(Entity.ID, out Vector3 translation);
                return translation;
            }
        }

        public Vector3 WorldRotation
        {
            get
            {
                InternalCalls.TransformComponent_GetWorldRotation(Entity.ID, out Vector3 rotation);
                return rotation;
            }
        }

        public Vector3 WorldScale
        {
            get
            {
                InternalCalls.TransformComponent_GetScale(Entity.ID, out Vector3 scale);
                return scale;
            }
        }
    }

    public class CameraComponent : Component
    {
        //public SceneCamera Camera;
        
        public bool Primary
        {
            get
            {
                return InternalCalls.CameraComponent_GetPrimary(Entity.ID);
            }
            set
            {
                InternalCalls.CameraComponent_SetPrimary(Entity.ID, value);
            }
        }

        bool FixedAspectRatio
        {
            get
            {
                return InternalCalls.CameraComponent_GetFixedRatio(Entity.ID);
            }
            set
            {
                InternalCalls.CameraComponent_SetFixedRatio(Entity.ID, value);
            }
        }
    }

    public class SpriteRendererComponent : Component
    {
        // public Texture2D Texture
        
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
                return InternalCalls.SpriteRendererComponent_GetTiling(Entity.ID);
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
                return InternalCalls.CircleRendererComponent_GetRadius(Entity.ID);
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
                return InternalCalls.CircleRendererComponent_GetThickness(Entity.ID);
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
                return InternalCalls.CircleRendererComponent_GetFade(Entity.ID);
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
                return InternalCalls.StringRendererComponent_GetText(Entity.ID);
            }
            set
            {
                InternalCalls.StringRendererComponent_SetText(Entity.ID, value);
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

        public bool Bold
        {
            get
            {
                return InternalCalls.StringRendererComponent_GetBold(Entity.ID);
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
                return InternalCalls.StringRendererComponent_GetItalic(Entity.ID);
            }
            set
            {
                InternalCalls.StringRendererComponent_SetItalic(Entity.ID, value);
            }
        }

        public string FontName
        {
            get
            {
                return InternalCalls.StringRendererComponent_GetFontName(Entity.ID);
            }
            set
            {
                InternalCalls.StringRendererComponent_SetFontName(Entity.ID, value);
            }
        }
    }

    public class Rigidbody2DComponent : Component
    {
        public enum BodyType { Static = 0, Dynamic, Kinematic };
        
        public BodyType Type
        {
            get
            {
                return (BodyType)InternalCalls.Rigidbody2DComponent_GetBodyType(Entity.ID);
            }
            set
            {
                InternalCalls.Rigidbody2DComponent_SetBodyType(Entity.ID, (int)value);
            }
        }

        public bool FixedRotation
        {
            get
            {
                return InternalCalls.Rigidbody2DComponent_GetFixedRotation(Entity.ID);
            }
            set
            {
                InternalCalls.Rigidbody2DComponent_SetFixedRotation(Entity.ID, value);
            }
        }

        public Vector2 LinearVelocity
        {
            get
            {
                InternalCalls.Rigidbody2DComponent_GetLinearVelocity(Entity.ID, out Vector2 velocity);
                return velocity;
            }
        }

        public enum Filters
        {
            A = 0x0001, B = 0x0002, C = 0x0004, D = 0x0008,
            E = 0x0010, F = 0x0020, G = 0x0040, H = 0x0080,
            I = 0x0100, J = 0x0200, K = 0x0400, L = 0x0800,
            M = 0x1000, N = 0x2000, O = 0x4000, P = 0x8000
        };

        public void ApplyLinearImpulse(Vector2 impulse)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impulse);
        }
        
        public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPoint)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref worldPoint);
        }
        
        public void ApplyForce(Vector2 force)
        {
            InternalCalls.Rigidbody2DComponent_ApplyForceToCenter(Entity.ID, ref force);
        }
        
        public void ApplyForce(Vector2 force, Vector2 worldPoint)
        {
            InternalCalls.Rigidbody2DComponent_ApplyForce(Entity.ID, ref force, ref worldPoint);
        }
    }

    public class BoxCollider2DComponent : Component
    {
        public Vector2 Size
        {
            get
            {
                InternalCalls.BoxCollider2DComponent_GetSize(Entity.ID, out Vector2 size);
                return size;
            }
            set
            {
                InternalCalls.BoxCollider2DComponent_SetSize(Entity.ID, ref value);
            }
        }

        public Vector2 Offset
        {
            get
            {
                InternalCalls.BoxCollider2DComponent_GetOffset(Entity.ID, out Vector2 offset);
                return offset;
            }
            set
            {
                InternalCalls.BoxCollider2DComponent_SetOffset(Entity.ID, ref value);
            }
        }

        public Rigidbody2DComponent.Filters Category
        {
            get
            {
                return (Rigidbody2DComponent.Filters)InternalCalls.BoxCollider2DComponent_GetCategory(Entity.ID);
                
            }
            set
            {
                InternalCalls.BoxCollider2DComponent_SetCategory(Entity.ID, (int)value);
            }
        }

        public Rigidbody2DComponent.Filters Mask
        {
            get
            {
                return (Rigidbody2DComponent.Filters)InternalCalls.BoxCollider2DComponent_GetMask(Entity.ID);
            }
            set
            {
                InternalCalls.BoxCollider2DComponent_SetMask(Entity.ID, (int)value);
            }
        }

        public float Density
        {
            get
            {
                return InternalCalls.BoxCollider2DComponent_GetDensity(Entity.ID);
            }
            set
            {
                InternalCalls.BoxCollider2DComponent_SetDensity(Entity.ID, value);
            }
        }

        public float Friction
        {
            get
            {
                return InternalCalls.BoxCollider2DComponent_GetFriction(Entity.ID);
            }
            set
            {
                InternalCalls.BoxCollider2DComponent_SetFriction(Entity.ID, value);
            }
        }

        public float Restitution
        {
            get
            {
                return InternalCalls.BoxCollider2DComponent_GetRestitution(Entity.ID);
            }
            set
            {
                InternalCalls.BoxCollider2DComponent_SetRestitution(Entity.ID, value);
            }
        }

        public float RestitutionThreshold
        {
            get
            {
                return InternalCalls.BoxCollider2DComponent_GetThreshold(Entity.ID);
            }
            set
            {
                InternalCalls.BoxCollider2DComponent_SetThreshold(Entity.ID, value);
            }
        }
    }

    public class CircleColliderComponent : Component
    {
        public float Radius
        {
            get
            {
                return InternalCalls.CircleCollider2DComponent_GetRadius(Entity.ID);
            }
            set
            {
                InternalCalls.CircleCollider2DComponent_SetRadius(Entity.ID, value);
            }
        }

        public Vector2 Offset
        {
            get
            {
                InternalCalls.CircleCollider2DComponent_GetOffset(Entity.ID, out Vector2 offset);
                return offset;
            }
            set
            {
                InternalCalls.CircleCollider2DComponent_SetOffset(Entity.ID, ref value);
            }
        }

        public Rigidbody2DComponent.Filters Category
        {
            get
            {
                return (Rigidbody2DComponent.Filters)InternalCalls.CircleCollider2DComponent_GetCategory(Entity.ID);

            }
            set
            {
                InternalCalls.CircleCollider2DComponent_SetCategory(Entity.ID, (int)value);
            }
        }

        public Rigidbody2DComponent.Filters Mask
        {
            get
            {
                return (Rigidbody2DComponent.Filters)InternalCalls.CircleCollider2DComponent_GetMask(Entity.ID);
            }
            set
            {
                InternalCalls.CircleCollider2DComponent_SetMask(Entity.ID, (int)value);
            }
        }

        public float Density
        {
            get
            {
                return InternalCalls.CircleCollider2DComponent_GetDensity(Entity.ID);
            }
            set
            {
                InternalCalls.CircleCollider2DComponent_SetDensity(Entity.ID, value);
            }
        }

        public float Friction
        {
            get
            {
                return InternalCalls.CircleCollider2DComponent_GetFriction(Entity.ID);
            }
            set
            {
                InternalCalls.CircleCollider2DComponent_SetFriction(Entity.ID, value);
            }
        }

        public float Restitution
        {
            get
            {
                return InternalCalls.CircleCollider2DComponent_GetRestitution(Entity.ID);
            }
            set
            {
                InternalCalls.CircleCollider2DComponent_SetRestitution(Entity.ID, value);
            }
        }

        public float RestitutionThreshold
        {
            get
            {
                return InternalCalls.CircleCollider2DComponent_GetThreshold(Entity.ID);
            }
            set
            {
                InternalCalls.CircleCollider2DComponent_SetThreshold(Entity.ID, value);
            }
        }
    }
}