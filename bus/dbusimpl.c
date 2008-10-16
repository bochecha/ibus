/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2008-2009 Huang Peng <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "dbusimpl.h"
#include "connection.h"

#define BUS_DBUS_IMPL_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_DBUS_IMPL, BusDBusImplPrivate))

enum {
    LAST_SIGNAL,
};

enum {
    PROP_0,
};


/* IBusDBusImplPriv */
struct _BusDBusImplPrivate {
    GHashTable *unique_names;
    GHashTable *names;
    GSList *connections;
    gint id;
};

typedef struct _BusDBusImplPrivate BusDBusImplPrivate;

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_dbus_impl_class_init      (BusDBusImplClass    *klass);
static void     bus_dbus_impl_init            (BusDBusImpl         *dbus_impl);
static void     bus_dbus_impl_dispose         (BusDBusImpl         *dbus_impl);
static gboolean bus_dbus_impl_dbus_message    (BusDBusImpl         *dbus_impl,
                                               BusConnection       *connection,
                                               DBusMessage         *message);

static IBusServiceClass  *_parent_class = NULL;

GType
bus_dbus_impl_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusDBusImplClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_dbus_impl_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusDBusImpl),
        0,
        (GInstanceInitFunc) bus_dbus_impl_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_SERVICE,
                    "BusDBusImpl",
                    &type_info,
                    (GTypeFlags) 0);
    }
    return type;
}

BusDBusImpl *
bus_dbus_impl_new (void)
{
    // BusDBusImplPrivate *priv;
    BusDBusImpl *dbus_impl;

    dbus_impl = BUS_DBUS_IMPL (g_object_new (BUS_TYPE_DBUS_IMPL,
                    "path", DBUS_PATH_DBUS,
                    NULL));

    return dbus_impl;
}

static void
bus_dbus_impl_class_init (BusDBusImplClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    IBusServiceClass *service_class = IBUS_SERVICE_CLASS (klass);

    _parent_class = (IBusServiceClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusDBusImplPrivate));

    gobject_class->dispose = (GObjectFinalizeFunc) bus_dbus_impl_dispose;

    service_class->dbus_message = (ServiceDBusMessageFunc) bus_dbus_impl_dbus_message;

}

static void
bus_dbus_impl_init (BusDBusImpl *dbus_impl)
{
    BusDBusImplPrivate *priv;
    priv = BUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    priv->unique_names = g_hash_table_new (g_str_hash, g_str_equal);
    priv->names = g_hash_table_new (g_str_hash, g_str_equal);
    priv->connections = NULL;
    priv->id = 1;

}

static void
bus_dbus_impl_dispose (BusDBusImpl *dbus_impl)
{
    BusDBusImplPrivate *priv;
    priv = BUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    G_OBJECT_CLASS(_parent_class)->dispose (G_OBJECT (dbus_impl));
}


