/*  pwicq.c - pwICQ Panel Plugin for XFCE
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

#ifdef HAVE_CONFIG_H
   #include <config.h>
#endif

#include <time.h>

#include <panel/global.h>
#include <panel/controls.h>
#include <panel/icons.h>
#include <panel/plugins.h>
#include <libxfcegui4/xfce_iconbutton.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include "icqxpde.h"

#ifdef DEBUG

static void DBGPRINT(const char *fmt, ...)
{
   va_list arg_ptr;
   FILE    *out             = fopen("/tmp/" PROJECT ".dbg", "a");

   if(out)
   {
      va_start(arg_ptr, fmt);
      vfprintf(out, fmt, arg_ptr);
      va_end(arg_ptr);
	  fprintf(out,"\n");
	  fclose(out);
   }

 }

 #define DBGMessage(x) DBGPRINT(__FILE__ "(%d)\t%s",       __LINE__, x);
 #define DBGString(x)  DBGPRINT(__FILE__ "(%d)\t%s=%s",    __LINE__, #x, x);
 #define DBGTracex(x)  DBGPRINT(__FILE__ "(%d)\t%s=%08lx", __LINE__, #x, (unsigned long) x);
 #define DBGTrace(x)   DBGPRINT(__FILE__ "(%d)\t%s=%08ld", __LINE__, #x, (unsigned long) x);
 #define CHKPoint()    DBGPRINT(__FILE__ "(%d)\t%s",       __LINE__, __FUNCTION__ );
 
#else
 
 #define DBGMessage(x) /* */
 #define DBGString(x)  /* */
 #define DBGTracex(x)  /* */
 #define DBGTrace(x)   /* */
 #define CHKPoint()    /* */
 
#endif

/*---[ IPC Calls with pwICQ ]-------------------------------------------------*/

static ICQSHAREMEM * IPCGetSharedBlock(void)
{
   key_t                  memKey       =  ftok(PWICQ_SHAREMEM,0x230167);
   int            shmID;
   ICQSHAREMEM    *icq         =  NULL;

   shmID = shmget(memKey,sizeof(sizeof(ICQSHAREMEM)),0660);

   if( ((int) shmID) == -1)
      return NULL;

   icq = shmat(shmID,0,0);

   if( ((int) icq) == -1)
      return NULL;

   if(icq->sz != sizeof(ICQSHAREMEM) || icq->si != sizeof(ICQINSTANCE))
   {
      shmdt(icq);
      return NULL;
   }

   return icq;
}

static int IPCReleaseSharedBlock(ICQSHAREMEM *icq)
{
   if(!icq)
      return -1;

   shmdt(icq);

   return 0;
}

static ICQINSTANCE * IPCGetInstance(pwICQPlugin *cfg)
{
   int         f;
   ICQINSTANCE *ret;
   
   if(!(cfg && cfg->uin))
	  return NULL;

  if(!cfg->icq)
	  cfg->icq = IPCGetSharedBlock();
   
   if(cfg->icq && cfg->uin)
   {
	  ret = cfg->icq->i;
	  
	  for(f=0;f<PWICQ_MAX_INSTANCES;f++)
	  {
		 if(ret->uin == cfg->uin)
			return ret;
	  }
   }
   
   return NULL;
}

static int IPCSendCommand(ULONG uin, ICQQUEUEBLOCK *cmd)
{
   /* Envia um comando */
    
   ICQQUE_PACKET	 packet;
   ICQQUEUEBLOCK  *req;
   int			    rc		= 0;
   key_t			 key;
   int			    queue;
   int				 f;

   if(cmd->szPrefix != sizeof(ICQQUEUEBLOCK) || cmd->sz < sizeof(ICQQUEUEBLOCK))
      return -2;

   memset(&packet,0,sizeof(ICQQUE_PACKET));
   packet.uin    = uin;
   packet.msg.sz = sizeof(ICQQUE_MESSAGE);

   if( (packet.msg.shmID = shmget(IPC_PRIVATE,cmd->sz,IPC_CREAT | 0660)) == -1)
      return -3;

   req = shmat(packet.msg.shmID,0,0);
   if( ((int) req) == -1)
   {
      return -4;
   }
   else
   {
      memcpy(req,cmd,cmd->sz);

      key = ftok("/tmp/pwicq.queue",0x230167); 	    
       
      if( ((int) key) == -2)
      {
         rc = -5;
      }
      else
      {
         queue = msgget(key,0660);
          
         if(queue == -1)
         {
            rc = -6;
         }
         else
         {
            if(msgsnd(queue, (struct msgbuf *) &packet, sizeof(ICQQUE_PACKET), 0) == -1)
            {
               rc = -7;
            }
            else
            {
               for(f=0;f<100 && req->status == cmd->status;f++)
                  usleep(10000);
               DBGTrace(req->status);
               DBGTrace(cmd->status);
 	            memcpy(cmd,req,cmd->sz);
            }
         }
      }
 	  shmdt(req);
   }
   return rc;
}

