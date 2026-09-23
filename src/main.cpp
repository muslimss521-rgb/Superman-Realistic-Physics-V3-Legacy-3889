#include <windows.h>
#include <cmath>
#include <algorithm>

#include "ScriptHookV/main.h"
#include "ScriptHookV/natives.h"
#include "Superman.h"

static HMODULE g_module = nullptr;
static SupermanController g_superman;

static bool g_menuOpen = false;
static int g_selected = 0;
static Entity g_grabbed = 0;
static Entity g_frozenTarget = 0;
static int g_freezeUntil = 0;
static int g_actionCooldown = 0;
static bool g_groundPounding = false;
static bool g_groundPoundArmed = false;
static bool g_bulletTimeActive = false;
static float g_savedTimeScale = 1.0f;
static float g_lastFlightSpeed = 0.0f;

static bool lastF3=false,lastUp=false,lastDown=false,lastEnter=false;
static bool lastF=false,lastG=false,lastH=false,lastJ=false,lastK=false,lastR=false;
static bool lastV=false,lastE=false,lastQ=false,lastB=false,lastX=false;

static const int MENU_COUNT=14;
static const char* MENU[MENU_COUNT]={
    "Superman","Flight / Hover","Boost","Super Speed","Super Jump",
    "Invincibility","Heat Vision","Freeze Breath","Super Breath",
    "Super Strength","Ground Pound","Grab / Throw","Bullet Time","Super Senses"
};

static bool Pressed(int key,bool& last){
    bool now=(GetAsyncKeyState(key)&0x8000)!=0;
    bool result=now&&!last;
    last=now;
    return result;
}

static float Clamp01(float v){ return v<0.0f?0.0f:(v>1.0f?1.0f:v); }
static float MaxF(float a,float b){ return a>b?a:b; }
static int MaxI(int a,int b){ return a>b?a:b; }
static float MinF(float a,float b){ return a<b?a:b; }

static float Dot(const Vector3& a,const Vector3& b){ return a.x*b.x+a.y*b.y+a.z*b.z; }

static float Length3(const Vector3& v){ return std::sqrt(v.x*v.x+v.y*v.y+v.z*v.z); }

static Vector3 MakeVec(float x,float y,float z){ Vector3 v{}; v.x=x; v.y=y; v.z=z; return v; }

static Vector3 Normalize3(const Vector3& v){
    float l=Length3(v);
    if(l<=0.0001f) return MakeVec(0.0f,0.0f,0.0f);
    return MakeVec(v.x/l,v.y/l,v.z/l);
}

static Vector3 Add(const Vector3& a,const Vector3& b){ return MakeVec(a.x+b.x,a.y+b.y,a.z+b.z); }
static Vector3 Mul(const Vector3& a,float s){ return MakeVec(a.x*s,a.y*s,a.z*s); }

static void CameraBasis(Vector3& forward,Vector3& right){
    const float r=0.017453292519943295f;
    Vector3 rot=CAM::GET_GAMEPLAY_CAM_ROT(2);
    float pitch=rot.x*r;
    float yaw=rot.z*r;
    float cp=std::cos(pitch),sp=std::sin(pitch);
    float cy=std::cos(yaw),sy=std::sin(yaw);
    forward=MakeVec(-sy*cp,cy*cp,sp);
    right=MakeVec(cy,sy,0.0f);
}

static void Text(const char* text,float x,float y,float scale){
    UI::SET_TEXT_FONT(0); UI::SET_TEXT_SCALE(0.0f,scale);
    UI::SET_TEXT_COLOUR(255,255,255,255); UI::SET_TEXT_PROPORTIONAL(true);
    UI::SET_TEXT_OUTLINE(); UI::_SET_TEXT_ENTRY("STRING");
    UI::_ADD_TEXT_COMPONENT_STRING((char*)text); UI::_DRAW_TEXT(x,y);
}