/* introspectable interface */
static DBusMessage *
_dbus_introspect (BusDBusImpl     *dbus_impl,
                  DBusMessage     *message,
                  BusConnection   *connection)
{
    static const gchar *introspect =
        "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
        "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
        "<node>\n"
        "  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
        "    <method name=\"Introspect\">\n"
        "      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
        "    </method>\n"
        "  </interface>\n"
        "  <interface name=\"org.freedesktop.DBus\">\n"
        "    <method name=\"Hello\">\n"
        "      <arg direction=\"out\" type=\"s\"/>\n"
        "    </method>\n"
        "    <method name=\"RequestName\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"in\" type=\"u\"/>\n"
        "      <arg direction=\"out\" type=\"u\"/>\n"
        "    </method>\n"
        "    <method name=\"ReleaseName\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"u\"/>\n"
        "    </method>\n"
        "    <method name=\"StartServiceByName\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"in\" type=\"u\"/>\n"
        "      <arg direction=\"out\" type=\"u\"/>\n"
        "    </method>\n"
        "    <method name=\"UpdateActivationEnvironment\">\n"
        "      <arg direction=\"in\" type=\"a{ss}\"/>\n"
        "    </method>\n"
        "    <method name=\"NameHasOwner\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"b\"/>\n"
        "    </method>\n"
        "    <method name=\"ListNames\">\n"
        "      <arg direction=\"out\" type=\"as\"/>\n"
        "    </method>\n"
        "    <method name=\"ListActivatableNames\">\n"
        "      <arg direction=\"out\" type=\"as\"/>\n"
        "    </method>\n"
        "    <method name=\"AddMatch\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "    </method>\n"
        "    <method name=\"RemoveMatch\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "    </method>\n"
        "    <method name=\"GetNameOwner\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"s\"/>\n"
        "    </method>\n"
        "    <method name=\"ListQueuedOwners\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"as\"/>\n"
        "    </method>\n"
        "    <method name=\"GetConnectionUnixUser\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"u\"/>\n"
        "    </method>\n"
        "    <method name=\"GetConnectionUnixProcessID\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"u\"/>\n"
        "    </method>\n"
        "    <method name=\"GetAdtAuditSessionData\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"ay\"/>\n"
        "    </method>\n"
        "    <method name=\"GetConnectionSELinuxSecurityContext\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"out\" type=\"ay\"/>\n"
        "    </method>\n"
        "    <method name=\"ReloadConfig\">\n"
        "    </method>\n"
        "    <method name=\"GetId\">\n"
        "      <arg direction=\"out\" type=\"s\"/>\n"
        "    </method>\n"
        "    <signal name=\"NameOwnerChanged\">\n"
        "      <arg type=\"s\"/>\n"
        "      <arg type=\"s\"/>\n"
        "      <arg type=\"s\"/>\n"
        "    </signal>\n"
        "    <signal name=\"NameLost\">\n"
        "      <arg type=\"s\"/>\n"
        "    </signal>\n"
        "    <signal name=\"NameAcquired\">\n"
        "      <arg type=\"s\"/>\n"
        "    </signal>\n"
        "  </interface>\n"
        "</node>\n";

    DBusMessage *reply_message;
    reply_message = dbus_message_new_method_return (message);
    dbus_message_append_args (reply_message,
            DBUS_TYPE_STRING, &introspect,
            DBUS_TYPE_INVALID);

    return reply_message;
}


/* dbus interface */
static DBusMessage *
_dbus_no_implement (BusDBusImpl     *dbus_impl,
                    DBusMessage     *message,
                    BusConnection   *connection)
{
    DBusMessage *reply_message;
    reply_message = dbus_message_new_error_printf (message,
                                    "org.freedesktop.DBus.Error.Failed",
                                    "IBus does not support %s.",
                                    dbus_message_get_member (message));
    return reply_message;
}


static DBusMessage *
_dbus_hello (BusDBusImpl    *dbus_impl,
             DBusMessage    *message,
             BusConnection  *connection)
{
    BusDBusImplPrivate *priv;
    priv = BUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    DBusMessage *reply_message;

    if (bus_connection_get_unique_name (connection) != NULL) {
        reply_message = dbus_message_new_error (message,
                                    "org.freedesktop.DBus.Error.Failed",
                                    "Already handled an Hello message");
    }
    else {
        gchar *name = g_strdup_printf (":1.%d", priv->id ++);
        bus_connection_set_unique_name (connection, name);

        reply_message = dbus_message_new_method_return (message);
        dbus_message_append_args (reply_message,
                    DBUS_TYPE_STRING, &name,
                    DBUS_TYPE_INVALID);
        g_free (name);
    }

    return reply_message;
}

