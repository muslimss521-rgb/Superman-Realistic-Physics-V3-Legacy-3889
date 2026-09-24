#include "Superman.h"
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstring>

static HMODULE g_module = nullptr;
static DWORD g_lastTick = 0;

static bool g_menuOpen = false;
static int g_selected = 0;

static bool lastF3=false,lastUp=false,lastDown=false,lastEnter=false;
static bool lastGround=false,lastClap=false,lastBoom=false,lastFlare=false;
static bool lastGrab=false,lastThrow=false,lastJump=false;

static Entity g_grabbed = 0;
static bool g_grabbing = false;
static DWORD g_lastAbility = 0;
static DWORD g_lastImpact = 0;
static DWORD g_lastHeat = 0;
static DWORD g_lastFreeze = 0;
static DWORD g_lastBreath = 0;

static int g_boostFxLeft = 0;
static int g_boostFxRight = 0;
static int g_heatFx = 0;
static int g_freezeFx = 0;

static const int MENU_COUNT = 16;
static const char* MENU[MENU_COUNT] =
{
    "Superman",
    "Flight",
    "Boost",
    "Super Speed",
    "Super Jump",
    "Invincibility",
    "Heat Vision",
    "Freeze Breath",
    "Super Breath",
    "Ground Pound",
    "Grab / Carry",
    "Throw",
    "Thunder Clap",
    "Sonic Boom",
    "Solar Flare",
    "X-Ray / Super Hearing"
};

static bool Pressed(int key, bool& last)
{
    bool now = (GetAsyncKeyState(key) & 0x8000) != 0;
    bool result = now && !last;
    last = now;
    return result;
}