static void DrawMenu(){
    if(!g_menuOpen) return;
    GRAPHICS::DRAW_RECT(0.20f,0.38f,0.36f,0.57f,0,0,0,215);
    GRAPHICS::DRAW_RECT(0.20f,0.105f,0.36f,0.055f,25,70,160,240);
    Text("SUPERMAN  REALISTIC PHYSICS",0.035f,0.083f,0.36f);
    for(int i=0;i<MENU_COUNT;++i){
        float y=0.125f+i*0.035f;
        if(i==g_selected) GRAPHICS::DRAW_RECT(0.20f,y+0.009f,0.33f,0.031f,50,110,210,220);
        Text(MENU[i],0.035f,y,0.27f);
    }
    Text("F3 CLOSE | ARROWS SELECT | ENTER",0.035f,0.635f,0.22f);
}

static Ped FindTarget(Ped self,float radius){
    Vector3 p=ENTITY::GET_ENTITY_COORDS(self,true);
    Ped target=0;
    if(!PED::GET_CLOSEST_PED(p.x,p.y,p.z,radius,true,false,&target,false,false,-1)) return 0;
    if(target==0||target==self||ENTITY::IS_ENTITY_DEAD(target)) return 0;
    return target;
}

static void ApplyDirectionalForce(Entity entity,const Vector3& dir,float force){
    Vector3 n=Normalize3(dir);
    ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(entity,1,n.x*force,n.y*force,n.z*force,true,true,true,true);
}

static void DamageAndRagdoll(Ped target,int damage,int ragdollMs){
    if(!target||ENTITY::IS_ENTITY_DEAD(target)) return;
    int hp=ENTITY::GET_ENTITY_HEALTH(target);
    if(hp>0 && damage>0) ENTITY::SET_ENTITY_HEALTH(target,MaxI(1,hp-damage));
    PED::SET_PED_TO_RAGDOLL(target,ragdollMs,ragdollMs,0,true,true,false);
}

static void SupermanOn(Ped ped){
    g_superman.enabled=true;
    g_superman.abilities.state.superSpeed=true;
    g_superman.abilities.state.godMode=true;
    ENTITY::SET_ENTITY_INVINCIBLE(ped,true);
    PED::SET_PED_MOVE_RATE_OVERRIDE(ped,2.0f);
}

static void SupermanOff(Ped ped){
    g_superman.enabled=false;
    g_superman.abilities.state={};
    g_groundPounding=false; g_groundPoundArmed=false;
    g_bulletTimeActive=false;
    if(g_grabbed){ ENTITY::FREEZE_ENTITY_POSITION(g_grabbed,false); g_grabbed=0; }
    if(g_frozenTarget){ ENTITY::FREEZE_ENTITY_POSITION(g_frozenTarget,false); g_frozenTarget=0; }
    GAMEPLAY::SET_TIME_SCALE(1.0f);
    ENTITY::SET_ENTITY_INVINCIBLE(ped,false);
    ENTITY::SET_ENTITY_HAS_GRAVITY(ped,true);
    PED::SET_PED_MOVE_RATE_OVERRIDE(ped,1.0f);
    ENTITY::SET_ENTITY_VELOCITY(ped,0.0f,0.0f,0.0f);
    g_superman.velocity=Vec3();
}

static void ToggleBulletTime(){
    g_bulletTimeActive=!g_bulletTimeActive;
    g_superman.abilities.state.bulletTime=g_bulletTimeActive;
    GAMEPLAY::SET_TIME_SCALE(g_bulletTimeActive?0.35f:1.0f);
}

static void HeatVision(Ped ped,float dt){
    if(!g_superman.abilities.state.heatVision || !g_superman.enabled) return;

    Vector3 forward,right;
    CameraBasis(forward,right);

    Vector3 start=CAM::GET_GAMEPLAY_CAM_COORD();
    Vector3 end=Add(start,Mul(forward,150.0f));

    // Visual beam. We deliberately avoid weapon natives because this
    // Legacy SDK does not expose SHOOT_SINGLE_BULLET_BETWEEN_COORDS.
    GRAPHICS::DRAW_LINE(
        start.x,start.y,start.z,
        end.x,end.y,end.z,
        255,40,20,220
    );

    // Apply the effect to a nearby target that is actually in the beam.
    Ped target=FindTarget(ped,35.0f);
    if(!target) return;

    Vector3 tp=ENTITY::GET_ENTITY_COORDS(target,true);
    Vector3 from=MakeVec(tp.x-start.x,tp.y-start.y,tp.z-start.z);
    float distance=Length3(from);
    if(distance>35.0f || distance<=0.001f) return;

    Vector3 toTarget=Normalize3(from);
    float alignment=Dot(forward,toTarget);
    if(alignment<0.96f) return;

    float damagePerSecond=55.0f;
    int damage=(int)(damagePerSecond*dt);
    if(damage<1) damage=1;

    DamageAndRagdoll(target,damage,120);
    ApplyDirectionalForce(target,forward,12.0f*dt*60.0f);
}