static DBusMessage *
_dbus_list_names (BusDBusImpl       *dbus_impl,
                  DBusMessage       *message,
                  BusConnection     *connection)
{
    BusDBusImplPrivate *priv;
    priv = BUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    DBusMessage *reply_message;
    DBusMessageIter iter, sub_iter;
    GList *name, *names;

    reply_message = dbus_message_new_method_return (message);

    dbus_message_iter_init_append (message, &iter);
    dbus_message_iter_open_container (&iter, DBUS_TYPE_ARRAY, "s", &sub_iter);

    // append unique names
    names = g_hash_table_get_keys (priv->unique_names);
    names = g_list_sort (names, (GCompareFunc) g_strcmp0);
    for (name = names; name != NULL; name = name->next) {
        dbus_message_iter_append_basic (&sub_iter, DBUS_TYPE_STRING, &(name->data));
    }
    g_list_free (names);

    // append well-known names
    names = g_hash_table_get_keys (priv->names);
    names = g_list_sort (names, (GCompareFunc) g_strcmp0);
    for (name = names; name != NULL; name = name->next) {
        dbus_message_iter_append_basic (&sub_iter, DBUS_TYPE_STRING, &(name->data));
    }
    g_list_free (names);

    dbus_message_iter_close_container (&iter, &sub_iter);

    return reply_message;
}

static DBusMessage *
_dbus_name_has_owner (BusDBusImpl   *dbus_impl,
                      DBusMessage   *message,
                      BusConnection *connection)
{
    BusDBusImplPrivate *priv;
    priv = BUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    gchar *name;
    gboolean retval;
    gboolean has_owner;
    DBusMessage *reply_message;
    DBusError error;

    dbus_error_init (&error);
    retval = dbus_message_get_args (message, &error,
                    DBUS_TYPE_STRING, &name,
                    DBUS_TYPE_INVALID);

    if (! retval) {
        reply_message = dbus_message_new_error (message,
                                    error.name,
                                    error.message);
        dbus_error_free (&error);
    }
    else {
        if (name[0] == ':') {
            has_owner = g_hash_table_lookup (priv->unique_names, name) != NULL;
        }
        else {
            has_owner = g_hash_table_lookup (priv->names, name) != NULL;
        }
        reply_message = dbus_message_new_method_return (message);
        dbus_message_append_args (reply_message,
                    DBUS_TYPE_BOOLEAN, &has_owner,
                    DBUS_TYPE_INVALID);
    }

    return reply_message;
}


static DBusMessage *
_dbus_get_name_owner (BusDBusImpl   *dbus_impl,
                      DBusMessage   *message,
                      BusConnection *connection)
{
    BusDBusImplPrivate *priv;
    priv = BUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    gchar *name;
    gboolean retval;
    const gchar *owner_name;
    DBusMessage *reply_message;
    DBusError error;

    dbus_error_init (&error);
    retval = dbus_message_get_args (message, &error,
                    DBUS_TYPE_STRING, &name,
                    DBUS_TYPE_INVALID);

    if (! retval) {
        reply_message = dbus_message_new_error (message,
                                    error.name,
                                    error.message);
        dbus_error_free (&error);
    }
    else {
        BusConnection *owner;
        if (name[0] == ':') {
            owner = BUS_CONNECTION (g_hash_table_lookup (priv->unique_names, name));
        }
        else {
            owner = BUS_CONNECTION (g_hash_table_lookup (priv->names, name));
        }

        if (owner != NULL) {
            owner_name = bus_connection_get_unique_name (owner);
            reply_message = dbus_message_new_method_return (message);
            dbus_message_append_args (reply_message,
                    DBUS_TYPE_STRING, &owner_name,
                    DBUS_TYPE_INVALID);
        }
        else {
            reply_message = dbus_message_new_error_printf (message,
                                    "org.freedesktop.DBus.Error.NameHasNoOwner",
                                    "Name %s does have owner",
                                    name);
        }
    }

    return reply_message;
}

static DBusMessage *
_dbus_get_id (BusDBusImpl   *dbus_impl,
              DBusMessage   *message,
              BusConnection *connection)
{
    BusDBusImplPrivate *priv;
    priv = BUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    DBusMessage *reply_message;
    const gchar *name;

    name = bus_connection_get_unique_name (connection);

    if (name == NULL) {
        reply_message = dbus_message_new_error (message,
                                    "org.freedesktop.DBus.Error.Failed",
                                    "Can not GetId before Hello");
    }
    else {
        reply_message = dbus_message_new_method_return (message);
        dbus_message_append_args (reply_message,
                    DBUS_TYPE_STRING, &name,
                    DBUS_TYPE_INVALID);
    }

    return reply_message;
}

