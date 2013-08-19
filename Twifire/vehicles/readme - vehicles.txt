Vehicles are literal vehicles - things you drive or fly.
In game, simply /amvehicle <player> <vehicle file name>
and the player becomes a vehicle based on the file's data

Vehicle files are stored as vehicles/vehicle_VEHICLENAMEHERE.cfg

To edit Vehicle files manually, I recommend notepad++ http://notepad-plus-plus.org/

On each line of the vehicle file you can specify any datapoint in the form name:value
(Like 'weapon:blaster')
If you specify the same name multiple times, the last one will be the one used
The following names are available:
model, weapon, speed, movetype, xmin, ymin, zmin, xmax, ymax, zmax, zadjust, pitch