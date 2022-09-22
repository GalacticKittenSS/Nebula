namespace Nebula
{
    public class Mathf
    {
        public static float ToDegrees(float radians)
        {
            return InternalCalls.Mathf_ToDegrees(radians);
        }

        public static float ToRadians(float degrees)
        {
            return InternalCalls.Mathf_ToRadians(degrees);
        }

        public static float Atan(float value)
        {
            return InternalCalls.Mathf_Atan(value);
        }

        public static float Sqrt(float value)
        {
            return InternalCalls.Mathf_Sqrt(value);
        }
    }
}
