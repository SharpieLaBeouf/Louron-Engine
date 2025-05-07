#pragma once

#include "Script Core API/ScriptAPI.h"

using namespace Louron;
using namespace Louron::Assets;
using namespace Louron::Components;
using namespace Louron::Vectors;
using namespace Louron::Matrices;
using namespace Louron::Partitions;

class GameManager : public IScript
{

public:

    [[ExposedInEditor]] Prefab Player_1_Prefab;
    [[ExposedInEditor]] Prefab Player_2_Prefab;

    [[ExposedInEditor]] Entity Players_Parent_Entity;
    [[ExposedInEditor]] Entity Player_1_Spawn_Point;
    [[ExposedInEditor]] Entity Player_2_Spawn_Point;
    [[ExposedInEditor]] Entity Winner_Location;

    [[ExposedInEditor]] float Speed_Multiplier = 5.0f;

    std::array<Entity, 2> Players {};

    [[ExposedInEditor]] float DeathFloorY = -50.0f;

    std::array<uint8_t, 2> LifeCount = { 3, 3 };

    bool GameOver = false;

    int Winner = -1;

    float TotalLerpTimeSeconds = 2.5f;
    float CurrentLerpTime = 0.0f;
    Vector3 StartLerpPos;

    void OnCreate() override
    {
        ResetGame();            
    }

    void OnDestroy() override
    {
        Destroy(Players[0]);
        Destroy(Players[1]);
    }

    void OnUpdate() override
    {
        if (GameOver)
        {
            LerpWinner();
            return;
        }

        CheckDeaths();
    }

    void CheckDeaths()
    {
        if(Players[0].GetTransform().position.y <= DeathFloorY)
        {
            LifeCount[0]--;
            if (LifeCount[0] <= 0)
            {
                GameOver = true;
                Winner = 1;
                StartLerpPos = Players[1].GetTransform().position;
                Players[1].GetComponent<RigidbodyComponent>().SetKinematic(true);
                
                Destroy(Players[0]);
                Players[0] = {};
                return;
            }

            for(int i = 0; i < 2; i++)
            {
                Destroy(Players[i]);
                Players[i] = {};
            }
            SpawnPlayers();
        }

        if(Players[1].GetTransform().position.y <= DeathFloorY)
        {
            LifeCount[1]--;
            if (LifeCount[1] <= 0)
            {
                GameOver = true;
                Winner = 0;
                StartLerpPos = Players[0].GetTransform().position;
                Players[0].GetComponent<RigidbodyComponent>().SetKinematic(true);
                
                Destroy(Players[1]);
                Players[1] = {};
                return;
            }

            for(int i = 0; i < 2; i++)
            {
                Destroy(Players[i]);
                Players[i] = {};
            }
            SpawnPlayers();
        }
    }

    void LerpWinner()
    {
        if (CurrentLerpTime < 1.0f)
        {                    
            switch(Winner)
            {
                case 0:
                {
                    Players[0].SetPosition(Vector3::Lerp(CurrentLerpTime, StartLerpPos, Winner_Location.GetPosition()));

                    CurrentLerpTime += Time::GetDeltaTime() / TotalLerpTimeSeconds;
                    break;
                }
                case 1:
                {
                    Players[1].SetPosition(Vector3::Lerp(CurrentLerpTime, StartLerpPos, Winner_Location.GetPosition()));

                    CurrentLerpTime += Time::GetDeltaTime() / TotalLerpTimeSeconds;
                    break;
                }
                default: break;
            }
            
        }
        else
        {
            if(Input::GetKeyDown(KeyCode::Enter))
                ResetGame();
        }
        
        // WINNERS SPIN!
        switch(Winner)
        {
            case 0:
            {
                Players[0].SetRotation(Players[0].GetRotation() + Vector3(Time::GetDeltaTime() * 30.0f, 0.0f, Time::GetDeltaTime() * 40.0f));
                break;
            }
            case 1:
            {
                Players[1].SetRotation(Players[1].GetRotation() + Vector3(Time::GetDeltaTime() * 30.0f, 0.0f, Time::GetDeltaTime() * 40.0f));
                break;
            }
            default: break;
        }
    }

    void ResetGame()
    {
        for(int i = 0; i < 2; i++)
        {
            if(Players[i])
            {
                Destroy(Players[i]);
                Players[i] = {};
            }
        }

        LifeCount[0] = 3;
        LifeCount[1] = 3;

        SpawnPlayers();

        Winner = -1;
        CurrentLerpTime = 0.0f;
        GameOver = false;
        StartLerpPos = Vector3::Zero();
    }

    void SpawnPlayers()
    {
        Players[0] = Instantiate(Player_1_Prefab, Players_Parent_Entity);
        Players[1] = Instantiate(Player_2_Prefab, Players_Parent_Entity);

        Players[0].SetPosition(Player_1_Spawn_Point.GetPosition());
        Players[1].SetPosition(Player_2_Spawn_Point.GetPosition());
    }

    void OnFixedUpdate() 
    {
        if (GameOver)
        {
            
            return;
        }
        
        Vector3 player_force{};
        if(Input::GetKey(KeyCode::W))
        {
            player_force.z += 1.0f;
        }
        if(Input::GetKey(KeyCode::S))
        {
            player_force.z -= 1.0f;
        }
        if(Input::GetKey(KeyCode::A))
        {
            player_force.x += 1.0f;
        }
        if(Input::GetKey(KeyCode::D))
        {
            player_force.x -= 1.0f;
        }       
        Players[0].GetComponent<RigidbodyComponent>().ApplyForce(player_force * Speed_Multiplier * .1f, RigidbodyComponent::ForceMode::Impulse);

        player_force = {};
        if(Input::GetKey(KeyCode::Up))
        {
            player_force.z += 1.0f;
        }
        if(Input::GetKey(KeyCode::Down))
        {
            player_force.z -= 1.0f;
        }
        if(Input::GetKey(KeyCode::Left))
        {
            player_force.x += 1.0f;
        }
        if(Input::GetKey(KeyCode::Right))
        {
            player_force.x -= 1.0f;
        }
        Players[1].GetComponent<RigidbodyComponent>().ApplyForce(player_force * Speed_Multiplier * .1f, RigidbodyComponent::ForceMode::Impulse);
    }

private:

};