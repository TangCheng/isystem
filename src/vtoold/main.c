/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.c
 * Copyright (C) 2015 Watson Xu <xuhuashan@gmail.com>
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <json-glib/json-glib.h>

#define DEBUG	   1
#ifdef DEBUG
# define DBG_PRINT(...)	printf(__VA_ARGS__)
#else
# define DBG_PRINT(...)
#endif

#include "sysutils.h"

static char netif[16] = "eth0";
static int __server_port = 6000;
static char __hwaddr[32];

static JsonNode *vtool_discovery(JsonNode *request, struct sockaddr_in *peer)
{
	JsonObject *req_obj;
	JsonBuilder *builder;
	JsonNode *resp_node;
	char *ipaddr = NULL;
	char *netmask = NULL;
	char *gateway = NULL;
	char *hwaddr = NULL;
	char *device_name = NULL;
	char *serial = NULL;
	char *manufacturer = NULL;
	char *device_type = NULL;
	char *model = NULL;

	req_obj = json_node_get_object(request);

	g_return_val_if_fail(req_obj != NULL, NULL);
	g_return_val_if_fail(json_object_has_member(req_obj, "msg_num"), NULL);
	g_return_val_if_fail(json_object_has_member(req_obj, "tool_ip"), NULL);
	g_return_val_if_fail(json_object_has_member(req_obj, "tool_port"), NULL);

	gint64 msg_num = json_object_get_int_member(req_obj, "msg_num");


	const gchar *tool_ip;
	gint64 tool_port;

	tool_ip = json_object_get_string_member(req_obj, "tool_ip");
	inet_aton(tool_ip, &peer->sin_addr);
	tool_port = json_object_get_int_member(req_obj, "tool_port");
	peer->sin_port = htons((int)tool_port);

	sysutils_network_get_address(netif, &ipaddr, &netmask);
	sysutils_network_get_hwaddr(netif, &hwaddr);
	sysutils_network_get_gateway(netif, &gateway);
	sysutils_device_get("device_name", &device_name);
	sysutils_device_get("serial", &serial);
	sysutils_device_get("manufacturer", &manufacturer);
	sysutils_device_get("device_type", &device_type);
	

	builder = json_builder_new();

	json_builder_begin_object(builder);
	json_builder_set_member_name(builder, "cmd");
	json_builder_add_int_value(builder, 2);
	json_builder_set_member_name(builder, "msg_num");
	json_builder_add_int_value(builder, msg_num);
	json_builder_set_member_name(builder, "status");
	json_builder_add_int_value(builder, 0);
	json_builder_set_member_name(builder, "name");
	json_builder_add_string_value(builder, device_name);
	json_builder_set_member_name(builder, "ipaddr");
	json_builder_add_string_value(builder, ipaddr);
	json_builder_set_member_name(builder, "netmask");
	json_builder_add_string_value(builder, netmask);
	json_builder_set_member_name(builder, "gateway");
	json_builder_add_string_value(builder, gateway);
	json_builder_set_member_name(builder, "hwaddr");
	json_builder_add_string_value(builder, hwaddr);
	json_builder_set_member_name(builder, "port");
	json_builder_add_int_value(builder, __server_port);
	json_builder_set_member_name(builder, "serial");
	json_builder_add_string_value(builder, serial);
	json_builder_set_member_name(builder, "manufacturer");
	json_builder_add_string_value(builder, manufacturer);
	json_builder_set_member_name(builder, "device_type");
	gint64 dt = 0;
	if (device_type)
		dt = strtoul(device_type, NULL, 0);
	json_builder_add_int_value(builder, dt);
	json_builder_end_object(builder);

	resp_node = json_builder_get_root(builder);

	g_object_unref(builder);

	free(ipaddr);
	free(netmask);
	free(gateway);
	free(hwaddr);
	free(device_name);
	free(serial);
	free(manufacturer);
	free(device_type);
	free(model);

	return resp_node;
}

static JsonNode *vtool_set_hwaddr(JsonNode *request, struct sockaddr_in *peer)
{
	JsonObject *req_obj;
	JsonBuilder *builder;
	JsonNode *resp_node;

	req_obj = json_node_get_object(request);

	g_return_val_if_fail(req_obj != NULL, NULL);
	g_return_val_if_fail(json_object_has_member(req_obj, "msg_num"), NULL);
	g_return_val_if_fail(json_object_has_member(req_obj, "tool_ip"), NULL);
	g_return_val_if_fail(json_object_has_member(req_obj, "tool_port"), NULL);

	gint64 msg_num = json_object_get_int_member(req_obj, "msg_num");

	const gchar *tool_ip;
	gint64 tool_port;

	tool_ip = json_object_get_string_member(req_obj, "tool_ip");
	inet_aton(tool_ip, &peer->sin_addr);
	tool_port = json_object_get_int_member(req_obj, "tool_port");
	peer->sin_port = htons((int)tool_port);

	const gchar *hwaddr = json_object_get_string_member(req_obj, "hwaddr");
	if (hwaddr) {
		sysutils_network_set_hwaddr(netif, hwaddr);
	}
	strncpy(__hwaddr, hwaddr, sizeof(hwaddr));

	builder = json_builder_new();

	json_builder_begin_object(builder);
	json_builder_set_member_name(builder, "cmd");
	json_builder_add_int_value(builder, 4);
	json_builder_set_member_name(builder, "msg_num");
	json_builder_add_int_value(builder, msg_num);
	json_builder_set_member_name(builder, "status");
	json_builder_add_int_value(builder, 0);
	json_builder_set_member_name(builder, "hwaddr");
	json_builder_add_string_value(builder, hwaddr);
	json_builder_end_object(builder);

	resp_node = json_builder_get_root(builder);

	g_object_unref(builder);

	return resp_node;
}

