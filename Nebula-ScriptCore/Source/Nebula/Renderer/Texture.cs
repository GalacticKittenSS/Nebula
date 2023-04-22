namespace Nebula
{
    public class Texture : Asset
    {
        protected Texture()
             : base()
        {
        }

        internal Texture(ulong handle)
            : base(handle)
        {
        }

        public Texture(string path)
            : base(path)
        {
        }

        public static implicit operator string(Texture prefab) => prefab.FilePath;
        public static implicit operator Texture(string path) => new Texture(path);
    }
}
