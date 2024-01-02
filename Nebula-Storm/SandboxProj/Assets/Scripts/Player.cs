using Nebula;

namespace Sandbox
{
    /*
     * This Script is based on the player movement in emplo,
     * A Game (Currently in early development) made in nebula.
     * This will be a good example of stretching the scripting 
     * limits in a realistic example.
    */

    /*
     * Enviroment Filter
     * Rigidbody2DComponent.Filters.A
     * 
     * Bomb Filter
     * Rigidbody2DComponent.Filters.B
     * 
     * Fragments Filter
     * Rigidbody2DComponent.Filters.C
     * 
     * Player Filter
     * Rigidbody2DComponent.Filters.D
     */

    public class Player : Entity
    {
        public Entity Camera, Floor_Collider;

        public Prefab m_Prefab = "Prefabs/Enviroment.prefab";
        public Font m_Font = "Fonts/arial.ttf";
        public Texture m_Texture;

        private Rigidbody2DComponent Rigidbody;
        private TransformComponent Transform;

        private KeyCode LastPressed;
        private bool[] KeyCodesHeld = new bool[70];
        
        private bool IsOnFloor = false, JumpForceApplied = false;
        public Vector2 JumpImpulse = new Vector2(0.0f, 50.0f);
        public Vector2 MoveSpeed = new Vector2(9.0f, 0.0f);
        private Vector2 Force, ForceMult;
        
        public void OnCreate()
        {
            Debug.Log($"Player.OnCreate - {ID}, {Name}");
            Debug.Log($"Bold: {m_Font.Bold}, Italic: {m_Font.Italic}");
            Debug.Log(m_Font.FilePath);

            m_Texture = "Textures/Checkerboard.png";

            //Layer = (int)Rigidbody2DComponent.Filters.D;
            
            Transform = GetComponent<TransformComponent>();
            Rigidbody = GetComponent<Rigidbody2DComponent>();
            
            BoxCollider2DComponent boxCollider2D = GetComponent<BoxCollider2DComponent>();
            boxCollider2D.Mask = Rigidbody2DComponent.Filters.A;

            Material material = new Material();
            material.Colour = new Vector4(0.0f, 0.5f, 1.0f, 1.0f);
            Material = material;

            StringRendererComponent stringRenderer = AddComponent<StringRendererComponent>();
            stringRenderer.Text = "This is a String";
            stringRenderer.Font = m_Font;

            Entity entity = m_Prefab.Create();
            entity.Name = "Prefab";
            entity.SetAs<Camera>();
        }

        public void OnUpdate(float ts)
        {
            MoveCamera(ts);

            UpdateCollision();

            HandleMovementInput(ts);
            HandleForce();
        }

        void UpdateCollision()
        {
            if (Floor_Collider == null)
                return;

            Collision collider = Floor_Collider.As<Collision>();
            IsOnFloor = collider.CollisionCount > 0;

            if (IsOnFloor && !collider.Handled)
                JumpForceApplied = false;

            collider.Handled = true;
        }

        private void MoveCamera(float ts)
        {
            if (Camera == null)
                return;
            
            Camera script = Camera.As<Camera>();

            float offset_speed = 25.0f * ts;
            float distance_speed = 3.0f * ts;

            if (Input.IsKeyDown(KeyCode.Up))
            {
                script.Offset.y += offset_speed;
            }

            if (Input.IsKeyDown(KeyCode.Down))
            {
                script.Offset.y -= offset_speed;
            }

            if (Input.IsKeyDown(KeyCode.Right))
            {
                script.Offset.x += offset_speed;
            }

            if (Input.IsKeyDown(KeyCode.Left))
            {
                script.Offset.x -= offset_speed;
            }

            if (Input.IsKeyDown(KeyCode.Q))
            {
                script.DistanceFromPlayer += distance_speed;
            }

            if (Input.IsKeyDown(KeyCode.E))
            {
                script.DistanceFromPlayer -= distance_speed;
            }
        }

        private void HandleMovementInput(float ts)
        {
            if (Input.IsKeyDown(KeyCode.Space))
                Jump(JumpImpulse);

            bool isAPressed = Input.IsKeyDown(KeyCode.A);
            bool isDPressed = Input.IsKeyDown(KeyCode.D);

            if (!(isAPressed && isDPressed))
            {
                if (isAPressed)
                    LastPressed = KeyCode.A;

                if (isDPressed)
                    LastPressed = KeyCode.D;
            }
            else if (!(KeyCodesHeld[(int)KeyCode.A] && KeyCodesHeld[(int)KeyCode.D]))
            {
                if (LastPressed == KeyCode.A)
                    LastPressed = KeyCode.D;

                else if (LastPressed == KeyCode.D)
                    LastPressed = KeyCode.A;
            }

            if (!isAPressed && !isDPressed)
                LastPressed = (KeyCode)0;

            if (LastPressed == KeyCode.A)
                Move(-MoveSpeed * (IsOnFloor ? 1.0f : 1.25f));

            if (LastPressed == KeyCode.D)
                Move(MoveSpeed * (IsOnFloor ? 1.0f : 1.25f));

            KeyCodesHeld[(int)KeyCode.A] = isAPressed;
            KeyCodesHeld[(int)KeyCode.D] = isDPressed;
        }

        private void Move(Vector2 speed)
        {
            Transform.Translation += new Vector3(speed * Nebula.Time.DeltaTime, 0.0f);

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

        private void HandleForce()
        {
            if (LastPressed != (KeyCode)0)
                return;

            if (IsOnFloor)
                Force *= 0.95f;

            Move(Force * ForceMult);
            Force -= Force * Nebula.Time.DeltaTime;

            if (Force.x <= 0.2f && Force.y <= 0.2f)
                Force = new Vector2(0.0f, 0.0f);
        }

        void Jump(Vector2 impulse)
        {
            if (JumpForceApplied)
                return;

            Rigidbody.ApplyLinearImpulse(impulse);
            JumpForceApplied = true;
        }
    }
}