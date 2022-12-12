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

        public static Vector2 operator *(Vector2 vector, float scalar)
        {
            return new Vector2(vector.x * scalar, vector.y * scalar);
        }
        
        public static Vector2 operator *(Vector2 v1, Vector2 v2)
        {
            return new Vector2(v1.x * v2.x, v1.y * v2.y);
        }

        public static Vector2 operator +(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x + b.x, a.y + b.y);
        }

        public static Vector2 operator -(Vector2 vector)
        {
            return new Vector2(-vector.x, -vector.y);
        } 
        
        public static Vector2 operator -(Vector2 vector, Vector2 other)
        {
            return new Vector2(vector.x - other.x, vector.y - other.y);
        }

        public static Vector2 operator /(Vector2 vector, float scalar)
        {
            return new Vector2(vector.x / scalar, vector.y / scalar);
        }

        public static bool operator <(Vector2 v1, Vector2 v2)
        {
            return v1.x < v2.x || v1.y < v2.y;
        }

        public static bool operator >(Vector2 v1, Vector2 v2)
        {
            return v1.x > v2.x || v1.y > v2.y;
        }

        public float LengthSquared()
        {
            return x * x + y * y;
        }
        
        public float Length()
        {
            return (float)Math.Sqrt(LengthSquared());
        }
    }
}