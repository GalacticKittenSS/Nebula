namespace Nebula
{
	public class Input
	{
		public static bool IsKeyDown(KeyCode key)
		{
			return InternalCalls.Input_IsKeyDown(key);
		}

		public static bool IsMouseButtonDown(MouseCode mouse)
		{
			return InternalCalls.Input_IsMouseButtonDown(mouse);
		}

		public static Vector2 GetMousePos()
        {
			InternalCalls.Input_GetMousePos(out Vector2 pos);
			return pos;
		}
	}
}