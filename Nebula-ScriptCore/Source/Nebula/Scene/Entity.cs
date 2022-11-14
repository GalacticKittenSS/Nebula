using System;

namespace Nebula
{
    public class Entity
    {
        protected Entity() { ID = 0; }

        internal Entity(ulong id)
        {
            ID = id;
        }

        public readonly ulong ID;

        public Vector3 Translation
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.TransformComponent_SetTranslation(ID, ref value);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                InternalCalls.TransformComponent_GetRotation(ID, out Vector3 rotation);
                return rotation;
            }
            set
            {
                InternalCalls.TransformComponent_SetRotation(ID, ref value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                InternalCalls.TransformComponent_GetScale(ID, out Vector3 scale);
                return scale;
            }
            set
            {
                InternalCalls.TransformComponent_SetScale(ID, ref value);
            }
        }

        public string Name
        {
            get
            {
                return InternalCalls.Entity_GetName(ID);
            }
            set
            {
                InternalCalls.Entity_SetName(ID, value);
            }
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return InternalCalls.Entity_HasComponent(ID, componentType);
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T() { Entity = this };
            return component;
        }

        public T AddComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            InternalCalls.Entity_AddComponent(ID, componentType);

            T component = new T() { Entity = this };
            return component;
        }

        public T As<T>() where T : Entity, new()
        {
            object instance = InternalCalls.Entity_GetScriptInstance(ID);
            return instance as T;
        }

        public Entity FindChildByName(string name)
        {
            ulong id = InternalCalls.Entity_FindChildByName(ID, name);
            if (id == 0)
                return null;

            return new Entity(id);
        }
    }
}