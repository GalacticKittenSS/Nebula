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
    }
}
