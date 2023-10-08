namespace Nebula
{
    public struct Application
    {
        public static Vector2 GetWindowSize()
        {
            InternalCalls.Application_GetWindowSize(out Vector2 size);
            return size;
        }
        
        public static Entity GetHoveredEntity()
        {
            ulong entityID = InternalCalls.Application_GetHoveredEntity();
            if (entityID == 0)
                return null;

            return new Entity(entityID);
        }
    }
}
