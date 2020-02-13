New to server hosting? This guide will show you how to create a simple Twifire server

-First, you must open your Gamedata folder. This is where all relevant bits of JK2 information are stored.
It is defaultly at C:\Program Files\LucasArts\Star Wars JK II Jedi Outcast\GameData but you may have
installed it elsewhere.

-Copy the folder 'Twifire' (the folder containing this readme!) and paste it into your gamedata folder

-You need a server executable. This is usually JK2MP.exe, but professionals prefer JK2ded.exe
///// NOTE: HIGHLY RECOMMENDED YOU USE JK2MP2, JK2DED2, OR JK2:SE INSTEAD OF JK2MF OR JK2DED
///// JK2MP2 AND JK2DED2 ARE INCLUDED in Twifire/helpers
///// JK2SE (Most highly recommended!) IS AT https://github.com/mcmonkeyprojects/JK2SE/

-Open your Twifire folder (JK2/Gamedata/Twifire), and copy the file "Launcher"

-Open your Gamedata folder (JK2/Gamedata) and paste the file "Launcher"

-Double click on "Launcher" (JK2/Gamedata/Launcher.bat)

-A black and white console window will appear.

-This is your server. If that's open, your server is running.

-Open JK2, go to the server list, find 'Source: Internet' at the top and click it
until it becomes 'Source: Local'
You will see 'My Twifire Server' listed below.
You can join the fully working Twifire server.


Server interaction:
-You can do many JK2 commands from the server console. Just type them in and press enter.
These include "map ffa_yavin" or "mckick Padawan" or many more. (See JK2/Gamedata/Twifire/readmes/server commands)


Advanced setup:
-You may want to change, say, the name of your server.
For this setting and many others, open JK2/Gamedata/Twifire/ExampleSettings.cfg in a text editor like Notepad or Notepad++ or even MS Word
(It's labeled as a fancy script file, but really it's just plain text)

-Browse through the settings listed and edit to your liking.

-Save the file as exactly what it was. Do not edit the file name or remove the .cfg
(note: be careful not to save it as a standard text file. This will be 'ExampleSettings.cfg.txt' - which won't work.) [Press 'Save', not 'Save As']

-Restart your server by closing the window and clicking on Launcher.bat again





Getting your server online:
-Okay, you have your server up and running, you've changed the settings to your liking,
and you've probably used the other included readmes and tutorials to set up admin accounts and similar (if not, you might want to do so before continuing)
Now you want to let other people play with you

-This requires something called 'Port Forwarding'
That basically means telling your router to put your server online.

-Port Forwarding involves changing router settings... how to do it is defined in your router's manual.
Read the manual or use google to find details on how to adjust router settings, port forwarding specifically

-Port forward port 28070 on both TCP and UDP to your local IP

-Restart your server

-Your server will appear on the 'Internet' list and be accessible by other players if you did it correctly.


