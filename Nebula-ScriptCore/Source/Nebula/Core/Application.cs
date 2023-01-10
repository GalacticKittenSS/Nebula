namespace Nebula
{
    public struct Application
    {
        public static Vector2 GetWindowSize()
        {
            InternalCalls.Application_GetWindowSize(out Vector2 size);
            return size;
        }
    }
}
