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

        public static float Tan(float value)
        {
            return InternalCalls.Mathf_Tan(value);
        }
        
        public static float Atan(float value)
        {
            return InternalCalls.Mathf_Atan(value);
        }
        
        public static float Cos(float value)
        {
            return InternalCalls.Mathf_Cos(value);
        }
        
        public static float Acos(float value)
        {
            return InternalCalls.Mathf_Acos(value);
        }
        
        public static float Sin(float value)
        {
            return InternalCalls.Mathf_Sin(value);
        }
        
        public static float Asin(float value)
        {
            return InternalCalls.Mathf_Asin(value);
        }

        public static float Sqrt(float value)
        {
            return InternalCalls.Mathf_Sqrt(value);
        }
    }
}
