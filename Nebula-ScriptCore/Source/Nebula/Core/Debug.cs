namespace Nebula
{
    public class Debug
    {
        public static void Log(string text)
        {
            InternalCalls.Native_Log(0, text);
        }

        public static void Warn(string text)
        {
            InternalCalls.Native_Log(1, text);
        }

        public static void Error(string text)
        {
            InternalCalls.Native_Log(2, text);
        }
    }
}
