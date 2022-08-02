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
    }
    
    public class Debug
    {
        public static void Log(string text)
        {
            InternalCalls.Native_Log(0, text);
        }

        public static void Warn(string text)
        {
            InternalCalls.Native_Log(1, text);
        }

        public static void Error(string text)
        {
            InternalCalls.Native_Log(2, text);
        }
    }
}