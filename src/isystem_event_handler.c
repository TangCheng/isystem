#include <json-glib/json-glib.h>
#include "isystem_event_handler.h"
#include "messages.h"
#include "isystem.h"

G_DEFINE_TYPE(IpcamISystemEventHandler, ipcam_isystem_event_handler, IPCAM_EVENT_HANDLER_TYPE);

static void ipcam_isystem_event_handler_run_impl(IpcamISystemEventHandler *isystem_event_handler,
                                                 IpcamMessage *message);

static void ipcam_isystem_event_handler_init(IpcamISystemEventHandler *self)
{
}
static void ipcam_isystem_event_handler_class_init(IpcamISystemEventHandlerClass *klass)
{
    IpcamEventHandlerClass *event_handler_class = IPCAM_EVENT_HANDLER_CLASS(klass);
    event_handler_class->run = &ipcam_isystem_event_handler_run_impl;
}
static void ipcam_isystem_event_handler_run_impl(IpcamISystemEventHandler *isystem_event_handler,
                                                 IpcamMessage *message)
{
    IpcamISystem *isystem;
    const gchar *event;
    JsonNode *body;

    g_object_get(G_OBJECT(message), "event", &event, NULL);
    g_object_get(G_OBJECT(isystem_event_handler), "service", &isystem, NULL);
    g_object_get(G_OBJECT(message), "body", &body, NULL);
    
    if (g_strcmp0(event, "set_network") == 0)
    {
        ipcam_isystem_update_network_setting(isystem, body);
    }
    else if(g_strcmp0(event, "set_datetime") == 0)
    {
        ipcam_isystem_update_datetime_setting(isystem, body);
    }
    else
    {
        g_warn_if_reached();
    }
}
