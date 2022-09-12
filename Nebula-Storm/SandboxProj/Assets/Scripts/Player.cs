using Nebula;

namespace Sandbox
{
    public class Player : Entity
    {
        private Rigidbody2DComponent Rigidbody;
        private TransformComponent Transform;

        private KeyCode LastPressed;
        private bool AHeldLast = false, DHeldLast = false;

        public Vector2 JumpImpulse = new Vector2(0.0f, 50.0f);
        public Vector2 MoveSpeed = new Vector2(9.0f, 0.0f);
        
        private Vector2 Force, ForceMult;
        public float Time = 0.0f;
        
        public void OnCreate()
        {
            Debug.Log($"Player.OnCreate - {ID}, {Name}");
            Rigidbody = GetComponent<Rigidbody2DComponent>();
            Transform = GetComponent<TransformComponent>();
        
            SpriteRendererComponent SpriteRenderer = GetComponent<SpriteRendererComponent>();
            SpriteRenderer.Colour = new Vector4(0.0f, 0.5f, 1.0f, 1.0f);
        }

        public void OnUpdate(float ts)
        {
            Time += ts;

            if (Input.IsKeyDown(KeyCode.Space))
                Jump(JumpImpulse * ts);

            bool isAPressed = Input.IsKeyDown(KeyCode.A);
            bool isDPressed = Input.IsKeyDown(KeyCode.D);

            if (!(isAPressed && isDPressed))
            {
                if (isAPressed)
                    LastPressed = KeyCode.A;

                if (isDPressed)
                    LastPressed = KeyCode.D;
            }
            else if (!(AHeldLast && DHeldLast))
            {
                if (LastPressed == KeyCode.A)
                    LastPressed = KeyCode.D;

                else if (LastPressed == KeyCode.D)
                    LastPressed = KeyCode.A;
            }

            if (!isAPressed && !isDPressed)
                LastPressed = (KeyCode)0;

            if (LastPressed == KeyCode.A)
                Move(-MoveSpeed * ts);

            if (LastPressed == KeyCode.D)
                Move(MoveSpeed * ts);

            AHeldLast = isAPressed;
            DHeldLast = isDPressed;
        }

        private void Move(Vector2 speed)
        {
            Transform.Translation += new Vector3(speed.x, speed.y, 0.0f);

            if (speed.x < 0.0f)
                ForceMult.x = -1.0f;
            else
                ForceMult.x = 1.0f;

            if (speed.y < 0.0f)
                ForceMult.y = -1.0f;
            else
                ForceMult.y = 1.0f;

            Force = speed * ForceMult;
        }

        void Jump(Vector2 impulse)
        {
            Rigidbody.ApplyLinearImpulse(impulse);
        }
    }
}