static void FreezeBreath(Ped ped,float dt){
    if(!g_superman.abilities.state.freezeBreath || !g_superman.enabled) return;
    int now=(int)GAMEPLAY::GET_GAME_TIMER();
    if(g_frozenTarget && now>=g_freezeUntil){ ENTITY::FREEZE_ENTITY_POSITION(g_frozenTarget,false); g_frozenTarget=0; }
    if(now<g_actionCooldown) return;
    Ped target=FindTarget(ped,35.0f);
    if(!target) return;
    Vector3 a=ENTITY::GET_ENTITY_COORDS(ped,true), b=ENTITY::GET_ENTITY_COORDS(target,true);
    Vector3 dir=Normalize3(MakeVec(b.x-a.x,b.y-a.y,b.z-a.z));
    ApplyDirectionalForce(target,dir,3.0f*dt*60.0f);
    ENTITY::FREEZE_ENTITY_POSITION(target,true);
    g_frozenTarget=target;
    g_freezeUntil=now+350;
    g_actionCooldown=now+140;
}

static void SuperBreath(Ped ped){
    if(!g_superman.abilities.state.superBreath || !g_superman.enabled) return;
    int now=(int)GAMEPLAY::GET_GAME_TIMER();
    if(now<g_actionCooldown) return;
    Ped target=FindTarget(ped,22.0f);
    if(!target) return;
    Vector3 forward,right; CameraBasis(forward,right);
    Vector3 toTarget=Normalize3(MakeVec(ENTITY::GET_ENTITY_COORDS(target,true).x-ENTITY::GET_ENTITY_COORDS(ped,true).x,
                                        ENTITY::GET_ENTITY_COORDS(target,true).y-ENTITY::GET_ENTITY_COORDS(ped,true).y,
                                        ENTITY::GET_ENTITY_COORDS(target,true).z-ENTITY::GET_ENTITY_COORDS(ped,true).z));
    float alignment=Dot(forward,toTarget);
    if(alignment<0.35f) return;
    float force=40.0f+80.0f*Clamp01(alignment);
    ApplyDirectionalForce(target,forward,force);
    DamageAndRagdoll(target,2,350);
    g_actionCooldown=now+160;
}

static void SuperPunch(Ped ped){
    if(!g_superman.abilities.state.superStrength || !g_superman.enabled) return;
    int now=(int)GAMEPLAY::GET_GAME_TIMER();
    if(now<g_actionCooldown) return;
    Ped target=FindTarget(ped,3.2f);
    if(!target) return;
    Vector3 forward,right; CameraBasis(forward,right);
    float speed=MaxF(8.0f,ENTITY::GET_ENTITY_SPEED(ped));
    float energy=g_superman.mass*speed*speed*0.5f*g_superman.abilities.punchEnergyScale;
    float force=std::sqrt(MaxF(1.0f,energy))*2.0f;
    ApplyDirectionalForce(target,forward,force);
    DamageAndRagdoll(target,(int)MinF(120.0f,8.0f+energy*0.015f),900);
    g_actionCooldown=now+450;
}

static void UpdateGrab(Ped ped){
    if(!g_superman.abilities.state.grabbing || !g_superman.enabled) return;
    if(!g_grabbed){
        Ped target=FindTarget(ped,4.0f);
        if(!target) return;
        g_grabbed=target;
        ENTITY::SET_ENTITY_DYNAMIC(g_grabbed,false);
        ENTITY::FREEZE_ENTITY_POSITION(g_grabbed,true);
    }
    Vector3 forward,right; CameraBasis(forward,right);
    Vector3 p=ENTITY::GET_ENTITY_COORDS(ped,true);
    Vector3 hold=Add(p,Mul(forward,2.2f)); hold.z+=0.6f;
    ENTITY::SET_ENTITY_COORDS_NO_OFFSET(g_grabbed,hold.x,hold.y,hold.z,true,true,true);
}

