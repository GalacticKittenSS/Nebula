using Nebula;

namespace Sandbox
{
    public class Player : Entity
    {
        private Rigidbody2DComponent Rigidbody;

        void OnCreate()
        {
            Debug.Log($"Player.OnCreate - {ID}");
            Rigidbody = GetComponent<Rigidbody2DComponent>();
        }

        void OnUpdate(float ts)
        {
            float speed = 25.0f;
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
            Rigidbody.ApplyLinearImpulse(Velocity.xy);
        }
    }
}