static int IPCSendCMD(ULONG uin, UCHAR cmd)
{
   ICQQUEUEBLOCK       blk;

   memset(&blk,0,sizeof(ICQQUEUEBLOCK));

   blk.szPrefix        =                               // Size of ICQQUEUEBLOCK (Version check)
   blk.sz              = sizeof(ICQQUEUEBLOCK);        // Size of datablock
   blk.cmd             = cmd;                          // Command type
   blk.rc              = 0;                            // Return code
   blk.status          = 0;                            // Status code

   return IPCSendCommand(uin, &blk);

}

/*---[ GUI ]------------------------------------------------------------------*/
 
static gboolean pwicq_update(gpointer data)
{
    pwICQPlugin   *cfg   = (pwICQPlugin *) data;
    ICQINSTANCE   *icq;
    unsigned char icon;

    if(!cfg)
	   return FALSE;

	cfg->event = !cfg->event;
	icon       = cfg->icon;
	icq        = IPCGetInstance(cfg);
	
	if(icq)
	{
	   if(icq->modeIcon == icq->eventIcon || icq->eventIcon >= PWICQICONBARSIZE)
	   {
	      icon = icq->modeIcon;   
	   }
	   else
	   {
		  icon = cfg->event ? icq->modeIcon : icq->eventIcon;
	   }
	}
	else
	{
       icon = ICQICON_INACTIVE;
	}
	
	if(icon != cfg->icon)
	{
	   DBGMessage("Icon has changed");
       DBGTrace(icon);
	   
	   cfg->icon = icon;
       xfce_iconbutton_set_pixbuf(XFCE_ICONBUTTON(cfg->button),cfg->icons[icon]);
	}

    return TRUE;
}

static void clicked(GtkWidget *widget, pwICQPlugin *cfg)
{
   ICQINSTANCE   *icq = IPCGetInstance(cfg);

   CHKPoint();
   
   if(cfg->uin && icq)
      IPCSendCMD(cfg->uin,(icq->eventIcon != 0xFF) ? ICQCMD_OPENFIRST : ICQCMD_POPUPUSERLIST);

}

static pwICQPlugin *pwicq_new(void)
{
    GdkPixbuf   *icons;
    int         f,c;
    pwICQPlugin *cfg     = g_new(pwICQPlugin, 1);

    memset(cfg,0,sizeof(pwICQPlugin));
   
    icons         = gdk_pixbuf_new_from_file("/usr/share/pwicq/images/icons.gif",NULL);
   
    DBGTracex(icons);
   
    if(icons)
	{   
       cfg->iconSize = gdk_pixbuf_get_height(icons);
	   DBGTrace(cfg->iconSize);
    
       for(f=c=0;f<PWICQICONBARSIZE;f++)
       {
          cfg->icons[f] = gdk_pixbuf_new( gdk_pixbuf_get_colorspace(icons),
                                          gdk_pixbuf_get_has_alpha(icons),
                                          gdk_pixbuf_get_bits_per_sample(icons),
                                          cfg->iconSize, cfg->iconSize
	   								 );

          gdk_pixbuf_copy_area (icons,c,0,cfg->iconSize,cfg->iconSize,cfg->icons[f],0,0);

          c += cfg->iconSize;
       }
   
       gdk_pixbuf_unref(icons);
	   
    }
   
	cfg->icq    = NULL;
    cfg->button = xfce_iconbutton_new_from_pixbuf(cfg->icons[cfg->icon = ICQICON_INACTIVE]);
	
    gtk_button_set_relief(GTK_BUTTON (cfg->button), GTK_RELIEF_NONE);

    cfg->timeout_id = g_timeout_add(1000, pwicq_update, cfg);
	DBGTrace(cfg->timeout_id);

	gtk_widget_show_all(cfg->button);
	
	gtk_signal_connect(GTK_OBJECT(cfg->button), "clicked", GTK_SIGNAL_FUNC(clicked),cfg);

    return cfg;
}

static void pwicq_free(Control *control)
{
    pwICQPlugin *cfg;
    int         f;

    CHKPoint();
   
    g_return_if_fail(control != NULL);

    cfg = control->data;
    g_return_if_fail(cfg != NULL);

    if (cfg->timeout_id)
	   g_source_remove(cfg->timeout_id);

    for(f=0;f<PWICQICONBARSIZE;f++)
	{
	   if(cfg->icons[f]);
	      gdk_pixbuf_unref(cfg->icons[f]);
	}

	DBGTracex(cfg->icq);
	IPCReleaseSharedBlock(cfg->icq);
	cfg->icq = NULL;
	
    g_free(cfg);
}

