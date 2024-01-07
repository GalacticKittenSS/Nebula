using Nebula;
using System;

namespace Sandbox
{
    public class Scene : Entity
    {
        public Entity Duplicate;
        public int EntityCount = 415;

        /// <summary>
        /// OnCreate is called once when the Entity that this script is attached to
        /// is instantiated in the scene at runtime
        /// </summary>
        void OnCreate()
        {
            for (int i = 0; i < EntityCount; i++)
            {
                Entity entity = Nebula.Scene.DuplicateEntity(Duplicate);

                Random random = new Random();
                float x = random.Next(0, 80) - 40.0f;
                float y = random.Next(0, 80) - 40.0f;
                
                float r = random.Next(0, 255) / 255.0f;
                float g = random.Next(0, 255) / 255.0f;
                float b = random.Next(0, 255) / 255.0f;

                entity.Material = new Material();
                entity.Material.Colour = new Vector4(r, g, b, 1.0f);
                entity.Translation = new Vector3(x, y, 0.0f);
            }
        }
    }
}
