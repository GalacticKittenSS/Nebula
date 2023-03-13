using Nebula;

namespace Sandbox
{
    public class Collision : Entity
    {
		public int CollisionCount = 0;
		public bool Handled = true;
		
		void OnCollisionEnter(Entity other)
		{
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
