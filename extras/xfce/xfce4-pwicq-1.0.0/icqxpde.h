/*  icqxpde.h - pwICQ Panel Plugin for XFCE
 *
 *  Copyright (C) 2003 Perry Werneck (pwicq_devel@terra.com.br)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

 #include "icqdefs.h"
 #include "icqqueue.h"
 
 #pragma pack(1)
 
 typedef struct 
 {
    GtkWidget     *button;
    GtkWidget     *entry;
    GdkPixbuf     *icons[PWICQICONBARSIZE];
    guint         iconSize;
    guint         timeout_id;
    unsigned long uin;
	ICQSHAREMEM   *icq;
	unsigned char icon;
	unsigned char event;
 } pwICQPlugin;

 #pragma pack()

 
