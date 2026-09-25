# Superman Legacy 3889 — v0.1

Первая реальная версия проекта: standalone C++ ASI без NIBSHDotNet.

## Цель v0.1

- ASI корректно загружается через ScriptHookV.
- F3 — Superman ON/OFF.
- F5 — Flight ON/OFF.
- Полёт относительно камеры.
- W/S — ускорение/торможение вперёд/назад.
- A/D — боковое направление.
- Space — вверх.
- Ctrl — вниз.
- Shift — Boost.
- F8 — аварийно отключить полёт.

## Сборка

Проект использует официальный ScriptHookV SDK, приложенный к проекту.
Открыть `Superman.vcxproj` в Visual Studio и собрать `Release | x64`.

Готовый файл:
`bin/Superman.asi`

В GTA V:
`Grand Theft Auto V/Superman.asi`

Для запуска также нужен обычный `dinput8.dll`/ScriptHookV ASI loader.

## Важно

Это собственная реализация. Код JulioNIB не используется и не копируется.
