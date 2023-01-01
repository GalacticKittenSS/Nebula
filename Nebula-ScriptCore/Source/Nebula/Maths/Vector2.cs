using System;

namespace Nebula
{
    public struct Vector2
    {
        public float x, y;

        public static Vector2 Zero => new Vector2(0.0f);

        public Vector2(float scalar)
        {
            x = scalar;
            y = scalar;
        }

        public Vector2(float _x, float _y)
        {
            x = _x;
            y = _y;
        }

        public float LengthSquared()
        {
            return x * x + y * y;
        }

        public float Length()
        {
            return (float)Math.Sqrt(LengthSquared());
        }

        public void Normalize()
        {
            this /= Length();
        }

        public static float Distance(Vector2 a, Vector2 b)
        {
            return (a - b).Length();
        }

        public static float Dot(Vector2 a, Vector2 b)
        {
            return a.x * b.x + a.y * b.y;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public override bool Equals(Object obj)
        {
            if (obj == null || !this.GetType().Equals(obj.GetType()))
                return false;

            return this == (Vector2)obj;
        }

        public override string ToString()
        {
            return $"({x}, {y})";
        }

        public static Vector2 operator +(Vector2 vector, float scalar)
        {
            return new Vector2(vector.x + scalar, vector.y + scalar);
        }

        public static Vector2 operator +(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x + b.x, a.y + b.y);
        }

        public static Vector2 operator -(Vector2 vector)
        {
            return new Vector2(-vector.x, -vector.y);
        }

        public static Vector2 operator -(Vector2 vector, float scalar)
        {
            return new Vector2(vector.x - scalar, vector.y - scalar);
        }

        public static Vector2 operator -(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x - b.x, a.y - b.y);
        }

        public static Vector2 operator *(Vector2 vector, float scalar)
        {
            return new Vector2(vector.x * scalar, vector.y * scalar);
        }

        public static Vector2 operator *(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x * b.x, a.y * b.y);
        }

        public static Vector2 operator /(Vector2 vector, float scalar)
        {
            return new Vector2(vector.x / scalar, vector.y / scalar);
        }

        public static Vector2 operator /(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x / b.x, a.y / b.y);
        }

        public static bool operator ==(Vector2 a, Vector2 b)
        {
            return a.x == b.x && a.y == b.y;
        }

        public static bool operator !=(Vector2 a, Vector2 b)
        {
            return !(a == b);
        }
    }
}