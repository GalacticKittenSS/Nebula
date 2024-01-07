using Nebula;

namespace Sandbox
{
    public class Camera : Entity
    {
        private Entity Player;
        public Vector2 Offset = new Vector2(0.0f);
        public float DistanceFromPlayer = 5.0f;

        void OnCreate()
        {
            Player = Nebula.Scene.FindEntityByName("Player");
        }

        void OnUpdate(float ts)
        {
            if (Player == null)
                return;

            DistanceFromPlayer -= ts;
            Translation = new Vector3(Player.Translation.xy + Offset, DistanceFromPlayer);
        }
    }
}
