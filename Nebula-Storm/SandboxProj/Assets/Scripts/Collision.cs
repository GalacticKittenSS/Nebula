using Nebula;

namespace Sandbox
{
    public class Collision : Entity
    {
		public float CollisionCount = 0;
		public bool Handled = true;
		
		void OnCollisionEnter(Entity other)
		{
			Debug.Log($"{other.ID}");
			
			if (other.Name == "Player")
				return;
			
			CollisionCount++;
			Handled = false;
		}

		void OnCollisionExit(Entity other)
		{
			if (other.Name == "Player")
				return;

			CollisionCount--;
			Handled = false;
		}
    }
}
