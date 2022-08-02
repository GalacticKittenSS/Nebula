namespace Nebula
{
	public class Input
	{
		public static bool IsKeyDown(KeyCode key)
		{
			return InternalCalls.Input_IsKeyDown(key);
		}
	}
}