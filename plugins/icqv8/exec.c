/*
 * exec.c - Execute text-mode commands
 */

#ifdef __OS2__
 #pragma strings(readonly)
#endif

 #include <string.h>
 #include <stdlib.h>
 #include <time.h>

 #include "icqv8.h"

/*---[ Prototipes ]-------------------------------------------------------------------------------------------*/

/*
#ifdef DEBUG

   static char dbgPacket[] =
   {  0x37, 0x8f, 0xf9, 0x00, 0x9d, 0x7e, 0x00, 0x00, 0x00, 0x02, 0x08, 0x32, 0x36, 0x30, 0x37, 0x35,
      0x34, 0x32, 0x39, 0x00, 0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x02, 0x00, 0x50, 0x00, 0x06, 0x00,
      0x04, 0x20, 0x02, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x04, 0x00, 0x00, 0x3d, 0x75, 0x00, 0x03, 0x00,
      0x04, 0x3d, 0xe2, 0x0d, 0xcb, 0x00, 0x05, 0x01, 0x1c, 0x00, 0x00, 0x37, 0x8f, 0xf9, 0x00, 0x9d,
      0x7e, 0x00, 0x00, 0x09, 0x46, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53,
      0x54, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x0f, 0x00, 0x00, 0x27, 0x11, 0x00,
      0xf4, 0x1b, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0xcd, 0xff, 0x0e, 0x00,
      0xcd, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x00,
      0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x40, 0x00, 0x37, 0x1c, 0x58, 0x72, 0xe9, 0x87, 0xd4,
      0x11, 0xa4, 0xc1, 0x00, 0xd0, 0xb7, 0x59, 0xb1, 0xd9, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x53,
      0x65, 0x6e, 0x64, 0x20, 0x57, 0x65, 0x62, 0x20, 0x50, 0x61, 0x67, 0x65, 0x20, 0x41, 0x64, 0x64,
      0x72, 0x65, 0x73, 0x73, 0x20, 0x28, 0x55, 0x52, 0x4c, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x74, 0x00, 0x00,
      0x00, 0x4a, 0x61, 0x76, 0x61, 0x20, 0x54, 0x69, 0x70, 0x20, 0x34, 0x38, 0x3a, 0x20, 0x48, 0x6f,
      0x77, 0x20, 0x74, 0x6f, 0x20, 0x63, 0x72, 0x65, 0x61, 0x74, 0x65, 0x20, 0x61, 0x20, 0x72, 0x65,
      0x75, 0x73, 0x61, 0x62, 0x6c, 0x65, 0x20, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x42, 0x6f,
      0x78, 0x20, 0x63, 0x6c, 0x61, 0x73, 0x73, 0xfe, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77,
      0x77, 0x77, 0x2e, 0x6a, 0x61, 0x76, 0x61, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x2e, 0x63, 0x6f, 0x6d,
      0x2f, 0x6a, 0x61, 0x76, 0x61, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x2f, 0x6a, 0x61, 0x76, 0x61, 0x74,
      0x69, 0x70, 0x73, 0x2f, 0x6a, 0x77, 0x2d, 0x6a, 0x61, 0x76, 0x61, 0x74, 0x69, 0x70, 0x34, 0x38,
      0x2e, 0x68, 0x74, 0x6d, 0x6c
   };

   static char debugResponse[] =
   {  0x12, 0x00, 0xa4, 0x01, 0x0a, 0x29, 0x00, 0xd8, 0xdc, 0x09, 0x07, 0x08, 0x00, 0x44, 0x61, 0x72,
      0x6b, 0x4d, 0x61, 0x6e, 0x00, 0x06, 0x00, 0x50, 0x65, 0x72, 0x72, 0x79, 0x00, 0x08, 0x00, 0x57,
      0x65, 0x72, 0x6e, 0x65, 0x63, 0x6b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x33, 0x27, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00
   };

   static char debugOfflineMSG[] =
   { 0x00, 0x01, 0x00, 0x57, 0x55, 0x00, 0x12, 0xab, 0x9f, 0x01, 0x41, 0x00, 0x02, 0x00, 0x86, 0x98,
     0x00, 0x0a, 0xd2, 0x07, 0x0c, 0x06, 0x0d, 0x32, 0x01, 0x00, 0x3f, 0x00, 0x49, 0x20, 0x74, 0x72,
     0x69, 0x65, 0x64, 0x20, 0x74, 0x6f, 0x20, 0x67, 0x65, 0x74, 0x20, 0x61, 0x75, 0x74, 0x68, 0x20,
     0x66, 0x6f, 0x72, 0x20, 0x6e, 0x65, 0x77, 0x20, 0x70, 0x77, 0x69, 0x63, 0x71, 0x2c, 0x20, 0x62,
     0x75, 0x74, 0x20, 0x72, 0x65, 0x6a, 0x65, 0x63, 0x74, 0x65, 0x64, 0x20, 0x61, 0x73, 0x20, 0x73,
     0x70, 0x61, 0x6d, 0x6d, 0x65, 0x72, 0x2e, 0x20, 0x3b, 0x29, 0x00
   };



   static char debugMSG1[] =
   {
     0x00, 0x04, 0x00, 0x07, 0x00, 0x00, 0x97, 0xf5, 0xd5, 0x52, 0x6b, 0xd3, 0xea, 0x51, 0x88, 0x5e,
     0x4a, 0xdc, 0x02, 0x00, 0x09, 0x31, 0x33, 0x31, 0x37, 0x34, 0x38, 0x33, 0x33, 0x30, 0x00, 0xfe,
     0x00, 0xff, 0x00, 0x01, 0x00, 0x02, 0x00, 0x50, 0x00, 0x06, 0x00, 0x04, 0x20, 0x02, 0x00, 0x00,
     0x00, 0x0f, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x03, 0x00, 0x04, 0x3e, 0x15, 0x80, 0x65,
     0x00, 0x05, 0x01, 0x54, 0x00, 0x00, 0x51, 0xea, 0xd3, 0x6b, 0x5e, 0x88, 0x4a, 0xdc, 0x09, 0x46,
     0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00, 0x00, 0x0a,
     0x00, 0x02, 0x00, 0x01, 0x00, 0x0f, 0x00, 0x00, 0x27, 0x11, 0x01, 0x2c, 0x1b, 0x00, 0x08, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x0e, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0xc5,
     0x00, 0x7b, 0x5c, 0x72, 0x74, 0x66, 0x31, 0x5c, 0x61, 0x6e, 0x73, 0x69, 0x5c, 0x64, 0x65, 0x66,
     0x30, 0x0d, 0x0a, 0x7b, 0x5c, 0x66, 0x6f, 0x6e, 0x74, 0x74, 0x62, 0x6c, 0x7b, 0x5c, 0x66, 0x30,
     0x5c, 0x66, 0x73, 0x77, 0x69, 0x73, 0x73, 0x20, 0x4d, 0x53, 0x20, 0x53, 0x61, 0x6e, 0x73, 0x20,
     0x53, 0x65, 0x72, 0x69, 0x66, 0x3b, 0x7d, 0x7d, 0x0d, 0x0a, 0x7b, 0x5c, 0x63, 0x6f, 0x6c, 0x6f,
     0x72, 0x74, 0x62, 0x6c, 0x20, 0x3b, 0x5c, 0x72, 0x65, 0x64, 0x30, 0x5c, 0x67, 0x72, 0x65, 0x65,
     0x6e, 0x30, 0x5c, 0x62, 0x6c, 0x75, 0x65, 0x30, 0x3b, 0x7d, 0x0d, 0x0a, 0x5c, 0x76, 0x69, 0x65,
     0x77, 0x6b, 0x69, 0x6e, 0x64, 0x34, 0x5c, 0x70, 0x61, 0x72, 0x64, 0x5c, 0x63, 0x66, 0x31, 0x5c,
     0x66, 0x30, 0x20, 0x62, 0x6f, 0x6c, 0x69, 0x6e, 0x68, 0x6f, 0x20, 0x61, 0x73, 0x73, 0x61, 0x73,
     0x73, 0x69, 0x6e, 0x6f, 0x21, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x31,
     0x2e, 0x66, 0x6f, 0x6c, 0x68, 0x61, 0x2e, 0x75, 0x6f, 0x6c, 0x2e, 0x63, 0x6f, 0x6d, 0x2e, 0x62,
     0x72, 0x2f, 0x66, 0x6f, 0x6c, 0x68, 0x61, 0x2f, 0x72, 0x65, 0x75, 0x74, 0x65, 0x72, 0x73, 0x2f,
     0x75, 0x6c, 0x74, 0x31, 0x31, 0x32, 0x75, 0x32, 0x36, 0x35, 0x33, 0x39, 0x2e, 0x73, 0x68, 0x74,
     0x6d, 0x6c, 0x7d, 0x0d, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x26, 0x00,
     0x00, 0x00, 0x7b, 0x39, 0x37, 0x42, 0x31, 0x32, 0x37, 0x35, 0x31, 0x2d, 0x32, 0x34, 0x33, 0x43,
     0x2d, 0x34, 0x33, 0x33, 0x34, 0x2d, 0x41, 0x44, 0x32, 0x32, 0x2d, 0x44, 0x36, 0x41, 0x42, 0x46,
     0x37, 0x33, 0x46, 0x31, 0x34, 0x39, 0x32, 0x7d
   };

   static char debug2711[] =
   {
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x29, 0x00, 0xf0, 0x2d, 0x12, 0xd9, 0x30, 0x91, 0xd3, 0x11,
     0x8d, 0xd7, 0x00, 0x10, 0x4b, 0x06, 0x46, 0x2e, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x46, 0x69,
     0x6c, 0x65, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x1f, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x65, 0x00, 0x00,
     0x00, 0x00, 0x08, 0x00, 0x61, 0x61, 0x61, 0x2e, 0x6a, 0x70, 0x67, 0x00, 0xd2, 0x70, 0x03, 0x00,
     0x00, 0x00, 0x00, 0x00
   };


#endif

*/

