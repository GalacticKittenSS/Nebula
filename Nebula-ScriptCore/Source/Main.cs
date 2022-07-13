using System;

namespace Nebula
{
    public class Main
    {
        public float FloatVar { get; set; }

        public Main()
        {
            Console.WriteLine("Main Constructor!");
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
    }
}