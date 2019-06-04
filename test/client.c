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

#include <cjson/cJSON.h>

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

	FILE *f = NULL;
	long len = 0;
	char *data = NULL;

	/* open in read binary mode */
	f = fopen(argv[1], "rb");
	/* get the length */
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);

	data = (char *)malloc(len + 1);

	fread(data, 1, len, f);
	data[len] = '\0';
	fclose(f);

	char *out = NULL;
	cJSON *json = NULL;

	json = cJSON_Parse(data);

	if (!json) {
		printf("Error before: [%s]\n", cJSON_GetErrorPtr());
		exit(1);
	}

	out = cJSON_Print(json);
	cJSON_Delete(json);
	printf("%s\n", out);

	printf("%lu\n", sendto(client_socket, out, strlen(out), 0, (struct sockaddr *)&remote, sizeof(remote)));

	free(out);
	close(client_socket);
	return 0;
}
