namespace Nebula
{
    public class Asset
    {
        protected Asset() { AssetHandle = 0; }

        internal Asset(ulong handle)
        {
            AssetHandle = handle;
        }

        public Asset(string path)
        {
            AssetHandle = InternalCalls.Asset_GetOrCreateHandle(path);
        }

        public readonly ulong AssetHandle;

        public string FilePath
        {
            get => InternalCalls.Asset_GetPathFromHandle(AssetHandle);
        }
    }
}
