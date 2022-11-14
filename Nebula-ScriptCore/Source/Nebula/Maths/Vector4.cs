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

        public static Vector4 operator *(Vector4 vector, float scalar)
        {
            return new Vector4(vector.x * scalar, vector.y * scalar, vector.z * scalar, vector.w * scalar);
        }

        public static Vector4 operator +(Vector4 a, Vector4 b)
        {
            return new Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
        }
    }
}