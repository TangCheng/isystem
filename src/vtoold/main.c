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

#include "sysutils.h"

static char netif[16] = "eth0";
static int __server_port = 6000;
static struct sockaddr_in peer_addr;

static JsonNode *vtool_discovery(JsonNode *request)
{
	JsonObject *req_obj;
	JsonBuilder *builder;
	JsonNode *resp_node;
	char *ipaddr = NULL;
	char *netmask = NULL;
	char *gateway = NULL;
	char *hwaddr = NULL;

	req_obj = json_node_get_object(request);

	g_return_val_if_fail(req_obj != NULL, NULL);
	g_return_val_if_fail(json_object_has_member(req_obj, "msg_num"), NULL);

	gint64 msg_num = json_object_get_int_member(req_obj, "msg_num");

	if (json_object_has_member(req_obj, "ipaddr")) {
		const gchar *ip = json_object_get_string_member(req_obj, "ipaddr");
		inet_aton(ip, &peer_addr.sin_addr);
	}
	if (json_object_has_member(req_obj, "port")) {
		gint64 port = json_object_get_int_member(req_obj, "port");
		peer_addr.sin_port = htons((int)port);
	}

	sysutils_network_get_address(netif, &ipaddr, &netmask, NULL);
	sysutils_network_get_hwaddr(netif, &hwaddr);
	sysutils_network_get_gateway(netif, &gateway);
	builder = json_builder_new();

	json_builder_begin_object(builder);
	json_builder_set_member_name(builder, "cmd");
	json_builder_add_int_value(builder, 2);
	json_builder_set_member_name(builder, "msg_num");
	json_builder_add_int_value(builder, msg_num);
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
	json_builder_end_object(builder);

	resp_node = json_builder_get_root(builder);

	g_object_unref(builder);

	return resp_node;
}

static JsonNode *vtool_configure(JsonNode *request)
{
	JsonObject *req_obj;
	JsonBuilder *builder;
	JsonNode *resp_node;

	req_obj = json_node_get_object(request);

	g_return_val_if_fail(req_obj != NULL, NULL);
	g_return_val_if_fail(json_object_has_member(req_obj, "msg_num"), NULL);

	gint64 msg_num = json_object_get_int_member(req_obj, "msg_num");

	if (json_object_has_member(req_obj, "name")) {
		const gchar *devname = json_object_get_string_member(req_obj, "name");
		if (devname) {
			sysutils_device_set("device_name", devname);
		}
	}
	if (json_object_has_member(req_obj, "hwaddr")) {
		const gchar *hwaddr = json_object_get_string_member(req_obj, "hwaddr");
		if (hwaddr) {
			sysutils_network_set_hwaddr(netif, hwaddr);
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
		const gchar *device_type = json_object_get_string_member(req_obj, "device_type");
		if (device_type) {
			sysutils_device_set("device_type", device_type);
		}
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
	json_builder_add_int_value(builder, 4);
	json_builder_set_member_name(builder, "msg_num");
	json_builder_add_int_value(builder, msg_num);
	json_builder_set_member_name(builder, "status");
	json_builder_add_int_value(builder, 0);
	json_builder_end_object(builder);

	resp_node = json_builder_get_root(builder);

	g_object_unref(builder);

	return resp_node;
}

static void vtool_process_request(int sockfd, struct sockaddr_in *addr,
                                  const char *req_buf, size_t buf_size)
{
	JsonParser *parser = json_parser_new();
	if (json_parser_load_from_data(parser, req_buf, buf_size, NULL)) {
		JsonNode *root_node = json_parser_get_root(parser);
		JsonObject *root_obj = json_node_get_object(root_node);
		JsonNode *response = NULL;
		if (json_object_has_member(root_obj, "cmd")) {
			gint64 cmd = json_object_get_int_member(root_obj, "cmd");

			switch((int)cmd) {
			case 1:
				response = vtool_discovery(root_node);
				break;
			case 3:
				response = vtool_configure(root_node);
				break;
			default:
				break;
			}
		}

		if (response) {
			gsize text_length = 0;
			JsonGenerator *generator = json_generator_new();
			json_generator_set_root(generator, response);

			gchar *text = json_generator_to_data(generator, &text_length);

			if (text && text_length > 0) {
				sendto(sockfd, text, text_length, MSG_DONTROUTE,
				       (struct sockaddr*)&peer_addr, sizeof(peer_addr));

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

	bzero(&peer_addr, sizeof(peer_addr));

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
		printf("recv request msg : %s\n", req_buf);

		vtool_process_request(sockfd, &addr_from, req_buf, ret);
	}

	return 0;
}

