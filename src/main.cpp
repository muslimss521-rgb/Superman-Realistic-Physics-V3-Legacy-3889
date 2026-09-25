#include "main.h"
#include "script.h"
#include "Superman/Superman.h"

void ScriptMain(){
    Superman::Initialize();
    while(true){ Superman::Update(); WAIT(0); }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID){
    if(reason==DLL_PROCESS_ATTACH){ DisableThreadLibraryCalls(hModule); scriptRegister(hModule, ScriptMain); }
    else if(reason==DLL_PROCESS_DETACH){ scriptUnregister(hModule); }
    return TRUE;
}
