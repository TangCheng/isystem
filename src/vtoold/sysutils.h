/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * sysutils.h
 * Copyright (C) 2014 Watson Xu <xuhuashan@gmail.com>
 *
 * iconfig is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * iconfig is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SYSUTILS_H_
#define _SYSUTILS_H_

int sysutils_reset_system(const char *action);

int sysutils_device_set(const char *key, const char *value);

int sysutils_network_set_hwaddr(const char *ifname, const char *hwaddr);
int sysutils_network_get_hwaddr(const char *ifname, char **hwaddr);
int sysutils_network_get_address(const char *ifname,
                                 char **ipaddr,
                                 char **netmask);
int sysutils_network_set_address(const char *ifname,
                                 const char *ipaddr,
                                 const char *netmask);
int sysutils_network_get_gateway(const char *ifname, char **gwaddr);
int sysutils_network_set_gateway(const char *ifname, const char *gwaddr);

#endif // _SYSUTILS_H_

