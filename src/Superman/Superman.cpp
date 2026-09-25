#include "Superman.h"
#include "../Input.h"
#include "../Abilities/Flight.h"
#include "../Abilities/TargetLock.h"
#include "../Abilities/HeatVision.h"
#include "../Abilities/Combat.h"
#include "../Abilities/FlightAnimation.h"
#include "main.h"

namespace Superman {
    static bool g_enabled=false;
    static bool g_speed=false;

    void Initialize(){ g_enabled=false; g_speed=false; FlightAnimation::Initialize(); }

    void Update(){
        if(Input::JustPressed(170)) g_enabled=!g_enabled; // F3
        if(Input::JustPressed(166)) { if(Flight::IsEnabled()) Flight::Disable(); else Flight::Enable(); } // F5
        if(Input::JustPressed(67)) TargetLock::Toggle(); // F7
        if(Input::JustPressed(56)) HeatVision::Toggle(); // F9
        if(Input::JustPressed(57)) g_speed=!g_speed; // F10
        if(Input::JustPressed(167)) Combat::SuperPunch(); // F6
        if(Input::JustPressed(169)) { g_enabled=false; Flight::Disable(); } // F8

        if(!g_enabled){
            Flight::Disable();
            FlightAnimation::Update(false, false, false, false, false, false, false, false);
            return;
        }
        bool flying = Flight::IsEnabled();
        bool boosting = Input::Boost();

        Flight::SetBoost(boosting);
        Flight::Update();

        FlightAnimation::Update(
            flying,
            boosting,
            Input::Forward(),
            Input::Back(),
            Input::Left(),
            Input::Right(),
            Input::Up(),
            Input::Down());
        TargetLock::Update();
        HeatVision::Update();

        if(g_speed){
            Ped p=PLAYER::PLAYER_PED_ID();
            if(ENTITY::DOES_ENTITY_EXIST(p)) ENTITY::SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER(PLAYER::PLAYER_ID(), 1.49f);
        }
    }
}
