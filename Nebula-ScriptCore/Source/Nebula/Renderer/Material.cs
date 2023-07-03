namespace Nebula
{
    public class Material : Asset
    {
        public Material()
            : base(InternalCalls.Material_Create())
        {
        }

        internal Material(ulong handle)
            : base(handle)
        {
        }

        internal Material(ulong handle, ulong entityID)
            : base(handle)
        {
            EntityID = entityID;
        }

        internal ulong EntityID = 0;

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
            set
            {
                InternalCalls.Material_SetColour(AssetHandle, EntityID, ref value);
            }
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
                InternalCalls.Material_SetTexture(AssetHandle, EntityID, value.AssetHandle);
            }
        }

        public float Tiling
        {
            get => InternalCalls.Material_GetTiling(AssetHandle);
            set => InternalCalls.Material_SetTiling(AssetHandle, EntityID, value);
        }

        public static implicit operator string(Material material) => material.FilePath;
        public static implicit operator Material(string path) => new Material(path);
    }
}
