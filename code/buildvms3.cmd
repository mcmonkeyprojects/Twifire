set include=
cd game
@set include=
@set savedpath=%path%
@set path=%path%;..\..\..\bin
echo off
del /q vm
if not exist vm\nul mkdir vm
cd vm
set cc=..\..\..\bin\lcc -A -DQ3_VM -DMISSIONPACK -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui %1

%cc%  ../g_main.c
@if errorlevel 1 goto quit
echo -----processing g_main
%cc%  ../g_syscalls.c
echo -----processing g_syscalls
@if errorlevel 1 goto quit

%cc%  ../bg_misc.c
echo -----processing bg_misc
@if errorlevel 1 goto quit
%cc%  ../bg_lib.c
echo -----processing bg_lib
@if errorlevel 1 goto quit
%cc%  ../bg_pmove.c
echo -----processing bg_pmove
@if errorlevel 1 goto quit
%cc%  ../bg_saber.c
echo -----processing bg_saber
@if errorlevel 1 goto quit
%cc%  ../bg_slidemove.c
echo -----processing bg_slidemove
@if errorlevel 1 goto quit
%cc%  ../bg_panimate.c
echo -----processing bg_panimate
@if errorlevel 1 goto quit
%cc%  ../bg_weapons.c
echo -----processing bg_weapons
@if errorlevel 1 goto quit
%cc%  ../q_math.c
echo -----processing bg_math
@if errorlevel 1 goto quit
%cc%  ../q_shared.c
echo -----processing q_shared
@if errorlevel 1 goto quit

%cc%  ../ai_main.c
echo -----processing ai_main
@if errorlevel 1 goto quit
%cc%  ../ai_util.c
echo -----processing ai_util
@if errorlevel 1 goto quit
%cc%  ../ai_wpnav.c
echo -----processing ai_wpnav
@if errorlevel 1 goto quit

%cc%  ../g_active.c
echo -----processing g_active
@if errorlevel 1 goto quit

%cc%  ../g_arenas.c
echo -----processing g_arenas
@if errorlevel 1 goto quit
%cc%  ../g_bot.c
echo -----processing g_bot
@if errorlevel 1 goto quit
%cc%  ../g_client.c
echo -----processing g_client
@if errorlevel 1 goto quit
%cc%  ../g_cmds.c
echo -----processing g_cmds
@if errorlevel 1 goto quit
%cc%  ../g_combat.c
echo -----processing g_combat
@if errorlevel 1 goto quit
%cc%  ../g_items.c
echo -----processing g_items
@if errorlevel 1 goto quit
%cc%  ../g_log.c
echo -----processing g_log
@if errorlevel 1 goto quit
%cc%  ../g_mem.c
echo -----processing g_mem
@if errorlevel 1 goto quit
%cc%  ../g_misc.c
echo -----processing g_misc
@if errorlevel 1 goto quit
%cc%  ../g_missile.c
echo -----processing g_missile
@if errorlevel 1 goto quit
%cc%  ../g_mover.c
echo -----processing g_mover
@if errorlevel 1 goto quit
%cc%  ../g_object.c
echo -----processing g_object
@if errorlevel 1 goto quit
%cc%  ../g_saga.c
echo -----processing g_saga
@if errorlevel 1 goto quit
%cc%  ../g_session.c
echo -----processing g_session
@if errorlevel 1 goto quit
%cc%  ../g_spawn.c
echo -----processing g_spawn
@if errorlevel 1 goto quit
%cc%  ../g_svcmds.c
echo -----processing g_svcmds
@if errorlevel 1 goto quit
%cc%  ../g_target.c
echo -----processing g_target
@if errorlevel 1 goto quit
%cc%  ../g_team.c
echo -----processing g_team
@if errorlevel 1 goto quit
%cc%  ../g_trigger.c
echo -----processing g_trigger
@if errorlevel 1 goto quit
%cc%  ../g_utils.c
echo -----processing g_utils
@if errorlevel 1 goto quit
%cc%  ../g_weapon.c
echo -----processing g_weapon
@if errorlevel 1 goto quit
%cc%  ../w_force.c
echo -----processing g_force
@if errorlevel 1 goto quit
%cc%  ../w_saber.c
echo -----processing g_saber
@if errorlevel 1 goto quit
%cc%  ../g_nox_syscalls.c
echo -----processing g_nox_syscalls
@if errorlevel 1 goto quit
%cc%  ../g_twifireadmincommands.c
echo -----processing g_twifireadmincommands
@if errorlevel 1 goto quit

echo on
..\..\..\bin\q3asm -f ../game
@if errorlevel 1 goto quit

@set path=%savedpath%
@set savedpath=
echo off
cd ..

cd ..

cd ..
echo on
copy "code\base\vm\jk2mpgame.qvm" "twifire\vm\jk2mpgame.qvm"
echo off
pause
exit
quit
close

:quit
echo ERROR!
pause