#include "ScriptHookV/main.h"
#include "ScriptHookV/natives.h"
#include "Flight.h"

void ScriptMain()
{
    Flight::Initialize();

    while (true)
    {
        Flight::Update();
        scriptWait(0);
    }
}