static DBusMessage *
_dbus_add_match (BusDBusImpl    *dbus_impl,
                 DBusMessage    *message,
                 BusConnection  *connection)
{
    BusDBusImplPrivate *priv;
    priv = BUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    DBusMessage *reply_message;
    DBusError error;
    gboolean retval;
    gchar *rule;

    dbus_error_init (&error);
    retval = dbus_message_get_args (message, &error,
            DBUS_TYPE_STRING, &rule,
            DBUS_TYPE_INVALID);

    if (!retval) {
        reply_message = dbus_message_new_error (message,
                                    error.name,
                                    error.message);
        dbus_error_free (&error);
    }
    else {
        reply_message = dbus_message_new_method_return (message);
    }

    return reply_message;
}

static DBusMessage *
_dbus_remove_match (BusDBusImpl     *dbus_impl,
                    DBusMessage     *message,
                    BusConnection   *connection)
{
    BusDBusImplPrivate *priv;
    priv = BUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    DBusMessage *reply_message;
    DBusError error;
    gchar *rule;

    dbus_error_init (&error);
    if (!dbus_message_get_args (message, &error,
            DBUS_TYPE_STRING, &rule,
            DBUS_TYPE_INVALID)) {
        reply_message = dbus_message_new_error (message,
                                    error.name,
                                    error.message);
        dbus_error_free (&error);
    }
    else {
        reply_message = dbus_message_new_method_return (message);
    }

    return reply_message;
}

static DBusMessage *
_dbus_request_name (BusDBusImpl     *dbus_impl,
                    DBusMessage     *message,
                    BusConnection   *connection)
{
    BusDBusImplPrivate *priv;
    priv = BUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    DBusMessage *reply_message;
    DBusError error;
    gchar *name;
    guint flags;
    guint retval;

    dbus_error_init (&error);
    if (!dbus_message_get_args (message, &error,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_UINT32, &flags,
            DBUS_TYPE_INVALID)) {
        reply_message = dbus_message_new_error (message,
                                    error.name,
                                    error.message);
        dbus_error_free (&error);
    }
    else {
        if (g_hash_table_lookup (priv->names, name)) {
            reply_message = dbus_message_new_error_printf (message,
                        "org.freedesktop.DBus.Error.Failed",
                        "Name %s has owner", name);
        }
        else {
            retval = 1;
            g_hash_table_insert (priv->names,
                    (gpointer )bus_connection_add_name (connection, name),
                    connection);
            reply_message = dbus_message_new_method_return (message);
            dbus_message_append_args (message,
                    DBUS_TYPE_UINT32, &retval,
                    DBUS_TYPE_INVALID);
        }
    }

    return reply_message;
}

static DBusMessage *
_dbus_release_name (BusDBusImpl     *dbus_impl,
                    DBusMessage     *message,
                    BusConnection   *connection)
{
    BusDBusImplPrivate *priv;
    priv = BUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    DBusMessage *reply_message;
    DBusError error;
    gchar *name;
    guint retval;

    dbus_error_init (&error);
    if (!dbus_message_get_args (message, &error,
            DBUS_TYPE_STRING, &name,
            DBUS_TYPE_INVALID)) {
        reply_message = dbus_message_new_error (message,
                                    error.name,
                                    error.message);
        dbus_error_free (&error);
    }
    else {
        reply_message = dbus_message_new_method_return (message);
        if (bus_connection_remove_name (connection, name)) {
            retval = 1;
        }
        else {
            retval = 2;
        }
        dbus_message_append_args (message,
                    DBUS_TYPE_UINT32, &retval,
                    DBUS_TYPE_INVALID);
    }

    return reply_message;
}


