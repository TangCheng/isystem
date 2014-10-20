#ifndef __ISYSTEM_EVENT_HANDLER_H__
#define __ISYSTEM_EVENT_HANDLER_H__

#include <event_handler.h>

#define IPCAM_ISYSTEM_EVENT_HANDLER_TYPE (ipcam_isystem_event_handler_get_type())
#define IPCAM_ISYSTEM_EVENT_HANDLER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), IPCAM_ISYSTEM_EVENT_HANDLER_TYPE, IpcamISystemEventHandler))
#define IPCAM_ISYSTEM_EVENT_HANDLER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), IPCAM_ISYSTEM_EVENT_HANDLER_TYPE, IpcamISystemEventHandlerClass))
#define IPCAM_IS_ISYSTEM_EVENT_HANDLER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), IPCAM_ISYSTEM_EVENT_HANDLER_TYPE))
#define IPCAM_IS_ISYSTEM_EVENT_HANDLER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), IPCAM_ISYSTEM_EVENT_HANDLER_TYPE))
#define IPCAM_ISYSTEM_EVENT_HANDLER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), IPCAM_ISYSTEM_EVENT_HANDLER_TYPE, IpcamISystemEventHandlerClass))

typedef struct _IpcamISystemEventHandler IpcamISystemEventHandler;
typedef struct _IpcamISystemEventHandlerClass IpcamISystemEventHandlerClass;

struct _IpcamISystemEventHandler
{
    IpcamEventHandler parent;
};

struct _IpcamISystemEventHandlerClass
{
    IpcamEventHandlerClass parent_class;
};

GType ipcam_isystem_event_handler_get_type(void);

#endif /* __ISYSTEM_EVENT_HANDLER_H__ */