extern xmlDocPtr xmlconfig;

static void pwicq_read_config(Control *control, xmlNodePtr node)
{
    pwICQPlugin *cfg;
    xmlChar     *value;

    g_return_if_fail(control != NULL);
    g_return_if_fail(node != NULL);

    cfg = (pwICQPlugin *)control->data;

    node = node->children;
   
    if (node == NULL || !xmlStrEqual(node->name, (const xmlChar *) "pwICQ"))
	{
	   CHKPoint();
	   return;
	}

    node = node->children;
	
    while (node != NULL) 
	{
	   if (xmlStrEqual(node->name, (const xmlChar *) "UIN")) 
	   {
	      value = xmlNodeListGetString(xmlconfig, node->children, 1);
	      if (value) 
		  {
			 cfg->uin = atol(value);
			 DBGTrace(cfg->uin);
		     xmlFree(value);
		  }
	   }
	   node = node->next;
    }
}

static void pwicq_write_config(Control *control, xmlNodePtr parent)
{
    pwICQPlugin *cfg;
    xmlNodePtr  node;
    char        buffer[0x0100];

    g_return_if_fail (control != NULL);
    g_return_if_fail (parent != NULL);
   
    cfg = (pwICQPlugin *)control->data;
    g_return_if_fail(cfg != NULL);

    node = xmlNewTextChild(parent, NULL, (const xmlChar *)"pwICQ", NULL);
   
    sprintf(buffer,"%ld",cfg->uin);
    DBGMessage(buffer);
   
    xmlNewTextChild(node, NULL, (const xmlChar *) "UIN", buffer);
 
}

static void pwicq_attach_callback(Control *control, const char *signal, GCallback callback, gpointer data)
{
   pwICQPlugin *cfg = (pwICQPlugin *)control->data;
   g_signal_connect(cfg->button, signal, callback, data);
}

static void pwicq_apply_options_cb(GtkWidget *widget, pwICQPlugin *cfg)
{
    const gchar *uin = gtk_entry_get_text(GTK_ENTRY(cfg->entry));
   
    DBGTracex(uin);
   
    if(uin)
	{
	   DBGMessage(uin);
	   cfg->uin = atoi(uin);
	}
   
}

static void pwicq_create_options(Control *control, GtkContainer *container, GtkWidget *done)
{
    pwICQPlugin *cfg;
    GtkWidget   *table;
    GtkWidget   *label;
    char        buffer[0x0100];
   
    CHKPoint();
   
    g_return_if_fail(control != NULL);
    g_return_if_fail(container != NULL);
    g_return_if_fail(done != NULL);
   
    cfg = (pwICQPlugin *) control->data;
    g_return_if_fail(cfg != NULL);
   
    table = gtk_table_new(1, 2, FALSE);

    label = gtk_label_new("ICQ#:");
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
   
    cfg->entry = gtk_entry_new();
	sprintf(buffer,"%08ld",cfg->uin);
    gtk_entry_set_text(GTK_ENTRY(cfg->entry), buffer);

    gtk_table_attach_defaults(GTK_TABLE(table),cfg->entry, 1, 2, 0, 1);

    g_signal_connect(G_OBJECT(done), "clicked", G_CALLBACK(pwicq_apply_options_cb), cfg);

    gtk_widget_show_all(table);
    gtk_container_add(container, table);

}


static void pwicq_set_size(Control *control, int size)
{
    CHKPoint();
   
    g_return_if_fail(control != NULL);

    gtk_widget_set_size_request(control->base, -1, -1);
}

/*  
 * pwICQ panel control
 */
static gboolean create_pwicq_control(Control * control)
{
    pwICQPlugin *cfg = pwicq_new();

    DBGTracex(cfg);
   
    gtk_container_add(GTK_CONTAINER(control->base), cfg->button);

    control->data       = (gpointer) cfg;
    control->with_popup = FALSE;

    gtk_widget_set_size_request(control->base, -1, -1);

    return TRUE;
}

G_MODULE_EXPORT void xfce_control_class_init(ControlClass * cc)
{
    CHKPoint();
   
    cc->name    = "pwICQ";
   
#ifdef DEBUG
    cc->caption = "pwICQ Status [DEBUG]";
#else   
    cc->caption = "pwICQ Status";
#endif   

    cc->create_control = (CreateControlFunc) create_pwicq_control;

    cc->free            = pwicq_free;
    cc->read_config     = pwicq_read_config;
    cc->write_config    = pwicq_write_config;
    cc->attach_callback = pwicq_attach_callback;
    cc->create_options  = pwicq_create_options;
    cc->set_size        = pwicq_set_size;
}

/* macro defined in plugins.h */
XFCE_PLUGIN_CHECK_INIT

// vim: set ts=8 sw=4 :
