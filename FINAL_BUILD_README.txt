SUPERMAN GTA V LEGACY 3889
Final source project package

This archive is based on the supplied working project.
Upload the project contents to the GitHub repository and run the existing
GitHub Actions workflow.

Target:
- GTA V Legacy 3889
- Native C++ / ScriptHookV
- Superman.asi
- No ScriptHookVDotNet/NIB dependency

Important:
Build against the ScriptHookV SDK already used by this project.


FIXED BUILD
- Replaced broken src/main.cpp with balanced, compilable source.
- Added <cmath> and corrected GAMEPLAY::GET_GAME_TIMER().
- Flight/boost physics is applied directly to the GTA entity.
- Added boost visual light/flame/smoke effects.
- Added heat vision, freeze breath, super breath, ground pound, sonic boom and solar flare handlers.
- Kept ScriptHookV.lib and User32.lib linkage in Superman.vcxproj.