static gboolean
bus_dbus_impl_dbus_message (BusDBusImpl     *dbus_impl,
                            BusConnection   *connection,
                            DBusMessage     *message)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus_impl));
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    gint i;
    DBusMessage *reply_message = NULL;

    struct {
        const gchar *interface;
        const gchar *name;
        DBusMessage *(* handler) (BusDBusImpl *, DBusMessage *, BusConnection *);
    } handlers[] =  {
        /* Introspectable interface */
        { DBUS_INTERFACE_INTROSPECTABLE,
                               "Introspect", _dbus_introspect },
        /* DBus interface */
        { DBUS_INTERFACE_DBUS, "Hello",     _dbus_hello },
        { DBUS_INTERFACE_DBUS, "ListNames", _dbus_list_names },
        { DBUS_INTERFACE_DBUS, "ListActivatableNames",
                                            _dbus_no_implement },
        { DBUS_INTERFACE_DBUS, "NameHasOwner",
                                            _dbus_name_has_owner },
        { DBUS_INTERFACE_DBUS, "StartServiceByName",
                                            _dbus_no_implement },
        { DBUS_INTERFACE_DBUS, "GetNameOwner",
                                            _dbus_get_name_owner },
        { DBUS_INTERFACE_DBUS, "GetConnectionUnixUser",
                                            _dbus_no_implement },
        { DBUS_INTERFACE_DBUS, "AddMatch",  _dbus_add_match },
        { DBUS_INTERFACE_DBUS, "RemoveMatch",
                                            _dbus_remove_match },
        { DBUS_INTERFACE_DBUS, "GetId",     _dbus_get_id },
        { DBUS_INTERFACE_DBUS, "RequestName", _dbus_request_name },
        { DBUS_INTERFACE_DBUS, "ReleaseName", _dbus_release_name },
        { NULL, NULL, NULL }
    };

    dbus_message_set_sender (message, bus_connection_get_unique_name (connection));
    dbus_message_set_destination (message, DBUS_SERVICE_DBUS);

    for (i = 0; handlers[i].interface != NULL; i++) {
        if (dbus_message_is_method_call (message,
                                         handlers[i].interface,
                                         handlers[i].name)) {

            reply_message = handlers[i].handler (dbus_impl, message, connection);
            if (reply_message) {

                dbus_message_set_sender (reply_message, DBUS_SERVICE_DBUS);
                dbus_message_set_destination (reply_message, bus_connection_get_unique_name (connection));
                dbus_message_set_no_reply (reply_message, TRUE);

                ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
                dbus_message_unref (reply_message);
            }

            g_signal_stop_emission_by_name (dbus_impl, "dbus-message");
            return TRUE;
        }
    }

    reply_message = dbus_message_new_error_printf (message,
                                "org.freedesktop.DBus.Error.NoImplement",
                                "%s is not implemented",
                                dbus_message_get_member (message));

    ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
    dbus_message_unref (reply_message);
    return FALSE;
}

static void
_connection_destroy_cb (BusConnection   *connection,
                        BusDBusImpl     *dbus_impl)
{
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (BUS_IS_DBUS_IMPL (dbus_impl));

    BusDBusImplPrivate *priv;
    priv = BUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    /*
    ibus_service_remove_from_connection (
                    IBUS_SERVICE (dbus_impl),
                    IBUS_CONNECTION (connection));
    */

    const gchar *unique_name = bus_connection_get_unique_name (connection);
    if (unique_name != NULL)
        g_hash_table_remove (priv->unique_names, unique_name);

    const GSList *name = bus_connection_get_names (connection);

    while (name != NULL) {
        g_hash_table_remove (priv->names, name->data);
        name = name->next;
    }

    priv->connections = g_slist_remove (priv->connections, connection);
    g_object_unref (connection);
}


gboolean
bus_dbus_impl_new_connection (BusDBusImpl    *dbus_impl,
                              BusConnection  *connection)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus_impl));
    g_assert (BUS_IS_CONNECTION (connection));

    BusDBusImplPrivate *priv;
    priv = BUS_DBUS_IMPL_GET_PRIVATE (dbus_impl);

    g_assert (g_slist_find (priv->connections, connection) == NULL);

    g_object_ref (G_OBJECT (connection));
    priv->connections = g_slist_append (priv->connections, connection);

    g_signal_connect (connection, "destroy",
                      (GCallback) _connection_destroy_cb,
                      dbus_impl);

    ibus_service_add_to_connection (
            IBUS_SERVICE (dbus_impl),
            IBUS_CONNECTION (connection));

    return TRUE;
}
