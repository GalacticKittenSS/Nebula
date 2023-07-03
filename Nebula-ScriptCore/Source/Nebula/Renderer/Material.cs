namespace Nebula
{
    public class Material : Asset
    {
        protected Material()
            : base()
        {
        }

        internal Material(ulong handle)
            : base(handle)
        {
        }

        public Material(string path)
            : base(path)
        {
        }

        public Vector4 Colour
        {
            get
            {
                InternalCalls.Material_GetColour(AssetHandle, out Vector4 colour);
                return colour;
            }
            set => InternalCalls.Material_SetColour(AssetHandle, ref value);
        }

        public Texture Texture
        {
            get
            {
                ulong handle = InternalCalls.Material_GetTexture(AssetHandle);
                if (handle == 0)
                    return null;

                return new Texture(handle);
            }
            set
            {
                InternalCalls.Material_SetTexture(AssetHandle, value.AssetHandle);
            }
        }

        public float Tiling
        {
            get => InternalCalls.Material_GetTiling(AssetHandle);
            set => InternalCalls.Material_SetTiling(AssetHandle, value);
        }

        public static implicit operator string(Material material) => material.FilePath;
        public static implicit operator Material(string path) => new Material(path);
    }
}
