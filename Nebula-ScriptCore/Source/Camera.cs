using Nebula;

namespace Sandbox
{
    public class Camera : Entity
    {
        private TransformComponent Transform;
        
        void OnCreate()
        {
            Transform = GetComponent<TransformComponent>();
        }

        void OnUpdate(float ts)
        {
            float speed = 50.0f;
            Vector3 Velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.Up))
                Velocity.y = 1.0f;
            if (Input.IsKeyDown(KeyCode.Down))
                Velocity.y = -1.0f;

            if (Input.IsKeyDown(KeyCode.Left))
                Velocity.x = 1.0f;
            if (Input.IsKeyDown(KeyCode.Right))
                Velocity.x = -1.0f;

            Velocity *= speed * ts;
            Transform.Translation += Velocity;
        }
    }
}
