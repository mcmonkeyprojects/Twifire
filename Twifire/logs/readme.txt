This folder will contain all your twifire server logs.
Be careful, they get pretty big after a while.
It's best to occasionally rename the current log file to the date
so you don't get your entire server history in one file.
No example was included, the logs will create themselves if your server is running.


To read logs, I recommend using Notepad++ http://notepad-plus-plus.org/
or any specially made log viewer that can interpret \n single-character linebreaks

Files:
mainlog.log - all logged data goes into this file
mainlog_errors.log - if Twifire detects an error, it will be mentioned here
.log - this is an error produced by attempting to log data before the cvars are loading. It'll be full of nothing but game startup date and a little startup message.