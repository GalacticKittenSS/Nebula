using Nebula;
using System;

namespace Sandbox
{
    public class Fields : Entity
    {
        public Entity entity;
        public Prefab prefab;
        public Asset asset;
        public int i = 10;
        public float x = 100.5f;
        public bool b = false;
        
        void OnCreate()
        {
            entity.As<Fields>();
            Debug.Log(entity.Name.ToString());
            b = asset.ID == prefab.ID;
            x += i;
            prefab.Create();
        }
    }
}