/*---[ Implement ]--------------------------------------------------------------------------------------------*/

 int EXPENTRY icqv8_Interpret(HICQ icq, const char *cmd, const char *parm)
 {

#ifdef DEBUG
//    ICQV8  *cfg = icqGetPluginDataBlock(icq, module);

    if(!strcmp(cmd,"v8dbg"))
    {
//       DBGMessage(parm);
//       DBGTrace(sizeof(dbgPacket));
//      icqv8_snac04_table[2].exec(icq,cfg,sizeof(dbgPacket),0,dbgPacket);
//       icqv8_sendSystemMessage(icq, atoi(parm), 1, 0x0200, "", 0);
//       icqv8_searchByMail(icq, parm, 0, 0);
//       icqv8_searchByInformation(icq, 0, "R�diger", "Kuhlmann", "", NULL, 0, 0);
//       icqv8_searchByInformation(icq, 0, NULL, NULL, parm, NULL, 0, 0);
//       icqv8_searchRandom(icq, atoi(parm), 0, 0);
//       debug_SearchResponse(icq, cfg, sizeof(debugResponse), TRUE, debugResponse);
//       icqv8_processChannel2(icq, cfg, sizeof(debugMSG1), (SNAC *) debugMSG1);


//       debug_offlineMessage(icq, cfg, sizeof(debugOfflineMSG), debugOfflineMSG);
//       icqv8_srvRecGreet(icq, 27241234, sizeof(debug2711), (struct tlv2711 *) debug2711);

       return 0;
    }
#endif


    return -1;
 }


