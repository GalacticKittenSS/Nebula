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
        private Rigidbody2DComponent Rigidbody;
        private TransformComponent Transform;

        public Entity Camera, Floor_Collider, Spawn_Location;
        public TransformComponent GunTransform = null;
        
        private KeyCode LastPressed;
        private bool[] KeyCodesHeld = new bool[70];
        private bool[] MouseCodesHeld = new bool[2];

        private bool IsOnFloor = false, JumpForceApplied = false;
        public Vector2 JumpImpulse = new Vector2(0.0f, 50.0f);
        public Vector2 MoveSpeed = new Vector2(9.0f, 0.0f);

        private Vector2 Direction = new Vector2(0.0f); 
        private Vector2 DirectionMult = new Vector2(0.25f);
        private Vector2 Force, ForceMult;

        private float BombCooldown;

        public string[] BulletTemplates = { "", "" };
        
        enum ExplosiveType { 
            Bomb = 0, Tnt = 1, 
            Default = Bomb 
        };
        
        enum ExplosiveEffect { 
            Explosive = 0x0002, Implosive = 0x0004,
            Sticky = 0x0008, None = 0x0000
        };

        private Entity LastBullet = null;
        static ExplosiveType NextExplosiveType;
        static short NextExplosiveEffect;

        public void OnCreate()
        {
            Camera = Scene.FindEntityByName("Camera");
            
            Floor_Collider = FindChildByName("Floor Collider");
            Entity Gun = FindChildByName("Gun");
            GunTransform = null;

            if (Gun != null)
            {
                GunTransform = Gun.GetComponent<TransformComponent>();
                Spawn_Location = Gun.FindChildByName("");
            }

            BoxCollider2DComponent box2D = GetComponent<BoxCollider2DComponent>();
            box2D.Mask = (int)Rigidbody2DComponent.Filters.A;
            box2D.Category = Rigidbody2DComponent.Filters.D;
            
            Debug.Log($"Player.OnCreate - {ID}, {Name}");
            Rigidbody = GetComponent<Rigidbody2DComponent>();
            Transform = GetComponent<TransformComponent>();
        
            SpriteRendererComponent SpriteRenderer = GetComponent<SpriteRendererComponent>();
            SpriteRenderer.Colour = new Vector4(0.0f, 0.5f, 1.0f, 1.0f);
        }

        public void OnUpdate(float ts)
        {
            MoveCamera(ts);

            UpdateCollision();
            UpdateMousePos();

            HandleMovementInput(ts);
            HandleForce();

            /*if (BombCooldown > 0.0f)
                BombCooldown -= ts;

            if (BombCooldown <= 0.0f)
                HandleBombInput();*/
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

        private void UpdateMousePos()
        {
            if (GunTransform == null)
                return;
            
            Vector2 mousePos = Input.GetMousePos();
            Direction = mousePos - Transform.Translation.xy;

            float magnitude = Mathf.Sqrt((Direction.x * Direction.x) + (Direction.y * Direction.y));
            Direction /= magnitude;

            GunTransform.Rotation = new Vector3(GunTransform.Rotation.xy, RotateTowards(Direction));
        }

        float RotateTowards(Vector2 point)
        {
            float rotation = 0.0f;

            if (point.x > 0.0f && point.y > 0.0f)
            {
                rotation = Mathf.Atan(point.y / point.x);
            }
            else if (point.x < 0.0f && point.y > 0.0f)
            {
                rotation = -Mathf.ToRadians(180.0f - Mathf.ToDegrees(Mathf.Atan(point.y / point.x)));
            }
            else if (point.x < 0.0f && point.y < 0.0f)
            {
                rotation = Mathf.ToRadians(180.0f + Mathf.ToDegrees(Mathf.Atan(point.y / point.x)));
            }
            else if (point.x > 0.0f && point.y < 0.0f)
            {
                rotation = -Mathf.ToRadians(360.0f - Mathf.ToDegrees(Mathf.Atan(point.y / point.x)));
            }

            return rotation;
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
                script.DistanceFromCamera += distance_speed;
            }

            if (Input.IsKeyDown(KeyCode.E))
            {
                script.DistanceFromCamera -= distance_speed;
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

        void HandleBombInput()
        {
            ExplosiveEffect explosionType = Input.IsKeyDown(KeyCode.LeftShift) ? ExplosiveEffect.Implosive : ExplosiveEffect.Explosive;
            if (Input.IsKeyDown(KeyCode.Q))
                explosionType |= ExplosiveEffect.Sticky;

            bool leftPressed = Input.IsMouseButtonDown(MouseCode.ButtonLeft);
            bool rightPressed = Input.IsMouseButtonDown(MouseCode.ButtonRight);

            if (leftPressed && !MouseCodesHeld[(int)MouseCode.ButtonLeft])
            {
                Vector2 spawnPos = Spawn_Location.GetComponent<TransformComponent>().Translation.xy;
                Shoot(ExplosiveType.Bomb, (short)explosionType, new Vector3(spawnPos, 0.0f));
            }

            if (rightPressed && !MouseCodesHeld[(int)MouseCode.ButtonRight])
            {
                Vector2 translation = GetComponent<TransformComponent>().Translation.xy;
                Vector2 toFloor = new Vector2(0.0f, GetComponent<TransformComponent>().Scale.y * 0.5f);

                Vector2 spawnPos = translation - toFloor;

                Shoot(ExplosiveType.Tnt, (short)explosionType, new Vector3(spawnPos, 0.0f));
            }

            MouseCodesHeld[(int)MouseCode.ButtonLeft] = leftPressed;
            MouseCodesHeld[(int)MouseCode.ButtonRight] = rightPressed;
        }

        private void Move(Vector2 speed)
        {
            Transform.Translation += new Vector3(speed * Nebula.Time.DeltaTime(), 0.0f);

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
            Force -= Force * Nebula.Time.DeltaTime();

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

        void Shoot(ExplosiveType type, short effect, Vector3 spawnPos)
        {
            //Entity Projectile = PrefabCreator.Deserialize(BulletTemplates[(int)type]);
            Entity Projectile = Scene.CreateNewEntity("Bullet");
            Projectile.GetComponent<TransformComponent>().Translation = spawnPos;
            Projectile.AddComponent<SpriteRendererComponent>();

            BoxCollider2DComponent box2D = Projectile.AddComponent<BoxCollider2DComponent>();
            box2D.Category = Rigidbody2DComponent.Filters.B;
            box2D.Mask = (int)Rigidbody2DComponent.Filters.A | (int)Rigidbody2DComponent.Filters.B;
        
            //Projectile.AddComponent<ExplosiveScript>();

            if (type == ExplosiveType.Bomb)
            {
                Rigidbody2DComponent rigidbody = Projectile.AddComponent<Rigidbody2DComponent>();
                rigidbody.ApplyLinearImpulse(Direction * DirectionMult);
            }

            NextExplosiveEffect = effect;
            NextExplosiveType = type;

            BombCooldown = 1.5f;

            if (LastBullet != null)
            {
                Scene.DestroyEntity(LastBullet);
            }

            LastBullet = Projectile;
        }
    }
}