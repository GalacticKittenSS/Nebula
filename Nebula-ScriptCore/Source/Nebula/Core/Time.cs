namespace Nebula
{
    public class Time
    {
        public static float DeltaTime => GetDeltaTime();

        public static float GetDeltaTime()
        {
            return InternalCalls.Time_DeltaTime();
        }
    }
}
