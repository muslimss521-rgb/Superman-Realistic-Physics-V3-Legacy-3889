#include <windows.h>
#include "Superman.h"

// Native ASI entry point. GTA/ScriptHookV integration is intentionally kept
// behind this small controller so the physics core remains testable.

static SupermanController g_superman;

DWORD WINAPI MainThread(LPVOID) {
    // The actual ScriptHookV native calls are connected here when the
    // ScriptHookV SDK is supplied to the repository.
    for (;;) {
        Sleep(16);
        g_superman.Tick(0.016f);
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
