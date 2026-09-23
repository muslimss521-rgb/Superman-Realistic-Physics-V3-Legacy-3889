# Superman Realistic Physics V4 — GTA V Legacy 3889

Native C++ ASI project for GTA V Legacy 3889 using the supplied ScriptHookV SDK.
No NIBSHDotNet or ScriptHookVDotNet dependency is required.

## Implemented

- Stable camera-directed flight and hover
- Real frame-time acceleration and braking
- Inertia and aerodynamic damping
- Boost flight
- Stable upright steering
- Super speed
- Super jump
- Invincibility
- Heat vision ray damage
- Freeze breath hold/freeze effect
- Super breath directional force
- Super strength punch impulse, damage and ragdoll
- Ground pound descent and impact shockwave
- Grab, hold and throw
- Bullet time
- Super senses / see-through + night vision
- F3 menu plus direct keyboard controls

## Controls

- F3: menu
- F: flight
- Shift (hold): boost
- G: super speed
- H: heat vision
- J: freeze breath
- K: super breath
- R: super punch
- V: ground pound
- E: grab / hold
- Q: throw
- B: bullet time
- X: senses
- W/S/A/D: flight direction
- Space/Ctrl: vertical flight

## Build

GitHub Actions workflow: `.github/workflows/build.yml`

Output: `bin/Superman.asi`

## Installation

Copy the built `Superman.asi` to the GTA V Legacy 3889 root where ScriptHookV loads ASI plugins.

This project deliberately does not embed third-party PED costumes, custom animations, audio,
or particle packs. Those require user-owned/legally available assets and can be integrated separately.
