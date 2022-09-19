namespace Nebula
{
    public struct Scene
    {
        public static Entity FindEntityByName(string name)
        {
            ulong entityID = InternalCalls.Scene_FindEntityByName(name);
            if (entityID == 0)
                return null;

            return new Entity(entityID);
        }
    }
}
