#include <stdio.h>
#include <stdlib.h>
#include "isystem.h"
#include "messages.h"
#include "isystem_event_handler.h"
#include "utils/memmap.h"
#include <json-glib/json-glib.h>

typedef gint64 (*DELAYED_WORK_FUNC)(void *user_data);

typedef struct _ISystemDelayedWork
{
	gboolean enabled;
	gint64 end_time;
	DELAYED_WORK_FUNC func;
	gpointer user_data;
} ISystemDelayedWork;

typedef struct _IpcamISystemPrivate
{
	ISystemDelayedWork delayed_works[1];
} IpcamISystemPrivate;

#define NETWORK_DELAYED_WORK_ID	0

#define ARRAY_SIZE(x)	   (sizeof(x) / sizeof(x[0]))

G_DEFINE_TYPE_WITH_PRIVATE(IpcamISystem, ipcam_isystem, IPCAM_BASE_APP_TYPE);

#define DEFAULT_MD_LEN 128

static void ipcam_isystem_before_impl(IpcamISystem *isystem);
static void ipcam_isystem_in_loop_impl(IpcamISystem *isystem);
static void ipcam_isystem_status_led_proc(GObject *obj);

static void ipcam_isystem_init(IpcamISystem *self)
{
	IpcamISystemPrivate *priv = ipcam_isystem_get_instance_private(self);
	int i;

	for (i = 0; i < ARRAY_SIZE(priv->delayed_works); i++)
	{
		ISystemDelayedWork *work = &priv->delayed_works[i];
		work->enabled = FALSE;
		work->end_time = g_get_monotonic_time();
		work->func = NULL;
		work->user_data = NULL;
	}
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
    const gchar *interval = ipcam_base_app_get_config(IPCAM_BASE_APP(isystem), "system_led:interval");
    if (interval != NULL)
    {
        ipcam_base_app_add_timer(IPCAM_BASE_APP(isystem), "status_led_proc", interval, ipcam_isystem_status_led_proc);
    }
    else
    {
        ipcam_base_app_add_timer(IPCAM_BASE_APP(isystem), "status_led_proc", "1", ipcam_isystem_status_led_proc);
    }
}
static void ipcam_isystem_in_loop_impl(IpcamISystem *isystem)
{
	IpcamISystemPrivate *priv = ipcam_isystem_get_instance_private(isystem);
	int i;

	for (i = 0; i < ARRAY_SIZE(priv->delayed_works); i++)
	{
		gint64 now;
		ISystemDelayedWork *work = &priv->delayed_works[i];
		if (!work->enabled)
			continue;
		now = g_get_monotonic_time();
		if (now < work->end_time)
			continue;
		work->enabled = FALSE;
		if (work->func) {
			gint64 ret = work->func(work->user_data);
			if (ret > 0) {
				work->end_time = now + ret;
				work->enabled = TRUE;
			}
		}
	}
}

static void ipcam_isystem_status_led_proc(GObject *obj)
{
    g_return_if_fail(IPCAM_IS_ISYSTEM(obj));
    static gboolean light_on = FALSE;
    void *pMem  = NULL;
    const gchar *addr = ipcam_base_app_get_config(IPCAM_BASE_APP(obj), "system_led:address");
    const gchar *val = NULL;

    if (light_on)
    {
        light_on = FALSE;
        val = ipcam_base_app_get_config(IPCAM_BASE_APP(obj), "system_led:off");
    }
    else
    {
        light_on = TRUE;
        val = ipcam_base_app_get_config(IPCAM_BASE_APP(obj), "system_led:on");
    }
    if (val != NULL && addr != NULL)
    {
        guint32 address = g_ascii_strtoull(addr, NULL, 16);
        guint32 value = g_ascii_strtoull(val, NULL, 16);
        pMem = memmap(address, DEFAULT_MD_LEN);
        *(guint32*)pMem = value;
    }
}

static gint64 ipcam_isystem_apply_network_parameter(void *user_data)
{
	IpcamISystem *isystem = IPCAM_ISYSTEM(user_data);
	const gchar *script;

	script = ipcam_base_app_get_config(IPCAM_BASE_APP(isystem), "scripts:network");
	if (script) {
		FILE *fp;

		fp = popen(script, "w");
		if (fp == NULL)
		{
			perror("error exec network script:");
		}
		else
		{
			pclose(fp);
		}
	}

	return 0;
}

static void ipcam_isystem_sched_delayed_work(IpcamISystem *isystem, guint work_id,
                                             gint64 timeout_ms,
                                             DELAYED_WORK_FUNC func,
                                             gpointer user_data)
{
	IpcamISystemPrivate *priv = ipcam_isystem_get_instance_private(isystem);
	ISystemDelayedWork *work;

	g_return_if_fail(work_id >= 0 && work_id < ARRAY_SIZE(priv->delayed_works));
	g_return_if_fail(func != NULL);

	work = &priv->delayed_works[work_id];
	work->end_time = g_get_monotonic_time() + timeout_ms * 1000;
	work->func = func;
	work->user_data = user_data;
	work->enabled = TRUE;
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
	if (json_object_has_member(items_obj, "method") ||
	    json_object_has_member(items_obj, "address"))
	{
		const gchar *delay_str = ipcam_base_app_get_config(IPCAM_BASE_APP(isystem),
		                                                   "delayed_work:network");
		gint64 delay_ms = strtoul(delay_str, NULL, 0);
		ipcam_isystem_sched_delayed_work(isystem, NETWORK_DELAYED_WORK_ID, delay_ms,
		                                 ipcam_isystem_apply_network_parameter,
		                                 isystem);
	}
}

void ipcam_isystem_update_datetime_setting(IpcamISystem *isystem, JsonNode *body)
{
#if 0
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
#endif
}
