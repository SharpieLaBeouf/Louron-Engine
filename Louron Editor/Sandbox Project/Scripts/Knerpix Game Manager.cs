using System;
using System.Linq;
using System.Runtime.InteropServices;

using Louron;

namespace SandboxProject
{

    public class KnerpixGameManager : Entity
    {

        // Prefabs
        public Prefab Player1_Prefab;
        public Prefab Player2_Prefab;

        // Entities
        public Entity Players_Parent_Entity; // Parent Entity to Hold Instantiated Entities
        public Entity Player1_SpawnPoint;
        public Entity Player2_SpawnPoint;
        public Entity WinnerLocation; // Location Where Winner Will Lerp To

        // Movemement Speed
        public float Speed_Multiplier = 5.0f;

        // References to Instantiated Entities
        Entity Player1;
        Entity Player2;

        // Other Game Config
        public float DeathFloorY = -50.0f;
        int[] LifeCount;
        bool GameOver = false;

        int Winner = -1;

        public float TotalLerpTimeSeconds = 2.5f;
        float CurrentLerpTime = 0.0f;
        Vector3 StartLerpPos;

        public void OnStart()
        {
            ResetGame();
        }

        public void OnUpdate()
        {
            if (GameOver)
            {
                LerpWinner();
                return;
            }

            CheckDeaths();

        }

        private void CheckDeaths()
        {
            if(Player1.transform.position.Y <= DeathFloorY)
            {
                LifeCount[0]--;
                if (LifeCount[0] <= 0) 
                {
                    GameOver = true;
                    Winner = 1;
                    StartLerpPos = Player2.transform.position;
                    Player2.GetComponent<RigidbodyComponent>().isKinematic = true;
                    
                    Destroy(Player1.ID);
                    return;
                }

                Destroy(Player1.ID);
                Destroy(Player2.ID);
                SpawnPlayers();
            }

            if(Player2.transform.position.Y <= DeathFloorY)
            {
                LifeCount[1]--;
                if (LifeCount[1] <= 0) 
                {
                    GameOver = true;
                    Winner = 0;
                    StartLerpPos = Player1.transform.position;
                    Player1.GetComponent<RigidbodyComponent>().isKinematic = true;
                    
                    Destroy(Player2.ID);
                    return;
                }

                Destroy(Player1.ID);
                Destroy(Player2.ID);
                SpawnPlayers();
            }
        }

        private void LerpWinner()
        {
            if (CurrentLerpTime < 1.0f)
            {                    
                switch(Winner)
                {
                    case 0:
                    {
                        Player1.transform.position = Vector3.Lerp(CurrentLerpTime, StartLerpPos, WinnerLocation.transform.position);

                        CurrentLerpTime += Time.deltaTime / TotalLerpTimeSeconds;
                        break;
                    }
                    case 1:
                    {
                        Player2.transform.position = Vector3.Lerp(CurrentLerpTime, StartLerpPos, WinnerLocation.transform.position);

                        CurrentLerpTime += Time.deltaTime / TotalLerpTimeSeconds;
                        break;
                    }
                    default: break;
                }
                
            }
            else
            {
                if(Input.GetKeyDown(KeyCode.Enter))
                    ResetGame();
            }
            
            // WINNERS SPIN!
            switch(Winner)
            {
                case 0:
                {
                    Player1.transform.rotation += new Vector3(Time.deltaTime * 30.0f, 0.0f, Time.deltaTime * 40.0f);
                    break;
                }
                case 1:
                {
                    Player2.transform.rotation += new Vector3(Time.deltaTime * 30.0f, 0.0f, Time.deltaTime * 40.0f);
                    break;
                }
                default: break;
            }
        }

        private void ResetGame()
        {
            if (Player1 != null) Destroy(Player1.ID);
            if (Player2 != null) Destroy(Player2.ID);

            LifeCount = new int[2];
            LifeCount[0] = 3;
            LifeCount[1] = 3;

            SpawnPlayers();

            Winner = -1;
            CurrentLerpTime = 0.0f;
            GameOver = false;
            StartLerpPos = Vector3.Zero;
        }

        private void SpawnPlayers()
        {
            Player1 = Instantiate(Player1_Prefab, Player1_SpawnPoint.transform.position);
            Player1.parent = Players_Parent_Entity;
            Player2 = Instantiate(Player2_Prefab, Player2_SpawnPoint.transform.position);
            Player2.parent = Players_Parent_Entity;
        }

        public void OnFixedUpdate() 
        {
            if (GameOver)
            {
                
                return;
            }
            
            Vector3 player_force = new Vector3();
            if(Input.GetKey(KeyCode.W))
            {
                player_force.Z += 1.0f;
            }
            if(Input.GetKey(KeyCode.S))
            {
                player_force.Z -= 1.0f;
            }
            if(Input.GetKey(KeyCode.A))
            {
                player_force.X += 1.0f;
            }
            if(Input.GetKey(KeyCode.D))
            {
                player_force.X -= 1.0f;
            }       
            Player1.GetComponent<RigidbodyComponent>().ApplyForce(player_force * Speed_Multiplier * .1f, ForceMode.eIMPULSE);

            player_force = new Vector3();
            if(Input.GetKey(KeyCode.Up))
            {
                player_force.Z += 1.0f;
            }
            if(Input.GetKey(KeyCode.Down))
            {
                player_force.Z -= 1.0f;
            }
            if(Input.GetKey(KeyCode.Left))
            {
                player_force.X += 1.0f;
            }
            if(Input.GetKey(KeyCode.Right))
            {
                player_force.X -= 1.0f;
            }
            Player2.GetComponent<RigidbodyComponent>().ApplyForce(player_force * Speed_Multiplier * .1f, ForceMode.eIMPULSE);
        }
    }
}