static void ThrowGrabbed(Ped ped){
    if(!g_grabbed) return;
    Vector3 forward,right; CameraBasis(forward,right);
    Entity target=g_grabbed;
    ENTITY::FREEZE_ENTITY_POSITION(target,false);
    ENTITY::SET_ENTITY_DYNAMIC(target,true);
    ApplyDirectionalForce(target,forward,110.0f+ENTITY::GET_ENTITY_SPEED(ped)*2.0f);
    if(ENTITY::IS_ENTITY_A_PED(target)) DamageAndRagdoll((Ped)target,12,900);
    g_grabbed=0;
    g_superman.abilities.state.grabbing=false;
}

static void StartGroundPound(Ped ped){
    if(!g_superman.enabled || !g_superman.abilities.state.groundPound || g_groundPounding) return;
    if(!g_superman.abilities.state.flight && !ENTITY::IS_ENTITY_IN_AIR(ped)) return;
    g_groundPounding=true; g_groundPoundArmed=true;
    g_superman.abilities.state.flight=false;
    ENTITY::SET_ENTITY_HAS_GRAVITY(ped,true);
    Vector3 v=ENTITY::GET_ENTITY_VELOCITY(ped);
    ENTITY::SET_ENTITY_VELOCITY(ped,v.x,v.y,-120.0f);
}

static void UpdateGroundPound(Ped ped){
    if(!g_groundPounding) return;
    float h=ENTITY::GET_ENTITY_HEIGHT_ABOVE_GROUND(ped);
    if(h>2.0f) return;
    Vector3 p=ENTITY::GET_ENTITY_COORDS(ped,true);
    Ped target=FindTarget(ped,12.0f);
    if(target){
        Vector3 tp=ENTITY::GET_ENTITY_COORDS(target,true);
        Vector3 dir=Normalize3(MakeVec(tp.x-p.x,tp.y-p.y,0.4f));
        float energy=Physics::GroundImpactEnergy(g_superman.mass,120.0f);
        float force=std::sqrt(MaxF(1.0f,energy))*1.6f;
        ApplyDirectionalForce(target,dir,force);
        DamageAndRagdoll(target,(int)MinF(150.0f,energy*0.02f),1200);
    }
    // Local shockwave without an explosion: push every nearby pedestrian.
    Ped nearPed=FindTarget(ped,10.0f);
    if(nearPed){
        Vector3 np=ENTITY::GET_ENTITY_COORDS(nearPed,true);
        Vector3 dir=Normalize3(MakeVec(np.x-p.x,np.y-p.y,0.7f));
        ApplyDirectionalForce(nearPed,dir,80.0f);
        DamageAndRagdoll(nearPed,8,800);
    }
    ENTITY::SET_ENTITY_VELOCITY(ped,0.0f,0.0f,2.0f);
    g_groundPounding=false; g_groundPoundArmed=false;
}