static bool Held(int key)
{
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

static DWORD Now()
{
    return GetTickCount();
}

static float DT()
{
    DWORD n = Now();
    if (g_lastTick == 0)
    {
        g_lastTick = n;
        return 0.016f;
    }

    DWORD e = n - g_lastTick;
    g_lastTick = n;

    float dt = static_cast<float>(e) * 0.001f;
    return std::max(0.001f, std::min(0.033f, dt));
}

static float Len(const Vector3& v)
{
    return std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

static Vector3 Normalize(const Vector3& v)
{
    float l = Len(v);
    if (l < 0.0001f)
        return Vector3(0.0f,0.0f,0.0f);
    return Vector3(v.x/l,v.y/l,v.z/l);
}

static Vector3 Add(const Vector3& a,const Vector3& b)
{
    return Vector3(a.x+b.x,a.y+b.y,a.z+b.z);
}

static Vector3 Sub(const Vector3& a,const Vector3& b)
{
    return Vector3(a.x-b.x,a.y-b.y,a.z-b.z);
}

static Vector3 Mul(const Vector3& a,float s)
{
    return Vector3(a.x*s,a.y*s,a.z*s);
}

static Vector3 CameraForward()
{
    Vector3 r = CAM::GET_GAMEPLAY_CAM_ROT(2);
    const float d = 0.017453292519943295f;
    float pitch = r.x*d;
    float yaw = r.z*d;
    float cp = std::cos(pitch);
    return Normalize(Vector3(-std::sin(yaw)*cp,
                             std::cos(yaw)*cp,
                             std::sin(pitch)));
}

static void Text(const char* text,float x,float y,float scale)
{
    UI::SET_TEXT_FONT(0);
    UI::SET_TEXT_SCALE(0.0f,scale);
    UI::SET_TEXT_COLOUR(255,255,255,255);
    UI::SET_TEXT_PROPORTIONAL(true);
    UI::SET_TEXT_OUTLINE();
    UI::_SET_TEXT_ENTRY("STRING");
    UI::_ADD_TEXT_COMPONENT_STRING((char*)text);
    UI::_DRAW_TEXT(x,y);
}

static void DrawMenu()
{
    if (!g_menuOpen) return;

    float height = 0.105f + MENU_COUNT*0.034f;

    GRAPHICS::DRAW_RECT(0.18f,0.335f,0.36f,height,0,0,0,215);
    GRAPHICS::DRAW_RECT(0.18f,0.125f,0.36f,0.055f,25,70,160,245);
    Text("SUPERMAN  REALISTIC POWERS",0.035f,0.105f,0.38f);

    for (int i=0;i<MENU_COUNT;++i)
    {
        float y=0.165f+i*0.034f;
        if(i==g_selected)
            GRAPHICS::DRAW_RECT(0.18f,y+0.009f,0.33f,0.030f,50,110,210,220);
        Text(MENU[i],0.035f,y,0.255f);
    }

    Text("F3 CLOSE | UP/DOWN SELECT | ENTER ACTIVATE",
         0.035f,0.720f,0.21f);
}

static void StopFx()
{
    if(g_boostFxLeft) { GRAPHICS::STOP_PARTICLE_FX_LOOPED(g_boostFxLeft,false); g_boostFxLeft=0; }
    if(g_boostFxRight){ GRAPHICS::STOP_PARTICLE_FX_LOOPED(g_boostFxRight,false); g_boostFxRight=0; }
    if(g_heatFx)      { GRAPHICS::STOP_PARTICLE_FX_LOOPED(g_heatFx,false); g_heatFx=0; }
    if(g_freezeFx)    { GRAPHICS::STOP_PARTICLE_FX_LOOPED(g_freezeFx,false); g_freezeFx=0; }
}

static void StartBoostFx(Ped ped,float scale)
{
    if(g_boostFxLeft || g_boostFxRight) return;

    GRAPHICS::REQUEST_NAMED_PTFX_ASSET((char*)"core");
    if(!GRAPHICS::HAS_NAMED_PTFX_ASSET_LOADED((char*)"core")) return;

    GRAPHICS::SET_PTFX_ASSET_NEXT_CALL((char*)"core");
    g_boostFxLeft=GRAPHICS::START_PARTICLE_FX_LOOPED_ON_ENTITY(
        (char*)"ent_sht_flame",ped,-0.28f,-1.0f,0.08f,
        90.0f,0.0f,0.0f,scale,false,false,false);

    GRAPHICS::SET_PTFX_ASSET_NEXT_CALL((char*)"core");
    g_boostFxRight=GRAPHICS::START_PARTICLE_FX_LOOPED_ON_ENTITY(
        (char*)"ent_sht_flame",ped,0.28f,-1.0f,0.08f,
        90.0f,0.0f,0.0f,scale,false,false,false);
}

static void UpdateBoostFx(Ped ped,float speed,bool boost)
{
    if(!boost || !g_superman.enabled || !g_superman.abilities.state.flight)
    {
        if(g_boostFxLeft) { GRAPHICS::STOP_PARTICLE_FX_LOOPED(g_boostFxLeft,false); g_boostFxLeft=0; }
        if(g_boostFxRight){ GRAPHICS::STOP_PARTICLE_FX_LOOPED(g_boostFxRight,false); g_boostFxRight=0; }
        return;
    }

    float f=std::min(1.0f,speed/std::max(1.0f,g_superman.boostSpeed));
    StartBoostFx(ped,0.45f+f*0.8f);

    float s=0.45f+f*1.15f;
    if(g_boostFxLeft)
    {
        GRAPHICS::SET_PARTICLE_FX_LOOPED_SCALE(g_boostFxLeft,s);
        GRAPHICS::SET_PARTICLE_FX_LOOPED_COLOUR(g_boostFxLeft,1.0f,0.55f,0.10f);
    }
    if(g_boostFxRight)
    {
        GRAPHICS::SET_PARTICLE_FX_LOOPED_SCALE(g_boostFxRight,s);
        GRAPHICS::SET_PARTICLE_FX_LOOPED_COLOUR(g_boostFxRight,1.0f,0.55f,0.10f);
    }

    if(Now()-g_lastImpact>90 && speed>55.0f)
    {
        Vector3 p=ENTITY::GET_ENTITY_COORDS(ped,true);
        Vector3 v=ENTITY::GET_ENTITY_VELOCITY(ped);
        GRAPHICS::SET_PTFX_ASSET_NEXT_CALL((char*)"core");
        GRAPHICS::START_PARTICLE_FX_NON_LOOPED_AT_COORD(
            (char*)"ent_sht_flame",
            p.x-v.x*0.018f,p.y-v.y*0.018f,p.z-v.z*0.018f,
            0,0,0,0.35f+f*0.8f,false,false,false);
        g_lastImpact=Now();
    }

    if(speed>120.0f)
        CAM::SHAKE_GAMEPLAY_CAM((char*)"SMALL_EXPLOSION_SHAKE",
            std::min(0.32f,0.05f+(speed-120.0f)/450.0f));
}

static Ped FindPedInRay(Ped player,float maxRange)
{
    Vector3 origin=CAM::GET_GAMEPLAY_CAM_COORD();
    Vector3 dir=CameraForward();

    int handles[256];
    int count=worldGetAllPeds(handles,256);
    Ped best=0;
    float bestScore=999999.0f;

    for(int i=0;i<count;++i)
    {
        Ped p=(Ped)handles[i];
        if(!p || p==player || !ENTITY::DOES_ENTITY_EXIST(p)) continue;
        if(ENTITY::IS_ENTITY_DEAD(p)) continue;

        Vector3 q=ENTITY::GET_ENTITY_COORDS(p,true);
        Vector3 d=Sub(q,origin);
        float along=d.x*dir.x+d.y*dir.y+d.z*dir.z;
        if(along<1.0f || along>maxRange) continue;

        Vector3 closest=Add(origin,Mul(dir,along));
        float side=Len(Sub(q,closest));

        if(side<4.0f && side<bestScore)
        {
            bestScore=side;
            best=p;
        }
    }
    return best;
}

static void DamageAndPush(Ped target,float damage,const Vector3& direction,float force)
{
    if(!target || !ENTITY::DOES_ENTITY_EXIST(target)) return;

    int hp=ENTITY::GET_ENTITY_HEALTH(target);
    if(hp>0)
    {
        int newHp=std::max(1,hp-(int)std::max(0.0f,damage));
        ENTITY::SET_ENTITY_HEALTH(target,newHp);
    }

    ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(
        target,1,
        direction.x*force,direction.y*force,direction.z*force,
        false,true,true,false);
}

static void HeatVision(Ped player)
{
    if(!Held('H')) return;

    Vector3 o=CAM::GET_GAMEPLAY_CAM_COORD();
    Vector3 d=CameraForward();
    Vector3 end=Add(o,Mul(d,150.0f));

    GRAPHICS::DRAW_LINE(o.x,o.y,o.z,end.x,end.y,end.z,255,30,10,230);
    GRAPHICS::DRAW_LINE(o.x,o.y,o.z,end.x,end.y,end.z,255,180,80,150);

    Ped target=FindPedInRay(player,150.0f);
    if(target && Now()-g_lastHeat>55)
    {
        DamageAndPush(target,9.0f,d,2.0f);
        g_lastHeat=Now();
    }

    if(Now()-g_lastHeat>35)
    {
        GRAPHICS::SET_PTFX_ASSET_NEXT_CALL((char*)"core");
        GRAPHICS::START_PARTICLE_FX_NON_LOOPED_AT_COORD(
            (char*)"ent_sht_flame",end.x,end.y,end.z,0,0,0,0.18f,false,false,false);
    }
}

static void FreezeBreath(Ped player)
{
    if(!Held('J')) return;

    Vector3 o=CAM::GET_GAMEPLAY_CAM_COORD();
    Vector3 d=CameraForward();
    Vector3 end=Add(o,Mul(d,38.0f));

    GRAPHICS::DRAW_LINE(o.x,o.y,o.z,end.x,end.y,end.z,150,220,255,180);

    Ped target=FindPedInRay(player,38.0f);
    if(target && Now()-g_lastFreeze>100)
    {
        Vector3 v=ENTITY::GET_ENTITY_VELOCITY(target);
        ENTITY::SET_ENTITY_VELOCITY(target,v.x*0.12f,v.y*0.12f,v.z*0.12f);
        PED::SET_PED_TO_RAGDOLL(target,500,900,0,false,false,false);
        g_lastFreeze=Now();
    }

    if(Now()-g_lastFreeze>80)
    {
        GRAPHICS::SET_PTFX_ASSET_NEXT_CALL((char*)"core");
        GRAPHICS::START_PARTICLE_FX_NON_LOOPED_AT_COORD(
            (char*)"ent_sht_steam",end.x,end.y,end.z,0,0,0,0.45f,false,false,false);
    }
}

static void SuperBreath(Ped player)
{
    if(!Held('K')) return;

    Vector3 o=CAM::GET_GAMEPLAY_CAM_COORD();
    Vector3 d=CameraForward();
    Vector3 end=Add(o,Mul(d,45.0f));

    GRAPHICS::DRAW_LINE(o.x,o.y,o.z,end.x,end.y,end.z,180,240,255,130);

    int handles[256];
    int count=worldGetAllPeds(handles,256);

    for(int i=0;i<count;++i)
    {
        Ped p=(Ped)handles[i];
        if(!p || p==player || !ENTITY::DOES_ENTITY_EXIST(p)) continue;
        Vector3 q=ENTITY::GET_ENTITY_COORDS(p,true);
        Vector3 rel=Sub(q,o);
        float along=rel.x*d.x+rel.y*d.y+rel.z*d.z;
        if(along<2.0f || along>45.0f) continue;

        Vector3 near=Add(o,Mul(d,along));
        if(Len(Sub(q,near))<5.0f)
            ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(
                p,1,d.x*18.0f,d.y*18.0f,d.z*18.0f,false,true,true,false);
    }
}

static void GroundPound(Ped player)
{
    if(!Pressed('V',lastGround)) return;
    if(!g_superman.enabled) return;

    Vector3 p=ENTITY::GET_ENTITY_COORDS(player,true);
    Vector3 v=ENTITY::GET_ENTITY_VELOCITY(player);
    float speed=Len(v);

    float energy=0.5f*g_superman.mass*speed*speed;
    float scale=std::min(8.0f,std::max(1.0f,energy/50000.0f));

    GRAPHICS::ADD_EXPLOSION(p.x,p.y,p.z-0.8f,0,scale,true,false,
        std::min(1.0f,scale/5.0f));

    int handles[256];
    int count=worldGetAllPeds(handles,256);

    for(int i=0;i<count;++i)
    {
        Ped q=(Ped)handles[i];
        if(!q || q==player || !ENTITY::DOES_ENTITY_EXIST(q)) continue;

        Vector3 qp=ENTITY::GET_ENTITY_COORDS(q,true);
        Vector3 delta=Sub(qp,p);
        float dist=Len(delta);
        if(dist<18.0f && dist>0.1f)
        {
            float f=(1.0f-dist/18.0f)*35.0f;
            Vector3 dir=Normalize(delta);
            DamageAndPush(q,scale*7.0f,dir,f);
        }
    }

    CAM::SHAKE_GAMEPLAY_CAM((char*)"LARGE_EXPLOSION_SHAKE",0.55f);
}

static void ThunderClap(Ped player)
{
    if(!Pressed('T',lastClap)) return;
    Vector3 p=ENTITY::GET_ENTITY_COORDS(player,true);

    int handles[256];
    int count=worldGetAllPeds(handles,256);
    for(int i=0;i<count;++i)
    {
        Ped q=(Ped)handles[i];
        if(!q || q==player || !ENTITY::DOES_ENTITY_EXIST(q)) continue;
        Vector3 qp=ENTITY::GET_ENTITY_COORDS(q,true);
        Vector3 delta=Sub(qp,p);
        float dist=Len(delta);
        if(dist<25.0f && dist>0.1f)
            DamageAndPush(q,15.0f,Normalize(delta),(25.0f-dist)*1.5f);
    }

    GRAPHICS::ADD_EXPLOSION(p.x,p.y,p.z,8,0.15f,true,true,0.7f);
    CAM::SHAKE_GAMEPLAY_CAM((char*)"LARGE_EXPLOSION_SHAKE",0.35f);
}

static void SonicBoom(Ped player)
{
    if(!Pressed('Y',lastBoom)) return;
    if(!g_superman.abilities.state.flight) return;

    Vector3 p=ENTITY::GET_ENTITY_COORDS(player,true);
    Vector3 d=CameraForward();
    float speed=Len(ENTITY::GET_ENTITY_VELOCITY(player));

    if(speed<45.0f) speed=45.0f;

    int handles[256];
    int count=worldGetAllPeds(handles,256);
    for(int i=0;i<count;++i)
    {
        Ped q=(Ped)handles[i];
        if(!q || q==player || !ENTITY::DOES_ENTITY_EXIST(q)) continue;
        Vector3 qp=ENTITY::GET_ENTITY_COORDS(q,true);
        Vector3 delta=Sub(qp,p);
        float dist=Len(delta);
        if(dist<35.0f && dist>0.1f)
            DamageAndPush(q,speed*0.10f,Normalize(delta),speed*0.7f);
    }

    GRAPHICS::ADD_EXPLOSION(p.x+d.x*2.0f,p.y+d.y*2.0f,p.z+d.z*2.0f,
        8,0.35f,true,true,0.8f);
    CAM::SHAKE_GAMEPLAY_CAM((char*)"LARGE_EXPLOSION_SHAKE",0.5f);
}

static void SolarFlare(Ped player)
{
    if(!Pressed('U',lastFlare)) return;

    Vector3 p=ENTITY::GET_ENTITY_COORDS(player,true);
    int handles[256];
    int count=worldGetAllPeds(handles,256);

    for(int i=0;i<count;++i)
    {
        Ped q=(Ped)handles[i];
        if(!q || q==player || !ENTITY::DOES_ENTITY_EXIST(q)) continue;
        Vector3 qp=ENTITY::GET_ENTITY_COORDS(q,true);
        float dist=Len(Sub(qp,p));
        if(dist<22.0f)
            DamageAndPush(q,35.0f,Normalize(Sub(qp,p)),10.0f);
    }

    GRAPHICS::ADD_EXPLOSION(p.x,p.y,p.z,70,0.2f,true,true,1.0f);
    CAM::SHAKE_GAMEPLAY_CAM((char*)"LARGE_EXPLOSION_SHAKE",0.65f);
}

static void GrabCarryThrow(Ped player)
{
    if(Pressed('E',lastGrab))
    {
        if(!g_grabbed)
        {
            g_grabbed=FindPedInRay(player,8.0f);
            if(g_grabbed)
            {
                g_grabbing=true;
                ENTITY::SET_ENTITY_HAS_GRAVITY(g_grabbed,false);
                ENTITY::SET_ENTITY_COLLISION(g_grabbed,false,true);
            }
        }
        else
        {
            g_grabbing=false;
            ENTITY::SET_ENTITY_HAS_GRAVITY(g_grabbed,true);
            ENTITY::SET_ENTITY_COLLISION(g_grabbed,true,true);
            g_grabbed=0;
        }
    }

    if(g_grabbed && g_grabbing)
    {
        Vector3 p=ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(player,0.0f,1.0f,0.7f);
        ENTITY::SET_ENTITY_COORDS_NO_OFFSET(g_grabbed,p.x,p.y,p.z,false,false,false);
        ENTITY::SET_ENTITY_VELOCITY(g_grabbed,0,0,0);
    }

    if(Pressed('Q',lastThrow) && g_grabbed)
    {
        Vector3 d=CameraForward();
        ENTITY::SET_ENTITY_HAS_GRAVITY(g_grabbed,true);
        ENTITY::SET_ENTITY_COLLISION(g_grabbed,true,true);
        ENTITY::SET_ENTITY_VELOCITY(g_grabbed,d.x*45.0f,d.y*45.0f,d.z*45.0f+12.0f);
        g_grabbed=0;
        g_grabbing=false;
    }
}

static void SuperJump()
{
    if(g_superman.enabled && Held(VK_SPACE))
        GAMEPLAY::SET_SUPER_JUMP_THIS_FRAME(PLAYER::PLAYER_ID());
}

static void UpdateFlight(Ped ped,float dt)
{
    if(!g_superman.enabled || !g_superman.abilities.state.flight)
    {
        if(!g_superman.abilities.state.boost) StopFx();
        return;
    }

    Vector3 cam=CAM::GET_GAMEPLAY_CAM_ROT(2);
    const float d=0.017453292519943295f;
    float pitch=cam.x*d,yaw=cam.z*d,cp=std::cos(pitch);

    Vector3 forward=Normalize(Vector3(-std::sin(yaw)*cp,
                                      std::cos(yaw)*cp,
                                      std::sin(pitch)));
    Vector3 right=Vector3(std::cos(yaw),std::sin(yaw),0.0f);

    float f=0,s=0,u=0;
    if(Held('W')) f+=1;
    if(Held('S')) f-=1;
    if(Held('D')) s+=1;
    if(Held('A')) s-=1;
    if(Held(VK_SPACE)) u+=1;
    if(Held(VK_CONTROL)) u-=1;

    float inLen=std::sqrt(f*f+s*s+u*u);
    if(inLen>1.0f){f/=inLen;s/=inLen;u/=inLen;}

    bool boost=Held(VK_LSHIFT)||Held(VK_RSHIFT)||
               g_superman.abilities.state.boost;

    float maxSpeed=boost?g_superman.boostSpeed:g_superman.flightSpeed;
    float accel=boost?g_superman.boostAcceleration:g_superman.acceleration;

    Vector3 desired=Add(
        Add(Mul(forward,f*maxSpeed),Mul(right,s*maxSpeed)),
        Vector3(0,0,u*maxSpeed));

    Vector3 vel=ENTITY::GET_ENTITY_VELOCITY(ped);

    float response=1.0f-std::exp(-accel*dt/std::max(1.0f,maxSpeed));
    vel=Add(vel,Mul(Sub(desired,vel),response));

    float speed=Len(vel);

    if(speed>0.001f)
    {
        float drag=boost?0.035f:0.065f;
        float dragAmount=std::min(speed,drag*speed*speed*dt);
        vel=Sub(vel,Mul(Normalize(vel),dragAmount));
    }

    if(inLen<0.01f)
    {
        float brake=std::exp(-(boost?0.35f:1.8f)*dt);
        vel=Mul(vel,brake);
    }

    float finalSpeed=Len(vel);
    if(finalSpeed>maxSpeed)
        vel=Mul(Normalize(vel),maxSpeed);

    ENTITY::SET_ENTITY_HAS_GRAVITY(ped,false);
    ENTITY::SET_ENTITY_MAX_SPEED(ped,maxSpeed);
    ENTITY::SET_ENTITY_VELOCITY(ped,vel.x,vel.y,vel.z);

    if(inLen>0.01f)
        ENTITY::SET_ENTITY_ROTATION(ped,cam.x,0.0f,cam.z,2,true);

    g_superman.velocity=Vec3(vel.x,vel.y,vel.z);
    UpdateBoostFx(ped,Len(vel),boost);
}

static void SupermanOn(Ped p)
{
    g_superman.enabled=true;
    ENTITY::SET_ENTITY_INVINCIBLE(p,true);
    g_superman.abilities.state.superSpeed=true;
    PED::SET_PED_MOVE_RATE_OVERRIDE(p,2.0f);
}

static void SupermanOff(Ped p)
{
    g_superman.enabled=false;
    g_superman.abilities.state={};
    g_grabbed=0;
    g_grabbing=false;
    StopFx();

    ENTITY::SET_ENTITY_INVINCIBLE(p,false);
    ENTITY::SET_ENTITY_HAS_GRAVITY(p,true);
    ENTITY::SET_ENTITY_MAX_SPEED(p,1000.0f);
    PED::SET_PED_MOVE_RATE_OVERRIDE(p,1.0f);
    ENTITY::SET_ENTITY_VELOCITY(p,0,0,0);
    g_superman.velocity=Vec3();
    GAMEPLAY::SET_TIME_SCALE(1.0f);
}

static void Activate(Ped p)
{
    switch(g_selected)
    {
        case 0: g_superman.enabled?SupermanOff(p):SupermanOn(p); break;
        case 1:
            SupermanOn(p);
            g_superman.abilities.state.flight=!g_superman.abilities.state.flight;
            ENTITY::SET_ENTITY_HAS_GRAVITY(p,!g_superman.abilities.state.flight);
            break;
        case 2: SupermanOn(p); g_superman.abilities.state.boost=!g_superman.abilities.state.boost; break;
        case 3: SupermanOn(p); g_superman.abilities.state.superSpeed=!g_superman.abilities.state.superSpeed; break;
        case 4: SupermanOn(p); break;
        case 5: SupermanOn(p); g_superman.abilities.state.godMode=!g_superman.abilities.state.godMode; ENTITY::SET_ENTITY_INVINCIBLE(p,g_superman.abilities.state.godMode); break;
        case 6: SupermanOn(p); g_superman.abilities.state.heatVision=!g_superman.abilities.state.heatVision; break;
        case 7: SupermanOn(p); g_superman.abilities.state.freezeBreath=!g_superman.abilities.state.freezeBreath; break;
        case 8: SupermanOn(p); g_superman.abilities.state.superBreath=!g_superman.abilities.state.superBreath; break;
        case 9: GroundPound(p); break;
        case 10: GrabCarryThrow(p); break;
        case 11: if(g_grabbed) { lastThrow=false; GrabCarryThrow(p); } break;
        case 12: ThunderClap(p); break;
        case 13: SonicBoom(p); break;
        case 14: SolarFlare(p); break;
        case 15: SupermanOn(p); g_superman.abilities.state.xray=!g_superman.abilities.state.xray; break;
    }
}

static void MenuUpdate(Ped p)
{
    if(Pressed(VK_F3,lastF3)) g_menuOpen=!g_menuOpen;
    if(!g_menuOpen) return;

    if(Pressed(VK_UP,lastUp)){--g_selected;if(g_selected<0)g_selected=MENU_COUNT-1;}
    if(Pressed(VK_DOWN,lastDown)){++g_selected;if(g_selected>=MENU_COUNT)g_selected=0;}
    if(Pressed(VK_RETURN,lastEnter)) Activate(p);
}

static void UpdateSuperSpeed(Ped p)
{
    if(!g_superman.enabled) return;
    if(g_superman.abilities.state.superSpeed)
    {
        PED::SET_PED_MOVE_RATE_OVERRIDE(p,3.0f);
        if(Held('W'))
            PED::SET_PED_CAN_RAGDOLL(p,false);
    }
    else
        PED::SET_PED_MOVE_RATE_OVERRIDE(p,1.0f);
}

static void UpdateXRay()
{
    if(!g_superman.enabled || !g_superman.abilities.state.xray) return;

    int handles[256];
    int count=worldGetAllPeds(handles,256);
    Vector3 o=CAM::GET_GAMEPLAY_CAM_COORD();
    Vector3 d=CameraForward();

    for(int i=0;i<count;++i)
    {
        Ped p=(Ped)handles[i];
        if(!p || !ENTITY::DOES_ENTITY_EXIST(p)) continue;
        Vector3 q=ENTITY::GET_ENTITY_COORDS(p,true);
        Vector3 rel=Sub(q,o);
        float along=rel.x*d.x+rel.y*d.y+rel.z*d.z;
        if(along>2.0f && along<100.0f && Len(Sub(q,Add(o,Mul(d,along))))<3.0f)
            GRAPHICS::DRAW_LINE(o.x,o.y,o.z,q.x,q.y,q.z,0,220,255,150);
    }
}

static void AbilitiesUpdate(Ped p,float dt)
{
    if(!g_superman.enabled) return;

    UpdateFlight(p,dt);
    UpdateSuperSpeed(p);
    SuperJump();
    HeatVision(p);
    FreezeBreath(p);
    SuperBreath(p);
    GroundPound(p);
    ThunderClap(p);
    SonicBoom(p);
    SolarFlare(p);
    GrabCarryThrow(p);
    UpdateXRay();

    if(g_superman.abilities.state.godMode)
        ENTITY::SET_ENTITY_INVINCIBLE(p,true);

    g_superman.Tick(dt);
}

void ScriptMain()
{
    g_superman.Reset();
    g_lastTick=Now();

    while(true)
    {
        Ped p=PLAYER::PLAYER_PED_ID();
        if(p && ENTITY::DOES_ENTITY_EXIST(p))
        {
            float dt=DT();
            MenuUpdate(p);
            AbilitiesUpdate(p,dt);
            DrawMenu();
        }
        WAIT(0);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule,DWORD reason,LPVOID reserved)
{
    (void)reserved;
    if(reason==DLL_PROCESS_ATTACH)
    {
        g_module=hModule;
        scriptRegister(g_module,ScriptMain);
    }
    else if(reason==DLL_PROCESS_DETACH)
    {
        StopFx();
        scriptUnregister(g_module);
        g_module=nullptr;
    }
    return TRUE;
}
