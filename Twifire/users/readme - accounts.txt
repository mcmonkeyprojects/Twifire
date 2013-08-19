The admin account is there for your first login and early experimenting. As soon as you manage to create new accounts, do so,
or at least change the admin account's password if you can.
Default admin login: /amlogin admin adminpassword
Note that the admin account has every command, and is in every group. Don't let it fall into the wrong hands.

To edit account files manually, I recommend Helpfiles/TwifireAccountEditor.exe (see relevant readme)
OR use notepad++ http://notepad-plus-plus.org/

structure of account file

password
adminrank (1 through 5 [or whatever the mc_max_admin_rank is set to])
last used in-game name
credits
powers1 <see helpfiles>
powers2
powers4
Groups <group1 through group6, for shields, sentries>
powers5
powers6
powers7
Last Login Time (if timer is running)
blank line


Remember, only edit this file if
1: You know what you're doing
2: You need to do it this way

In most situations, it's best to simply have the users login and use the commands /amjoingroup, /amsetrank, and /amgivecommand.
/amjoingroup <user> <group number 1-6>
/amsetrank <user> <admin rank, 1-mc_max_admin_rank>
/amgivecommand <user> <amcommandhere>


File Names:
Accounts are saved by the following file names
account_1_USERNAMEHERE.cfg   for the main account data and everything important
mail_1_USERNAMEHERE.cfg      for the user's private mail