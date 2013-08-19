This folder contains all the mapedits for every map
That means all the models, effects, deletes, etc
This is mostly edited on it's own when you use ammap_save

Note that the mapedits files are all easy editable in plain notepad
but I still recommend notepad++ http://notepad-plus-plus.org/

The files all have the map name in them, and will be automatically run as scripts in every map
You can fill these files with servercommands like 'addip' or 'kick' or 'addbot' or any other server commands (listed in readme 'server commands')

also note, to delete ents that weren't edited in (in the original copy of the map)
add the command on an empty line:
mcmap_delent <entnumber>
and there are other ammap_* commands that work as mcmap_* serverside.

also, allmaps.cfg is a mapedit file that's executed first on every map. Including those with their own edits.
This can be used for reshader the console, or player skins, or whatever
allmaps2.cfg is the same, though it had a more complex intended purpose, it is now merely an easy way to separate your allmaps content into separate files.

example_ffa_bespin.cfg is the mapedits for ffa_bespin used at [HACKS] server
it is there so you can see how the mapedits work.
if you want to see it in action, rename it to mapedits_1_ffa_bespin.cfg and load ffa_bespin on your server.

the included mapedits_1_yavin_trial file deletes the yavin_trial map entities that cause crashes.

the included mapedits_1_yavin_temple file will spawn the elevators in the map yavin_temple.


mapedit file format:
mapedits_#_MAPNAMEHERE.cfg where # is the file number (starting with 1 and increasing whenever the last file hits 1000kb)

teleporters_1_MAPNAMEHERE.cfg contains all chat teleports