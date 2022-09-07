using Nebula;

namespace Sandbox
{
    public class Player : Entity
    {
        private Rigidbody2DComponent Rigidbody;
        private TransformComponent Transform;

        private KeyCode LastPressed;
        private bool AHeldLast = false, DHeldLast = false;

        public Vector2 JumpImpulse, MoveSpeed;
        public Vector2 Force, ForceMult;

        public float Time = 0.0f;
        public float PositionX;
        public float PositionY;

        public void OnCreate()
        {
            Debug.Log($"Player.OnCreate - {ID}, {Name}");
            Rigidbody = GetComponent<Rigidbody2DComponent>();
            Transform = GetComponent<TransformComponent>();
        
            SpriteRendererComponent SpriteRenderer = GetComponent<SpriteRendererComponent>();
            SpriteRenderer.Colour = new Vector4(0.0f, 0.5f, 1.0f, 1.0f);

            MoveSpeed = new Vector2(9.0f, 0.0f);
            JumpImpulse = new Vector2(0.0f, 50.0f);
        }

        public void OnUpdate(float ts)
        {
            PositionX = Transform.Translation.x;
            PositionY = Transform.Translation.y;

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
        
            Transform.Translation = new Vector3(PositionX, PositionY, Transform.Translation.z);
        }

        private void Move(Vector2 speed)
        {
            PositionX += speed.x;
            PositionY += speed.y;

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