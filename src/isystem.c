#include <stdio.h>
#include <stdlib.h>
#include "isystem.h"
#include "messages.h"
#include "isystem_event_handler.h"
#include "utils/memmap.h"
#include <json-glib/json-glib.h>

G_DEFINE_TYPE(IpcamISystem, ipcam_isystem, IPCAM_BASE_APP_TYPE);

#define DEFAULT_MD_LEN 128

static void ipcam_isystem_before_impl(IpcamISystem *isystem);
static void ipcam_isystem_in_loop_impl(IpcamISystem *isystem);
static void ipcam_isystem_status_led_proc(GObject *obj);

static void ipcam_isystem_init(IpcamISystem *self)
{
}
static void ipcam_isystem_class_init(IpcamISystemClass *klass)
{
    IpcamBaseServiceClass *base_service_class = IPCAM_BASE_SERVICE_CLASS(klass);
    base_service_class->before = ipcam_isystem_before_impl;
    base_service_class->in_loop = ipcam_isystem_in_loop_impl;
}
static void ipcam_isystem_before_impl(IpcamISystem *isystem)
{
	ipcam_base_app_register_notice_handler(IPCAM_BASE_APP(isystem), "set_datetime", IPCAM_ISYSTEM_EVENT_HANDLER_TYPE);
	ipcam_base_app_register_notice_handler(IPCAM_BASE_APP(isystem), "set_network", IPCAM_ISYSTEM_EVENT_HANDLER_TYPE);
    ipcam_base_app_add_timer(IPCAM_BASE_APP(isystem), "status_led_proc", "1", ipcam_isystem_status_led_proc);
}
static void ipcam_isystem_in_loop_impl(IpcamISystem *isystem)
{
}

static void ipcam_isystem_status_led_proc(GObject *obj)
{
    g_return_if_fail(IPCAM_IS_ISYSTEM(obj));
    static guint32 value = 0x0;
    void *pMem  = NULL;
    pMem = memmap(0x201C0020, DEFAULT_MD_LEN);
    if (value == 0x8)
    {
        value = 0x0;
    }
    else
    {
        value = 0x8;
    }
    *(guint32*)pMem = value;
}

void ipcam_isystem_update_network_setting(IpcamISystem *isystem, JsonNode *body)
{
    JsonObject *items_obj = json_object_get_object_member(json_node_get_object(body), "items");
    const gchar *lighttpd_script = ipcam_base_app_get_config(IPCAM_BASE_APP(isystem), "scripts:lighttpd");

    if (json_object_has_member(items_obj, "port"))
    {
        JsonObject *port_obj = json_object_get_object_member(items_obj, "port");
        if (NULL != lighttpd_script &&
            json_object_has_member(port_obj, "http"))
        {
            FILE *fp;

            fp = popen(lighttpd_script, "w");
            if (fp == NULL)
            {
                perror("error restart lighttpd!");
            }
            else
            {
                pclose(fp);
            }
        }
	}
}

void ipcam_isystem_update_datetime_setting(IpcamISystem *isystem, JsonNode *body)
{
    JsonObject *items_obj = json_object_get_object_member(json_node_get_object(body), "items");
    const gchar *time_script = ipcam_base_app_get_config(IPCAM_BASE_APP(isystem), "scripts:timezone");

    if (NULL != time_script &&
        json_object_has_member(items_obj, "timezone"))
    {
		FILE *fp;

        fp = popen(time_script, "w");
        if (fp == NULL)
        {
            perror("error set timezone!");
        }
        else
        {
            pclose(fp);
        }
	}
}
