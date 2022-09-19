using Nebula;

namespace Sandbox
{
    public class Camera : Entity
    {
        private Entity Player;
        public Vector2 Offset = new Vector2(0.0f);
        public float DistanceFromCamera = 5.0f;

        void OnCreate()
        {
            Player = Scene.FindEntityByName("Player");
        }

        void OnUpdate(float ts)
        {
            if (Player == null)
                return;
        
            Translation = new Vector3(Player.Translation.xy + Offset, DistanceFromCamera);
        }
    }
}
