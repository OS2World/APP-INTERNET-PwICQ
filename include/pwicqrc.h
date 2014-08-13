/*
 * pwICQRC.H - pwICQ Resource file manager include
 */

 #pragma pack(1)

/*** Standard Window Classes ***********************************************/

 #define WC_FRAME              0xffff0001
 #define WC_COMBOBOX           0xffff0002
 #define WC_BUTTON             0xffff0003
 #define WC_MENU               0xffff0004
 #define WC_STATIC             0xffff0005
 #define WC_ENTRYFIELD         0xffff0006
 #define WC_LISTBOX            0xffff0007
 #define WC_SCROLLBAR          0xffff0008
 #define WC_TITLEBAR           0xffff0009
 #define WC_MLE                0xffff000A
 #define WC_APPSTAT            0xffff0010
 #define WC_KBDSTAT            0xffff0011
 #define WC_PECIC              0xffff0012
 #define WC_DBE_KKPOPUP        0xffff0013
 #define WC_SPINBUTTON         0xffff0020
 #define WC_CONTAINER          0xffff0025
 #define WC_SLIDER             0xffff0026
 #define WC_VALUESET           0xffff0027
 #define WC_NOTEBOOK           0xffff0028
 #define WC_PENFIRST           0xffff0029
 #define WC_PENLAST            0xffff002C
 #define WC_MMPMFIRST          0xffff0040
 #define WC_CIRCULARSLIDER     0xffff0041
 #define WC_MMPMLAST           0xffff004f
 #define WC_PRISTDDLGFIRST     0xffff0050
 #define WC_PRISTDDLGLAST      0xffff0057
 #define WC_PUBSTDDLGFIRST     0xffff0058
 #define WC_PUBSTDDLGLAST      0xffff005f

 #define ES_LEFT               0x00000000
 #define ES_CENTER             0x00000001
 #define ES_RIGHT              0x00000002

 #define ES_MARGIN             0x00000008
 #define ES_AUTOTAB            0x00000010
 #define ES_READONLY           0x00000020
 #define ES_COMMAND            0x00000040
 #define ES_UNREADABLE         0x00000080
 #define ES_AUTOSIZE           0x00000200
 #define ES_ANY                0x00000000
 #define ES_SBCS               0x00001000
 #define ES_DBCS               0x00002000
 #define ES_MIXED              0x00003000
 #define ES_AUTOSCROLL         0


 #define LS_MULTIPLESEL        0x00000001
 #define LS_OWNERDRAW          0x00000002
 #define LS_NOADJUSTPOS        0x00000004
 #define LS_HORZSCROLL         0x00000008
 #define LS_EXTENDEDSEL        0x00000010


 #define BS_PUSHBUTTON         0
 #define BS_CHECKBOX           1
 #define BS_AUTOCHECKBOX       2
 #define BS_RADIOBUTTON        3
 #define BS_AUTORADIOBUTTON    4
 #define BS_3STATE             5
 #define BS_AUTO3STATE         6
 #define BS_USERBUTTON         7
 #define BS_NOTEBOOKBUTTON     8
 #define BS_PRIMARYSTYLES      0x0000000f

 #define BS_TEXT               0x0010
 #define BS_MINIICON           0x0020
 #define BS_BITMAP             0x0040
 #define BS_ICON               0x0080
 #define BS_HELP               0x0100
 #define BS_SYSCOMMAND         0x0200
 #define BS_DEFAULT            0x0400
 #define BS_NOPOINTERFOCUS     0x0800
 #define BS_NOBORDER           0x1000
 #define BS_NOCURSORSELECT     0x2000
 #define BS_AUTOSIZE           0x4000

 #define SS_TEXT               0x0001
 #define SS_GROUPBOX           0x0002
 #define SS_ICON               0x0003
 #define SS_BITMAP             0x0004
 #define SS_FGNDRECT           0x0005
 #define SS_HALFTONERECT       0x0006
 #define SS_BKGNDRECT          0x0007
 #define SS_FGNDFRAME          0x0008
 #define SS_HALFTONEFRAME      0x0009
 #define SS_BKGNDFRAME         0x000a
 #define SS_SYSICON            0x000b
 #define SS_AUTOSIZE           0x0040


 #define DT_LEFT               0x00000000
 #define DT_QUERYEXTENT        0x00000002
 #define DT_UNDERSCORE         0x00000010
 #define DT_STRIKEOUT          0x00000020
 #define DT_TEXTATTRS          0x00000040
 #define DT_EXTERNALLEADING    0x00000080
 #define DT_CENTER             0x00000100
 #define DT_RIGHT              0x00000200
 #define DT_TOP                0x00000000
 #define DT_VCENTER            0x00000400
 #define DT_BOTTOM             0x00000800
 #define DT_HALFTONE           0x00001000
 #define DT_MNEMONIC           0x00002000
 #define DT_WORDBREAK          0x00004000
 #define DT_ERASERECT          0x00008000

 #define WS_OVERLAPPED        0x00000000L
 #define WS_POPUP             0x80000000L
 #define WS_CHILD             0x40000000L
 #define WS_MINIMIZE          0x20000000L
 #define WS_VISIBLE           0x10000000L
 #define WS_DISABLED          0x08000000L
 #define WS_CLIPSIBLINGS      0x04000000L
 #define WS_CLIPCHILDREN      0x02000000L
 #define WS_MAXIMIZE          0x01000000L
 #define WS_BORDER            0x00800000L
 #define WS_DLGFRAME          0x00400000L
 #define WS_VSCROLL           0x00200000L
 #define WS_HSCROLL           0x00100000L
 #define WS_SYSMENU           0x00080000L
 #define WS_THICKFRAME        0x00040000L
 #define WS_GROUP             0x00020000L
 #define WS_TABSTOP           0x00010000L

 #define WS_MINIMIZEBOX       0x00020000L
 #define WS_MAXIMIZEBOX       0x00010000L

 #define WS_CHILDWINDOW       WS_CHILD
 #define WS_ICONIC            WS_MINIMIZE
 #define WS_SIZEBOX           WS_THICKFRAME
 #define WS_TILED             WS_OVERLAPPED
 #define WS_TILEDWINDOW       WS_OVERLAPPEDWINDOW

 #define MLS_WORDWRAP          0x00000001L
 #define MLS_BORDER            0x00000002L
 #define MLS_VSCROLL           0x00000004L
 #define MLS_HSCROLL           0x00000008L
 #define MLS_READONLY          0x00000010L
 #define MLS_IGNORETAB         0x00000020L
 #define MLS_DISABLEUNDO       0x00000040L
 #define MLS_LIMITVSCROLL      0x00000080L

 #define CBS_SIMPLE            0x0001L
 #define CBS_DROPDOWN          0x0002L
 #define CBS_DROPDOWNLIST      0x0003L
 #define CBS_OWNERDRAWFIXED    0x0010L
 #define CBS_OWNERDRAWVARIABLE 0x0020L
 #define CBS_AUTOHSCROLL       0x0040L
 #define CBS_OEMCONVERT        0x0080L
 #define CBS_SORT              0x0100L
 #define CBS_HASSTRINGS        0x0200L
 #define CBS_NOINTEGRALHEIGHT  0x0400L
 #define CBS_DISABLENOSCROLL   0x0800L

 struct icqStringTable
 {
	unsigned short              id;
	const char                  *str;
 };

 struct icqDialogDescriptor
 {
    const char                  *title;
    unsigned short	            id;
    unsigned short	            x;
    unsigned short	            y;
    unsigned short	            cx;
    unsigned short	            cy;
	unsigned long               className;
	unsigned long               flags;
 };

 struct icqDialogTable
 {
	unsigned short                   id;
	unsigned short                   el;
	const struct icqDialogDescriptor *descriptor;
 };
	

 struct icqResourceDescriptor
 {
	unsigned short              sz;
	
	unsigned short              szStringDescr;
	unsigned short              stringCount;
	const struct icqStringTable *stringTable;
	
	unsigned short              szDialogTable;
	unsigned short              szDialogDescr;
	unsigned short              dialogCount;
	const struct icqDialogTable *dialogTable;
	
 };
	


 #pragma pack()

 const struct icqResourceDescriptor * icqresource_getDescriptor(void);
	

