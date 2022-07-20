using System;
using System.Runtime.CompilerServices;

namespace Nebula
{
    public struct Vector3
    {
        public float x, y, z;

        public Vector3(float _x, float _y, float _z)
        {
            x = _x; 
            y = _y; 
            z = _z; 
        }
    }

    public class InternalCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog(string text, int parameter);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void NativeLog_Vector(ref Vector3 parameter, out Vector3 result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float NativeLog_VectorDot(ref Vector3 parameter);
    }

    public class Entity
    {
        public float FloatVar { get; set; }

        public Entity()
        {
            Vector3 pos = new Vector3(5, 2, 1);
            Vector3 result = Log(pos);

            Console.WriteLine("{0}, {1}, {2}", result.x, result.y, result.z);
            Console.WriteLine("{0}", InternalCalls.NativeLog_VectorDot(ref pos));
        }

        public void PrintMessage()
        {
            Console.WriteLine("Hello World from C#!");
        }

        public void PrintInt(int value)
        {
            Console.WriteLine(value);
        }
        
        public void PrintInts(int value, int value2)
        {
            Console.WriteLine($"Value: {value}, Value 2: {value2}");
        }

        public void PrintCustomMessage(string message)
        {
            Console.WriteLine(message);
        }

        private void Log(string text, int parameter)
        {
            InternalCalls.NativeLog(text, parameter);
        }

        private Vector3 Log(Vector3 parameter)
        {
            InternalCalls.NativeLog_Vector(ref parameter, out Vector3 result);
            return result;
        }
    }
}