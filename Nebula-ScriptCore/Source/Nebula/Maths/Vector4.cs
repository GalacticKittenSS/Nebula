using System;

namespace Nebula
{
    public struct Vector4
    {
        public float x, y, z, w;

        public static Vector4 Zero => new Vector4(0.0f);

        public Vector2 xy
        {
            get => new Vector2(x, y);
            set
            {
                x = value.x;
                y = value.y;
            }
        }

        public Vector4(float scalar)
        {
            x = scalar;
            y = scalar;
            z = scalar;
            w = scalar;
        }
        public Vector4(float _x, float _y, float _z, float _w)
        {
            x = _x;
            y = _y;
            z = _z;
            w = _w;
        }

        public float LengthSquared()
        {
            return x * x + y * y + z * z + w * w;
        }

        public float Length()
        {
            return (float)Math.Sqrt(LengthSquared());
        }

        public void Normalize()
        {
            this /= Length();
        }

        public static float Distance(Vector4 a, Vector4 b)
        {
            return (a - b).Length();
        }

        public static float Dot(Vector4 a, Vector4 b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public override bool Equals(Object obj)
        {
            if (obj == null || !this.GetType().Equals(obj.GetType()))
                return false;

            return this == (Vector4)obj;
        }

        public override string ToString()
        {
            return $"({x}, {y}, {z}, {w})";
        }

        public static Vector4 operator +(Vector4 vector, float scalar)
        {
            return new Vector4(vector.x + scalar, vector.y + scalar, vector.z + scalar, vector.w + scalar);
        }

        public static Vector4 operator +(Vector4 a, Vector4 b)
        {
            return new Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
        }

        public static Vector4 operator -(Vector4 vector)
        {
            return new Vector4(-vector.x, -vector.y, -vector.z, -vector.w);
        }

        public static Vector4 operator -(Vector4 vector, float scalar)
        {
            return new Vector4(vector.x - scalar, vector.y - scalar, vector.z - scalar, vector.w - scalar);
        }

        public static Vector4 operator -(Vector4 a, Vector4 b)
        {
            return new Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
        }

        public static Vector4 operator *(Vector4 vector, float scalar)
        {
            return new Vector4(vector.x * scalar, vector.y * scalar, vector.z * scalar, vector.w * scalar);
        }

        public static Vector4 operator *(Vector4 a, Vector4 b)
        {
            return new Vector4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
        }

        public static Vector4 operator /(Vector4 vector, float scalar)
        {
            return new Vector4(vector.x / scalar, vector.y / scalar, vector.z / scalar, vector.w / scalar);
        }

        public static Vector4 operator /(Vector4 a, Vector4 b)
        {
            return new Vector4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
        }

        public static bool operator ==(Vector4 a, Vector4 b)
        {
            return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
        }

        public static bool operator !=(Vector4 a, Vector4 b)
        {
            return !(a == b);
        }
    }
}