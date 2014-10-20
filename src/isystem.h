#ifndef __ISYSTEM_H__
#define __ISYSTEM_H__

#include <json-glib/json-glib.h>
#include <base_app.h>

#define IPCAM_ISYSTEM_TYPE (ipcam_isystem_get_type())
#define IPCAM_ISYSTEM(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), IPCAM_ISYSTEM_TYPE, IpcamISystem))
#define IPCAM_ISYSTEM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), IPCAM_ISYSTEM_TYPE, IpcamISystemClass))
#define IPCAM_IS_ISYSTEM(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), IPCAM_ISYSTEM_TYPE))
#define IPCAM_IS_ISYSTEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), IPCAM_ISYSTEM_TYPE))
#define IPCAM_ISYSTEM_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), IPCAM_ISYSTEM_TYPE, IpcamISystemClass))

typedef struct _IpcamISystem IpcamISystem;
typedef struct _IpcamISystemClass IpcamISystemClass;

struct _IpcamISystem
{
    IpcamBaseApp parent;
};

struct _IpcamISystemClass
{
    IpcamBaseAppClass parent_class;
};

GType ipcam_isystem_get_type(void);
void ipcam_isystem_update_network_setting(IpcamISystem *isystem, JsonNode *body);
void ipcam_isystem_update_datetime_setting(IpcamISystem *isystem, JsonNode *body);

#endif /* __ISYSTEM_H__ */
