#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <cJSON.h>

#define SERVER_PATH "/tmp/radvd.sock"
#define MAXLINE 2048

// Driver code
int main(int argc, char *argv[])
{
	int client_socket;
	struct sockaddr_un remote;

	memset(&remote, 0, sizeof(struct sockaddr_un));
	client_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (client_socket == -1) {
		printf("SOCKET ERROR");
		exit(1);
	}

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SERVER_PATH);

	printf("Sending data...\n");

	cJSON *ra_config = cJSON_CreateObject();
	cJSON *interface = cJSON_CreateObject();
	cJSON *prefix1 = cJSON_CreateObject();
	cJSON *prefix2 = cJSON_CreateObject();
	cJSON *prefix3 = cJSON_CreateObject();
	cJSON *prefixes = cJSON_CreateArray();

	cJSON_bool adv_autonomous = 1;
	cJSON_bool adv_on_link = 1;
	cJSON_bool action1 = atoi(argv[5]);
	cJSON_bool action2 = atoi(argv[6]);
	cJSON_bool action3 = atoi(argv[7]);
	cJSON_bool action4 = atoi(argv[8]);
	double max_rtr_interval = 650.5;
	double min_rtr_interval = 350.5;
	double adv_default_lifetime = 2 * max_rtr_interval;
	cJSON_AddItemToObject(ra_config, "interface", interface);
	cJSON_AddBoolToObject(interface, "destructive", action1);
	cJSON_AddStringToObject(interface, "name", argv[1]);
	cJSON_AddStringToObject(interface, "cdb_name", argv[2]);
	cJSON_AddBoolToObject(interface, "adv_send_advert", atoi(argv[3]));
	cJSON_AddNumberToObject(interface, "max_rtr_interval", max_rtr_interval);
	cJSON_AddNumberToObject(interface, "min_rtr_interval", min_rtr_interval);
	cJSON_AddNumberToObject(interface, "adv_default_lifetime", adv_default_lifetime);
	cJSON_AddItemToObject(interface, "prefixes", prefixes);
	cJSON_AddBoolToObject(prefix1, "destructive", action2);
	cJSON_AddStringToObject(prefix1, "addr", argv[4]);
	if(!action2) {
		cJSON_AddNumberToObject(prefix1, "adv_autonomous", adv_autonomous);
		cJSON_AddNumberToObject(prefix1, "adv_on_link", adv_on_link);
	}
	cJSON_AddItemToArray(prefixes, prefix1);

	cJSON_AddBoolToObject(prefix2, "destructive", action3);
	cJSON_AddStringToObject(prefix2, "addr", "2020::1");
	if(!action3) {
		cJSON_AddNumberToObject(prefix2, "adv_autonomous", adv_autonomous);
		cJSON_AddNumberToObject(prefix2, "adv_on_link", adv_on_link);
	}
	cJSON_AddItemToArray(prefixes, prefix2);

	cJSON_AddBoolToObject(prefix3, "destructive", action4);
	cJSON_AddStringToObject(prefix3, "addr", "abcd::1");
	if(!action4) {
		cJSON_AddNumberToObject(prefix3, "adv_autonomous", adv_autonomous);
		cJSON_AddNumberToObject(prefix3, "adv_on_link", adv_on_link);
	}
	cJSON_AddItemToArray(prefixes, prefix3);

	char *out = cJSON_Print(ra_config);
	printf("%s\n%lu\n", out, strlen(out));
	printf("%lu\n", sendto(client_socket, out, strlen(out), 0, (struct sockaddr *)&remote, sizeof(remote)));

	cJSON_Delete(ra_config);
	free(out);
	close(client_socket);
	return 0;
}
