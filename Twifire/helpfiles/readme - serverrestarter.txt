Twifire has 'mc_crash_fix' and 'mc_never_crash' to prevent most crashes... but what if the server completely crashes?
The program stops responding, there's an infinite loop, or for whatever other reason, the entire program just fails.

The answer: an external, second program, also running, that keeps track of the server.


Just copy ServerRestarter.exe into JK2/GameData,
then launch ServerRestarter.exe,
then choose your server's exe (jk2mp, jk2mp2, jk2ded... ),
then input your server's settings (similar to those in a .bat launch script),
And click 'Start', and now your server is running safely.