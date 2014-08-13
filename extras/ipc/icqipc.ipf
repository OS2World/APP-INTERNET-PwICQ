:userdoc.
:title.pwICQ Version 2 IPC Documentation

:h1 res=010.pwICQ Version 2 IPC
:p.
The ICQIPC.DLL is designed to supply a full set of IPC capabilities for pwICQ 2.0, including the REXX scripting support. 
Most of the supplied function are similar in C-Language and Rexx calls.

.*---[ Rexx API ]-------------------------------------------------------------------------

:h2 res=1000.Rexx API
:p.
To use the pwICQ Rexx API you must load ICQIPC.DLL by the following code:
:p.
:xmp.
rc = RxFuncAdd("rxICQLoadFuncs","icqipc","rxICQLoadFuncs")
rc = rxICQLoadFuncs()
:exmp.
:p.
To unload the DLL, you should first call the rxICQDropFuncs() function, then exit all CMD.EXE shells. 
After exiting all the command shells, the DLL will be unloaded by OS/2 and can be deleted or replaced. 
:p.

:h3 res=1001.rxICQQueryBuild
:p.
This call returns the current icqipc.dll build number.
:p.
Example:
:xmp.
say rxICQQueryBuild()
:exmp.

:h3 res=1002.rxICQSetMode
:p.
This call sets the online mode of the requested pwICQ instance (0 means the first one).
:p.
Example:
:xmp.
say rxICQSetMode(0,"Away")
:exmp.


:h3 res=1003.rxICQQueryUIN
Gets the ICQ# of the first pwICQ instance
:p.

:p.
Example:
:xmp.
say rxICQQueryUIN()
:exmp.


:h3 res=1004.rxICQExecute
:p.
This call pass one string to the internal pwICQ command processor; the first parameters is the pwICQ instance (can be 0) and the
second is the string to be parsed; usually a command name folowed by parameters.
:p.
The available command and parameters can change depending of the installed plugins.
:p.
Example:
:xmp.
say rxICQExecute(0,"mode DND")
:exmp.


:h3 res=1005.rxICQOpenFirstMessage
:p.
Open the first pending message on the requested instance
:p.
Example:
:xmp.
say rxICQOpenFirstMessage(0)
:exmp.


:h3 res=1006.rxICQShowPopupMenu
:p.
Ask for the GUI manager plugin to show the requested popup-menu.
:p.
Examples:
:xmp.
say rxICQShowPopupMenu(0,101)
say rxICQShowPopupMenu(0,102)
:exmp.


:h3 res=1007.rxICQShowUserList
:p.
Ask for the GUI manager plugin to show the popup user's list.
:p.


:h3 res=1008.rxICQPluginRequest
:p.
Send a remote request for the selected plugin; the format of the request can change depending of the plugin.
:p.


:h3 res=1009.rxICQQueryOnlineMode
:p.
Return the current online mode for the selected pwICQ instance.
:p.
Example:
:xmp.
say rxICQQueryOnlineMode(0)
:exmp.



