namespace Nebula
{
    public class Prefab : Asset
    {
        protected Prefab()
            : base()
        {
        }

        internal Prefab(ulong handle)
            : base(handle)
        {
        }

        public Prefab(string path)
            : base(path)
        {
        }
        
        public Entity Create()
        {
            ulong entityID = InternalCalls.Prefab_Create(ID);
            if (entityID == 0)
                return null;

            return new Entity(entityID);
        }

        public static implicit operator string(Prefab prefab) => prefab.FilePath;
        public static implicit operator Prefab(string path) => new Prefab(path);
    }
}