static void UpdateFlight(Ped ped,float dt){
    if(!g_superman.enabled || !g_superman.abilities.state.flight || g_groundPounding) return;
    if(dt<0.001f) dt=0.001f; if(dt>0.05f) dt=0.05f;
    Vector3 forward,right; CameraBasis(forward,right);
    float fi=0,si=0,vi=0;
    if(GetAsyncKeyState('W')&0x8000) fi+=1.0f;
    if(GetAsyncKeyState('S')&0x8000) fi-=1.0f;
    if(GetAsyncKeyState('D')&0x8000) si+=1.0f;
    if(GetAsyncKeyState('A')&0x8000) si-=1.0f;
    if(GetAsyncKeyState(VK_SPACE)&0x8000) vi+=1.0f;
    if(GetAsyncKeyState(VK_CONTROL)&0x8000) vi-=1.0f;
    bool boost=(GetAsyncKeyState(VK_SHIFT)&0x8000)!=0 || g_superman.abilities.state.boost;
    float inputLen=std::sqrt(fi*fi+si*si+vi*vi);
    if(inputLen>1.0f){fi/=inputLen;si/=inputLen;vi/=inputLen;inputLen=1.0f;}
    Vector3 desired=Add(Add(Mul(forward,fi),Mul(right,si)),MakeVec(0.0f,0.0f,vi));
    desired=Mul(Normalize3(desired),boost?g_superman.boostSpeed:g_superman.flightSpeed);
    if(inputLen<=0.001f) desired=MakeVec(0.0f,0.0f,0.0f);
    Vector3 current=ENTITY::GET_ENTITY_VELOCITY(ped);
    float acceleration=boost?g_superman.boostAcceleration:g_superman.acceleration;
    float braking=boost?32.0f:48.0f;
    float rate=inputLen>0.001f?acceleration:braking;
    float maxChange=rate*dt;
    Vector3 diff=MakeVec(desired.x-current.x,desired.y-current.y,desired.z-current.z);
    float d=Length3(diff);
    if(d>maxChange && d>0.001f) diff=Mul(diff,maxChange/d);
    Vector3 next=Add(current,diff);
    float maxSpeed=boost?g_superman.boostSpeed:g_superman.flightSpeed;
    float speed=Length3(next);
    if(speed>maxSpeed) next=Mul(next,maxSpeed/speed);
    // Mild aerodynamic damping prevents oscillation while keeping inertia.
    if(inputLen<=0.001f) next=Mul(next,std::pow(0.35f,dt));
    ENTITY::SET_ENTITY_HAS_GRAVITY(ped,false);
    ENTITY::SET_ENTITY_MAX_SPEED(ped,maxSpeed+10.0f);
    ENTITY::SET_ENTITY_VELOCITY(ped,next.x,next.y,next.z);
    g_superman.velocity=Vec3(next.x,next.y,next.z);
    g_lastFlightSpeed=speed;
    // Stable upright orientation; only yaw follows the flight vector. No direct camera pitch is injected.
    if(speed>2.0f){
        float yaw=std::atan2(next.x,next.y)*57.295779513f;
        ENTITY::SET_ENTITY_HEADING(ped,yaw);
    }
}

static void UpdateAbilities(Ped ped,float dt){
    if(!g_superman.enabled) return;
    if(g_superman.abilities.state.superSpeed) PED::SET_PED_MOVE_RATE_OVERRIDE(ped,2.8f);
    else PED::SET_PED_MOVE_RATE_OVERRIDE(ped,1.0f);
    if(g_superman.abilities.state.godMode) ENTITY::SET_ENTITY_INVINCIBLE(ped,true);
    UpdateFlight(ped,dt);
    HeatVision(ped,dt);
    FreezeBreath(ped,dt);
    SuperBreath(ped);
    UpdateGrab(ped);
    UpdateGroundPound(ped);
    // Senses are a true GTA visual mode rather than a menu-only flag.
    GRAPHICS::SET_SEETHROUGH(g_superman.abilities.state.senses);
    if(g_superman.abilities.state.senses) GRAPHICS::SET_NIGHTVISION(true);
    else GRAPHICS::SET_NIGHTVISION(false);
}

static void HandleHotkeys(Ped ped){
    if(!g_superman.enabled) return;
    if(Pressed('F',lastF)){
        g_superman.abilities.state.flight=!g_superman.abilities.state.flight;
        ENTITY::SET_ENTITY_HAS_GRAVITY(ped,!g_superman.abilities.state.flight);
    }
    if(Pressed('G',lastG)) g_superman.abilities.state.superSpeed=!g_superman.abilities.state.superSpeed;
    if(Pressed('H',lastH)) g_superman.abilities.state.heatVision=!g_superman.abilities.state.heatVision;
    if(Pressed('J',lastJ)) g_superman.abilities.state.freezeBreath=!g_superman.abilities.state.freezeBreath;
    if(Pressed('K',lastK)) g_superman.abilities.state.superBreath=!g_superman.abilities.state.superBreath;
    if(Pressed('R',lastR)) SuperPunch(ped);
    if(Pressed('V',lastV)) StartGroundPound(ped);
    if(Pressed('E',lastE)) g_superman.abilities.state.grabbing=!g_superman.abilities.state.grabbing;
    if(Pressed('Q',lastQ)) ThrowGrabbed(ped);
    if(Pressed('B',lastB)) ToggleBulletTime();
    if(Pressed('X',lastX)) g_superman.abilities.state.senses=!g_superman.abilities.state.senses;
}