static JsonNode *vtool_set_ipaddr(JsonNode *request, struct sockaddr_in *peer)
{
	JsonObject *req_obj;
	JsonBuilder *builder;
	JsonNode *resp_node;

	req_obj = json_node_get_object(request);

	g_return_val_if_fail(req_obj != NULL, NULL);
	g_return_val_if_fail(json_object_has_member(req_obj, "msg_num"), NULL);
	g_return_val_if_fail(json_object_has_member(req_obj, "tool_ip"), NULL);
	g_return_val_if_fail(json_object_has_member(req_obj, "tool_port"), NULL);

	gint64 msg_num = json_object_get_int_member(req_obj, "msg_num");

	const gchar *tool_ip;
	gint64 tool_port;

	tool_ip = json_object_get_string_member(req_obj, "tool_ip");
	inet_aton(tool_ip, &peer->sin_addr);
	tool_port = json_object_get_int_member(req_obj, "tool_port");
	peer->sin_port = htons((int)tool_port);

	const gchar *hwaddr = json_object_get_string_member(req_obj, "hwaddr");
	const gchar *ipaddr = json_object_get_string_member(req_obj, "ipaddr");
	const gchar *netmask = json_object_get_string_member(req_obj, "netmask");
	const gchar *gateway = json_object_get_string_member(req_obj, "gateway");
	if (hwaddr && ipaddr && netmask) {
		sysutils_network_set_address(netif, ipaddr, netmask);
	}
	if (gateway) {
		sysutils_network_set_gateway(netif, gateway);
	}

	builder = json_builder_new();

	json_builder_begin_object(builder);
	json_builder_set_member_name(builder, "cmd");
	json_builder_add_int_value(builder, 6);
	json_builder_set_member_name(builder, "msg_num");
	json_builder_add_int_value(builder, msg_num);
	json_builder_set_member_name(builder, "status");
	json_builder_add_int_value(builder, 0);
	json_builder_set_member_name(builder, "hwaddr");
	json_builder_add_string_value(builder, __hwaddr);
	json_builder_end_object(builder);

	resp_node = json_builder_get_root(builder);

	g_object_unref(builder);

	return resp_node;
}

static JsonNode *vtool_set_device_info(JsonNode *request, struct sockaddr_in *peer)
{
	JsonObject *req_obj;
	JsonBuilder *builder;
	JsonNode *resp_node;

	req_obj = json_node_get_object(request);

	g_return_val_if_fail(req_obj != NULL, NULL);
	g_return_val_if_fail(json_object_has_member(req_obj, "msg_num"), NULL);
	g_return_val_if_fail(json_object_has_member(req_obj, "tool_ip"), NULL);
	g_return_val_if_fail(json_object_has_member(req_obj, "tool_port"), NULL);

	gint64 msg_num = json_object_get_int_member(req_obj, "msg_num");

	const gchar *tool_ip;
	gint64 tool_port;

	tool_ip = json_object_get_string_member(req_obj, "tool_ip");
	inet_aton(tool_ip, &peer->sin_addr);
	tool_port = json_object_get_int_member(req_obj, "tool_port");
	peer->sin_port = htons((int)tool_port);

	if (json_object_has_member(req_obj, "name")) {
		const gchar *devname = json_object_get_string_member(req_obj, "name");
		if (devname) {
			sysutils_device_set("device_name", devname);
		}
	}
	if (json_object_has_member(req_obj, "serial")) {
		const gchar *serial = json_object_get_string_member(req_obj, "serial");
		if (serial) {
			sysutils_device_set("serial", serial);
		}
	}
	if (json_object_has_member(req_obj, "manufacturer")) {
		const gchar *manufacturer = json_object_get_string_member(req_obj, "manufacturer");
		if (manufacturer) {
			sysutils_device_set("manufacturer", manufacturer);
		}
	}
	if (json_object_has_member(req_obj, "device_type")) {
		int device_type = json_object_get_int_member(req_obj, "device_type");
		char str[16];

		snprintf(str, sizeof(str), "%d", device_type);
		sysutils_device_set("device_type", str);
	}
	if (json_object_has_member(req_obj, "model")) {
		const gchar *model = json_object_get_string_member(req_obj, "model");
		if (model) {
			sysutils_device_set("model", model);
		}
	}

	builder = json_builder_new();

	json_builder_begin_object(builder);
	json_builder_set_member_name(builder, "cmd");
	json_builder_add_int_value(builder, 22);
	json_builder_set_member_name(builder, "msg_num");
	json_builder_add_int_value(builder, msg_num);
	json_builder_set_member_name(builder, "status");
	json_builder_add_int_value(builder, 0);
	json_builder_set_member_name(builder, "hwaddr");
	json_builder_add_string_value(builder, __hwaddr);
	json_builder_end_object(builder);

	resp_node = json_builder_get_root(builder);

	g_object_unref(builder);

	return resp_node;
}

