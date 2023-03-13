namespace Nebula
{
    public class Prefab
    {
        public readonly string FilePath;

        public Prefab(string path) { FilePath = path; }

        public Entity Create()
        {
            ulong entityID = InternalCalls.Prefab_Create(FilePath);
            if (entityID == 0)
                return null;

            return new Entity(entityID);
        }

        public static implicit operator string(Prefab prefab) => prefab.FilePath;
        public static implicit operator Prefab(string path) => new Prefab(path);
    }
}
