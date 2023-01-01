using System;

namespace Nebula
{
    public struct Vector3
    {
        public float x, y, z;

        public static Vector3 Zero => new Vector3(0.0f);

        public Vector2 xy
        {
            get => new Vector2(x, y);
            set {
                x = value.x;
                y = value.y;
            }
        }

        public Vector3(float scalar)
        {
            x = scalar;
            y = scalar;
            z = scalar;
        }
        public Vector3(float _x, float _y, float _z)
        {
            x = _x;
            y = _y;
            z = _z;
        }
        
        public Vector3(Vector2 xy, float _z)
        {
            x = xy.x;
            y = xy.y;
            z = _z;
        }

        public float LengthSquared()
        {
            return x * x + y * y + z * z;
        }

        public float Length()
        {
            return (float)Math.Sqrt(LengthSquared());
        }

        public static float Distance(Vector3 a, Vector3 b)
        {
            return (a - b).Length();
        }

        public void Normalize()
        {
            this /= Length();
        }

        public static float Dot(Vector3 a, Vector3 b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        public static Vector3 Cross(Vector3 a, Vector3 b)
        {
            return new Vector3(
               a.y * b.z - a.y * b.z,
               a.z * b.x - a.z * b.x,
               a.x * b.y - a.x * b.y);
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public override bool Equals(Object obj)
        {
            if (obj == null || !this.GetType().Equals(obj.GetType()))
                return false;

            return this == (Vector3)obj;
        }

        public override string ToString()
        {
            return $"({x}, {y}, {z})";
        }

        public static Vector3 operator +(Vector3 vector, float scalar)
        {
            return new Vector3(vector.x + scalar, vector.y + scalar, vector.z + scalar);
        }

        public static Vector3 operator +(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
        }

        public static Vector3 operator -(Vector3 vector)
        {
            return new Vector3(-vector.x, -vector.y, -vector.z);
        }

        public static Vector3 operator -(Vector3 vector, float scalar)
        {
            return new Vector3(vector.x - scalar, vector.y - scalar, vector.z - scalar);
        }

        public static Vector3 operator -(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
        }

        public static Vector3 operator *(Vector3 vector, float scalar)
        {
            return new Vector3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
        }

        public static Vector3 operator *(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
        }

        public static Vector3 operator /(Vector3 vector, float scalar)
        {
            return new Vector3(vector.x / scalar, vector.y / scalar, vector.z / scalar);
        }

        public static Vector3 operator /(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x / b.x, a.y / b.y, a.z / b.z);
        }

        public static bool operator ==(Vector3 a, Vector3 b)
        {
            return a.x == b.x && a.y == b.y && a.z == b.z;
        }

        public static bool operator !=(Vector3 a, Vector3 b)
        {
            return !(a == b);
        }
    }
}