static void vtool_process_request(int sockfd, struct sockaddr_in *addr,
                                  const char *req_buf, size_t buf_size)
{
	JsonParser *parser = json_parser_new();
	struct sockaddr_in peer;

	bzero(&peer, sizeof(peer));
	peer.sin_family = AF_INET;
	peer.sin_addr.s_addr = htonl(INADDR_ANY);
	peer.sin_port = htons(__server_port + 1);

	if (json_parser_load_from_data(parser, req_buf, buf_size, NULL)) {
		JsonNode *root_node = json_parser_get_root(parser);
		JsonObject *root_obj = json_node_get_object(root_node);
		JsonNode *response = NULL;
		if (json_object_has_member(root_obj, "cmd")) {
			gint64 cmd = json_object_get_int_member(root_obj, "cmd");

			switch((int)cmd) {
			case 1:
				response = vtool_discovery(root_node, &peer);
				break;
			case 3:
				response = vtool_set_hwaddr(root_node, &peer);
				break;
			case 5:
				response = vtool_set_ipaddr(root_node, &peer);
				break;
			case 21:
				response = vtool_set_device_info(root_node, &peer);
				break;
			default:
				break;
			}
		}

		if (response) {
			gsize text_length = 0;
			JsonGenerator *generator = json_generator_new();
			json_generator_set_root(generator, response);
			json_generator_set_pretty(generator, TRUE);

			gchar *text = json_generator_to_data(generator, &text_length);

			DBG_PRINT("%s ==> [%s:%d]\n",
					  text,
					  inet_ntoa(peer.sin_addr),
					  ntohs(peer.sin_port));

			if (text && text_length > 0) {
				int ret;

				ret = sendto(sockfd, text, text_length, 0,
							 (struct sockaddr*)&peer, sizeof(peer));
				if (ret < 0) {
					perror("sendto failed:");
				}

				g_free(text);
			}

			g_object_unref(generator);
			json_node_free(response);
		}
	}
	g_object_unref(parser);
}

static const char *sopts = "i:p:";
static struct option lopts[] = {
	{ "server-port", required_argument, NULL, 'p' },
	{ "interface",   required_argument, NULL, 'i' }
};

int main(int argc, char *argv[])
{
	int sockfd;
	int ret;
	const int sock_opt = 1;
	struct sockaddr_in addr_from;
	struct sockaddr_in addr_rcv;
	int addr_from_len = sizeof(struct sockaddr_in);
	char req_buf[4096];

	int opt;
	int lidx;
	while ((opt = getopt_long(argc, argv, sopts, lopts, &lidx)) != -1) {
		switch (opt) {
		case 'i':
			strncpy(netif, optarg, sizeof(netif));
			break;
		case 'p':
			__server_port = strtoul(optarg, NULL, 0);
			break;
		}
	}

	char *hwaddr = NULL;
	if (sysutils_network_get_hwaddr(netif, &hwaddr) < 0) {
		perror("Cannot get hwaddr\n");
		return -1;
	}
	strncpy(__hwaddr, hwaddr, sizeof(__hwaddr));

	bzero(&addr_rcv, sizeof(addr_rcv));
	addr_rcv.sin_family = AF_INET;
	addr_rcv.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_rcv.sin_port = htons(__server_port);

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		fprintf(stderr, "socket error\n");
		return -1;
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,
	                 (char *)&sock_opt, sizeof(sock_opt));
	if(ret == -1)
	{
		perror("failed to setsockopt:");
		return -1;
	}

	if (bind(sockfd, (struct sockaddr *)&addr_rcv, sizeof(addr_rcv)) == -1)
	{
		perror("bind error:");
		return -1;
	}

	while(1)
	{
		int ret = recvfrom(sockfd, req_buf, sizeof(req_buf) - 1, 0,
		                   (struct sockaddr*)&addr_from,
		                   (socklen_t*)&addr_from_len);
		if(ret <= 0)
		{
			fprintf(stderr, "read error...\n");
			sleep (1);
			continue;
		}

		req_buf[ret] = 0;

		DBG_PRINT("\n%s <== [%s:%d]\n",
				  req_buf,
				  inet_ntoa(addr_from.sin_addr),
				  ntohs(addr_from.sin_port));

		vtool_process_request(sockfd, &addr_from, req_buf, ret);
	}

	return 0;
}

