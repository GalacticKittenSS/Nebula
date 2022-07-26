using Nebula;

namespace Sandbox
{
    public class Player : Entity
    {
        void OnCreate()
        {
            Debug.Log($"Player.OnCreate - {ID}");
        }

        void OnUpdate(float ts)
        {
            Debug.Log($"Player.OnUpdate: {ts}");

            float speed = 5.0f;
            Vector3 Velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.W))
                Velocity.y =  1.0f;
            if (Input.IsKeyDown(KeyCode.S))
                Velocity.y = -1.0f;

            if (Input.IsKeyDown(KeyCode.D))
                Velocity.x =  1.0f;
            if (Input.IsKeyDown(KeyCode.A))
                Velocity.x = -1.0f;

            Velocity *= speed * ts;

            Vector3 translation = Translation;
            translation += Velocity;
            Translation = translation;
        }
    }
}
