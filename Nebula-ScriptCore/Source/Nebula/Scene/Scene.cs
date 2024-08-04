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

        public static bool EntityExists(string name)
        {
            return InternalCalls.Scene_EntityNameExists(name);
        }

        public static Entity CreateNewEntity(string name)
        {
            ulong entityID = InternalCalls.Scene_CreateNewEntity(name);
            return new Entity(entityID);
        }

        public static Entity DuplicateEntity(Entity entity)
        {
            ulong entityID = InternalCalls.Scene_DuplicateEntity(entity.ID);
            return new Entity(entityID);
        }

        public static void DestroyEntity(Entity entity)
        {
            InternalCalls.Scene_DestroyEntity(entity.ID);
        }
    }
}
