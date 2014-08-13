/*
 * MAIN.C - pwICQ's Skin manager entry points
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <pwicqgui.h>

/*---[ Table descriptions ]--------------------------------------------------------------------------------*/

  HMODULE                     module                   = 0;

  const struct icqButtonTable icqgui_MainButtonTable[] =
  {
     { 0, ID_SEARCH,     11,  3, 53,  -6, 0xFFFF, "SearchButton", (BUTTON_CALLBACK) icqskin_searchButton, "Search/Add", "8.Helv"  },
     { 0, ID_SYSMSG,     10,  3, 28,  -6, 0xFFFF, "SysMsgButton", (BUTTON_CALLBACK) icqskin_sysmsgButton, "System",     "8.Helv"  },
     { 1, ID_CONFIG, 0xFFFF,  3,  3,  60, 22,     "SysButton",    (BUTTON_CALLBACK) icqskin_configButton, "pwICQ",      "8.Helv"  },
     { 2, ID_MODE,        7, -3,  3, -68, 22,     "ModeButton",   (BUTTON_CALLBACK) icqskin_modeButton,   "Offline",    "8.Helv"  }
  };

  const int icqgui_MainButtonTableElements = sizeof(icqgui_MainButtonTable)/sizeof(struct icqButtonTable);

  /* Buttons in message user info */
  const ICQUSERBUTTON icqgui_msgUserBoxButtonTable[] =
  {
	{ MSGID_INFOBUTTON,  ICQICON_USERCONFIG },
	{ MSGID_ABOUTBUTTON, ICQICON_ABOUT      }
  };

  const int icqgui_msgUserBoxButtonTableElements = sizeof(icqgui_msgUserBoxButtonTable)/sizeof(ICQUSERBUTTON);

  /* Buttons in configuration user info */
  const ICQUSERBUTTON icqgui_cfgUserBoxButtonTable[] =
  {
	{ MSGID_MESSAGE,     ICQICON_UNREADMESSAGE },
	{ MSGID_ABOUTBUTTON, ICQICON_ABOUT         }
  };

  const int icqgui_cfgUserBoxButtonTableElements = sizeof(icqgui_cfgUserBoxButtonTable)/sizeof(ICQUSERBUTTON);