static void Activate(Ped ped){
    switch(g_selected){
        case 0: if(g_superman.enabled) SupermanOff(ped); else SupermanOn(ped); break;
        case 1: if(!g_superman.enabled) SupermanOn(ped); g_superman.abilities.state.flight=!g_superman.abilities.state.flight; break;
        case 2: if(!g_superman.enabled) SupermanOn(ped); g_superman.abilities.state.boost=!g_superman.abilities.state.boost; break;
        case 3: if(!g_superman.enabled) SupermanOn(ped); g_superman.abilities.state.superSpeed=!g_superman.abilities.state.superSpeed; break;
        case 4: if(!g_superman.enabled) SupermanOn(ped); GAMEPLAY::SET_SUPER_JUMP_THIS_FRAME(PLAYER::PLAYER_ID()); break;
        case 5: if(!g_superman.enabled) SupermanOn(ped); g_superman.abilities.state.godMode=!g_superman.abilities.state.godMode; ENTITY::SET_ENTITY_INVINCIBLE(ped,g_superman.abilities.state.godMode); break;
        case 6: if(!g_superman.enabled) SupermanOn(ped); g_superman.abilities.state.heatVision=!g_superman.abilities.state.heatVision; break;
        case 7: if(!g_superman.enabled) SupermanOn(ped); g_superman.abilities.state.freezeBreath=!g_superman.abilities.state.freezeBreath; break;
        case 8: if(!g_superman.enabled) SupermanOn(ped); g_superman.abilities.state.superBreath=!g_superman.abilities.state.superBreath; break;
        case 9: if(!g_superman.enabled) SupermanOn(ped); g_superman.abilities.state.superStrength=!g_superman.abilities.state.superStrength; break;
        case 10: if(!g_superman.enabled) SupermanOn(ped); g_superman.abilities.state.groundPound=!g_superman.abilities.state.groundPound; break;
        case 11: if(!g_superman.enabled) SupermanOn(ped); g_superman.abilities.state.grabbing=!g_superman.abilities.state.grabbing; break;
        case 12: if(!g_superman.enabled) SupermanOn(ped); ToggleBulletTime(); break;
        case 13: if(!g_superman.enabled) SupermanOn(ped); g_superman.abilities.state.senses=!g_superman.abilities.state.senses; break;
    }
}

static void UpdateMenu(Ped ped){
    if(Pressed(VK_F3,lastF3)) g_menuOpen=!g_menuOpen;
    if(!g_menuOpen) return;
    if(Pressed(VK_UP,lastUp)){--g_selected;if(g_selected<0)g_selected=MENU_COUNT-1;}
    if(Pressed(VK_DOWN,lastDown)){++g_selected;if(g_selected>=MENU_COUNT)g_selected=0;}
    if(Pressed(VK_RETURN,lastEnter)) Activate(ped);
}

static void UpdateSuperJump(Ped ped){
    if(!g_superman.enabled) return;
    if(g_superman.abilities.state.flight) return;
    if(GetAsyncKeyState(VK_SPACE)&0x8000) GAMEPLAY::SET_SUPER_JUMP_THIS_FRAME(PLAYER::PLAYER_ID());
    (void)ped;
}

static void MainUpdate(){
    Ped ped=PLAYER::PLAYER_PED_ID();
    if(!ped) return;
    float dt=GAMEPLAY::GET_FRAME_TIME();
    if(dt<0.001f)dt=0.001f;if(dt>0.05f)dt=0.05f;
    UpdateMenu(ped);
    HandleHotkeys(ped);
    UpdateSuperJump(ped);
    UpdateAbilities(ped,dt);
    DrawMenu();
}

void ScriptMain(){
    g_superman.Reset();
    while(true){MainUpdate();WAIT(0);}
}

BOOL APIENTRY DllMain(HMODULE hModule,DWORD ul_reason_for_call,LPVOID lpReserved){
    (void)lpReserved;
    if(ul_reason_for_call==DLL_PROCESS_ATTACH){g_module=hModule;scriptRegister(g_module,ScriptMain);}
    else if(ul_reason_for_call==DLL_PROCESS_DETACH){scriptUnregister(g_module);g_module=nullptr;}
    return TRUE;
}
