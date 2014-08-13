/*
 * MSGDLG.H - pwICQ default windows control-IDs
 */

 #define ICQMSGC_WIDGETS       29
 #define ICQMSGC_URL           ICQMSGC_ENTRY
 #define ICQSHRC_BUTTONS	   4


 #ifdef linux

    #ifndef DLG_ELEMENT_ID
       #define DLG_ELEMENT_ID     0
    #endif

    #define ICQMSGC_STATICUIN     18
    #define ICQMSGC_UIN           19
    #define ICQMSGC_STATICEMAIL   20
    #define ICQMSGC_EMAIL         21
    #define ICQMSGC_TOFROM        22
    #define ICQMSGC_CHECKBOX      23
    #define ICQMSGC_TEXTBOX       24	
    #define ICQMSGC_REJECT        25
    #define ICQMSGC_STATICSUBJ    26
    #define ICQMSGC_SUBJECT       27
    #define ICQMSGC_OPEN          28

    #define ICQMSGS_DATE          ICQMSGC_WIDGETS
    #define ICQMSGS_AUTOOPEN      ICQMSGC_WIDGETS+1
    #define ICQMSGS_URGENT        ICQMSGC_WIDGETS+2
    #define ICQMSGS_AUTHORIZE     ICQMSGC_WIDGETS+3

    #define ICQMSGS_EMAIL         ICQMSGC_STATICEMAIL
    #define ICQMSGS_NAME          ICQMSGC_STATICNAME
    #define ICQMSGS_SUBJECT       ICQMSGC_STATICSUBJ
    #define ICQMSGS_OPEN          ICQMSGC_OPEN
    #define ICQMSGS_ADD           ICQMSGC_ADD

    /* Search window */
    #define ICQSHRC_CLOSE	 	  0
    #define ICQSHRC_ADD		 	  1
    #define ICQSHRC_SEARCH	 	  2
    #define ICQSHRC_ABOUT	      3	    	

    #define ICQSHRC_MESSAGE	      4000
    #define ICQSHRC_CONTAINER	  4001
    #define ICQSHRC_NOTEBOOK	  4002	    	

 #endif

 #ifdef __OS2__

    #ifndef DLG_ELEMENT_ID
       #define DLG_ELEMENT_ID     500
    #endif

    /* Message edit window (Not implemented in the skin manager V2) */
    #define ICQMSGC_STATICUIN    1001
    #define ICQMSGC_UIN          1002
    #define ICQMSGC_STATICEMAIL  1007
    #define ICQMSGC_EMAIL        1008
    #define ICQMSGC_TOFROM       1010
    #define ICQMSGC_CHECKBOX     1011
    #define ICQMSGC_TEXTBOX      1012
    #define ICQMSGC_REJECT       1014
    #define ICQMSGC_STATICSUBJ   1020
    #define ICQMSGC_SUBJECT      1021
    #define ICQMSGC_OPEN         1024

    #define ICQMSGS_AUTOOPEN     1100
    #define ICQMSGS_URGENT       1101
    #define ICQMSGS_DATE         1102
    #define ICQMSGS_AUTHORIZE    1103

    #define ICQMSGC_DOWN         2000
    #define ICQMSGC_UP           2001

    #define ICQMSGS_ADD          ICQMSGC_ADD
    #define ICQMSGS_EMAIL        ICQMSGC_STATICEMAIL
    #define ICQMSGS_NAME         ICQMSGC_STATICNAME
    #define ICQMSGS_SUBJECT      ICQMSGC_STATICSUBJ
    #define ICQMSGS_OPEN         ICQMSGC_OPEN

    #define ICQMSGM_SENDTO	 3000

    /* Search window */
    #define ICQSHRC_CLOSE	 ICQMSGC_CLOSE
    #define ICQSHRC_ADD		 ICQMSGC_ADD
    #define ICQSHRC_SEARCH	 1110
    #define ICQSHRC_ABOUT	 1111	    	
    #define ICQSHRC_MESSAGE	 4000

    #define ICQSHRC_CONTAINER	 4001
    #define ICQSHRC_NOTEBOOK	 4002	    	

 #endif

 /* Message edit window (Compatible with skin manager version 2) */
 #define ICQMSGC_CLOSE         DLG_ELEMENT_ID
 #define ICQMSGC_SEND          DLG_ELEMENT_ID+1
 #define ICQMSGC_ADD           DLG_ELEMENT_ID+2
 #define ICQMSGC_ABOUT         DLG_ELEMENT_ID+3
 #define ICQMSGC_INFO          DLG_ELEMENT_ID+4
 #define ICQMSGC_REPLY         DLG_ELEMENT_ID+5
 #define ICQMSGC_ACCEPT        DLG_ELEMENT_ID+6
 #define ICQMSGC_REFUSE        DLG_ELEMENT_ID+7
 #define ICQMSGC_BROWSE        DLG_ELEMENT_ID+8
 #define ICQMSGC_NEXT          DLG_ELEMENT_ID+9

 #define ICQMSGC_USERMODE      DLG_ELEMENT_ID+10

 #define ICQMSGC_STATICNICK    DLG_ELEMENT_ID+11
 #define ICQMSGC_NICKNAME      DLG_ELEMENT_ID+12

 #define ICQMSGC_STATICNAME    DLG_ELEMENT_ID+13
 #define ICQMSGC_NAME          DLG_ELEMENT_ID+14

 #define ICQMSGC_SENDTO        DLG_ELEMENT_ID+15

 #define ICQMSGC_TEXT          DLG_ELEMENT_ID+16

 #define ICQMSGC_ENTRY         DLG_ELEMENT_ID+17


