/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * sysutils.c
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

#define _GNU_SOURCE
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <printf.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/route.h>
#include <arpa/inet.h>
#include <errno.h>
#include "sysutils.h"

static char *dbname = "/data/configuration.sqlite3";

int sysutils_device_set(const char *key, const char *value)
{
    char *cmd;
    FILE *fp;

    asprintf(&cmd, "sqlite3 %s "
             "\"update base_info set value='%s' where name='%s'\"",
             dbname, value, key);
    fp = popen(cmd, "r");
    free(cmd);
    if (fp == NULL)
        return -1;

    pclose(fp);

    return 0;
}

static char *__prefixlen2_mask(unsigned int prefix_len)
{
	struct in_addr addr = { 0 };
	int i;

	for (i = 31; i >= 32 - (int)prefix_len; i--)
		addr.s_addr |= (1 << i);
    addr.s_addr = htonl(addr.s_addr);
	return inet_ntoa(addr);
}

int sysutils_network_set_hwaddr(const char *ifname, const char *hwaddr)
{
    char *cmd;
    FILE *fp;

    asprintf(&cmd, "fw_setenv %s %s", "ethaddr", hwaddr);
    fp = popen(cmd, "r");
    free(cmd);
    if (fp == NULL)
        return -1;

    pclose(fp);

    return 0;
}

int sysutils_network_get_hwaddr(const char *ifname, char **hwaddr)
{
    char *cmd;
    FILE *fp;
    int ret = -1;

    asprintf(&cmd, "fw_printenv ethaddr | cut -d \"=\" -f 2");
    fp = popen(cmd, "r");
    free(cmd);
    if (fp == NULL)
        return ret;

    *hwaddr = calloc(18, sizeof(char));
    if (fread(*hwaddr, sizeof(char), 17, fp) < 17)
        free(*hwaddr);
    else
        ret = 0;
    pclose(fp);
    
    return ret;
}

int sysutils_network_get_address(const char *ifname,
                                 char **ipaddr,
                                 char **netmask,
                                 char **broadaddr)
{
    char *cmd;
    FILE *fp;
    int ret = -1;

    asprintf(&cmd, "ip -o -4 addr show dev %s", ifname);
    fp = popen(cmd, "r");
    free(cmd);

    if (fp) {
        char ip[32];
        guint mask;
        char brd[32];

        /* 
         * Output for example
         * 2: eth0    inet 192.168.10.15/24 brd 192.168.10.255 scope global eth0
         */
        if (fscanf(fp, "%*d:%*s inet %32[0-9.] %*c %d brd %32[0-9.] %*[^$] %*[$]",
                   ip, &mask, brd) == 3)
        {
            if (ipaddr)
                *ipaddr = strdup(ip);
            if (netmask)
                *netmask = strdup(__prefixlen2_mask(mask));
            if (broadaddr)
                *broadaddr = strdup(brd);

            ret = 0;
        }
        pclose(fp);
    }

    return ret;
}

int sysutils_network_set_address(const char *ifname,
                                 const char *ipaddr,
                                 const char *netmask,
                                 const char *broadaddr)
{
    char *cmd = NULL;
    char *_brd = NULL;
    char *_mask = NULL;
    FILE *fp;
    int ret = -1;

    if (broadaddr)
        asprintf(&_brd, "-broadcast %s", broadaddr);
    if (netmask)
        asprintf(&_mask, "netmask %s", netmask);

    asprintf(&cmd, "ifconfig %s %s %s %s", ifname,
             ipaddr ? ipaddr : "",
             netmask ? _mask : "",
             broadaddr ? _brd : "");
    fp = popen(cmd, "r");
    free(cmd);
    free(_brd);
    free(_mask);

    if (fp) {
        pclose(fp);

        ret = 0;
    }

    return ret;
}

int sysutils_network_get_gateway(const char *ifname, char **gwaddr)
{
    FILE *fp;
    int ret = -1;
    char buf[128];
    char gw[32];

    fp = popen("ip route", "r");
    if (fp == NULL)
        return -1;

        /* 
         * Output for example
         * default via 192.168.1.1 dev wlp8s0  proto static  metric 1024
         */
    while(!feof(fp)) {
        if (fgets(buf, sizeof(buf), fp) == NULL)
            continue;
        if (sscanf(buf, "default via %32s %*[^$] %*[$]", gw) == 1) {
            *gwaddr = strdup(gw);
            ret = 0;
            break;
        }
    }
    pclose(fp);

    return ret;
}

int sysutils_network_set_gateway(const char *ifname, const char *gwaddr)
{
    char *cmd;
    FILE *fp;

    /* Delete the route */
    fp = popen("ip route del default", "r");
    if (fp == NULL)
        return -1;
    pclose(fp);

    /* Add new route */
    asprintf(&cmd, "ip route add default via %s dev %s", gwaddr, ifname);
    fp = popen(cmd, "r");
    free(cmd);
    if (fp == NULL)
        return -1;

    pclose(fp);

    return 0;
}
