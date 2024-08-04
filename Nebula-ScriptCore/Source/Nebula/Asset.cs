namespace Nebula
{
    public class Asset
    {
        protected Asset() { ID = 0; }

        internal Asset(ulong handle)
        {
            ID = handle;
        }

        public Asset(string path)
        {
            ID = InternalCalls.Asset_GetOrCreateHandle(path);
        }

        public readonly ulong ID;

        public string FilePath
        {
            get => InternalCalls.Asset_GetPathFromHandle(ID);
        }

        public bool IsValid()
        {
            if (ID == 0)
                return false;

            return InternalCalls.Asset_IsValid(ID);
        }

        public static implicit operator bool(Asset asset)
        {
            if (asset == null)
                return false;

            return asset.IsValid();
        }
    }
}
