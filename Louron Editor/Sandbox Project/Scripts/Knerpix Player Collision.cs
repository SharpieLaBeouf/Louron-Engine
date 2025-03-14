using System;
using System.Runtime.InteropServices;

using Louron;

namespace SandboxProject
{

    public class KnerpixPlayerCollision : Entity
    {

        static float CollisionTimerOffset = 0.0f;

        public void OnUpdate()
        {
            if(CollisionTimerOffset > 0.0f)
                CollisionTimerOffset -= Time.deltaTime;
        }

        // Collider Functions
        public void OnCollideEnter(Collider other) 
        {
            // Prevent Multiple Contacts in Short Period of Time
            if(CollisionTimerOffset > 0.0f)
                return;
            
            if ((this.tag == "Player 1" && other.entity.tag == "Player 2") || (this.tag == "Player 2" && other.entity.tag == "Player 1")) 
            {
                if (!this.HasComponent<RigidbodyComponent>() || !other.entity.HasComponent<RigidbodyComponent>())
                    return;

                Vector3 self_velocity = this.GetComponent<RigidbodyComponent>().linearVelocity;
                Vector3 other_velocity = other.entity.GetComponent<RigidbodyComponent>().linearVelocity;

                if (self_velocity.Length() < other_velocity.Length()) 
                {
                    // Calculate the direction from A to B
                    Vector3 direction = this.transform.position - other.entity.transform.position;
                    direction.Normalize();

                    // Calculate impulse magnitude based on other persons velocity
                    float impulseMagnitude = other_velocity.Length() * 50.0f; // length or length squared?

                    this.GetComponent<RigidbodyComponent>().ApplyForce(direction * impulseMagnitude, ForceMode.eFORCE);
                    CollisionTimerOffset = 1.0f;
                }
            }
        }
    }
}