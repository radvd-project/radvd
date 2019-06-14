#include "cjson_support.h"

#include "../radvd.h"
#include <stdio.h>
#include <string.h>

const struct in6_addr DFLT_Netmask64 = {
    .s6_addr = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}};

void apply_64_netmask(struct in6_addr *addr6)
{
	for (unsigned i = 0; i < 16; i++) {
		addr6->s6_addr[i] = addr6->s6_addr[i] & DFLT_Netmask64.s6_addr[i];
	}
}

struct Interface *process_command(char *json_message, struct Interface *ifaces)
{
	char *buffer = strdup(json_message);

	cJSON *cjson_ra_config = NULL;
	cJSON *cjson_action_iface = NULL;
	cJSON *cjson_interface = NULL;
	cJSON *cjson_prefix_ref = NULL;
	cJSON *cjson_prefix = NULL;
	cJSON *cjson_prefixes = NULL;
	cJSON *cjson_cdb_name = NULL;
	cJSON *cjson_prefix_addr = NULL;
	char *iface_cdb_name = NULL;
	char *addr_str = NULL;
	char *action_str = NULL;

	if (!(cjson_ra_config = cJSON_Parse(buffer))) {
		return ifaces;
	}
	if (cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(cjson_ra_config, "flush"))) {
		if (ifaces) {
			free_ifaces(ifaces);
		}
		cJSON_Delete(cjson_ra_config);
		return NULL;
	}
	if (!(cjson_interface = cJSON_GetObjectItemCaseSensitive(cjson_ra_config, "interface"))) {
		cJSON_Delete(cjson_ra_config);
		return ifaces;
	}
	if (!(cjson_cdb_name = cJSON_GetObjectItemCaseSensitive(cjson_interface, "cdb_name"))) {
		cJSON_Delete(cjson_ra_config);
		return ifaces;
	}
	if (!(iface_cdb_name = cJSON_GetStringValue(cjson_cdb_name))) {
		cJSON_Delete(cjson_ra_config);
		return ifaces;
	}

	cjson_action_iface = cJSON_GetObjectItemCaseSensitive(cjson_interface, "action");

	if (cjson_action_iface) {
		if (!(action_str = cJSON_GetStringValue(cjson_action_iface))) {
			cJSON_Delete(cjson_ra_config);
			return ifaces;
		}
	} else {
		action_str = "";
	}

	if (!strcmp(action_str, "DESTROY")) {
		ifaces = delete_iface_by_cdb_name(ifaces, iface_cdb_name);
		cJSON_Delete(cjson_ra_config);
		return ifaces;
	}

	struct Interface *iface = find_iface_by_cdb_name(ifaces, iface_cdb_name);

	if (!iface && !strcmp(action_str, "CREATE")) {
		iface = create_iface(iface_cdb_name);

		if (!ifaces) {
			ifaces = iface;
		} else {
			struct Interface *current = ifaces;
			while (current->next) {
				current = current->next;
			}
			current->next = iface;
		}
	}

	if (!iface) {
		cJSON_Delete(cjson_ra_config);
		return ifaces;
	}

	iface = update_iface(iface, cjson_interface);

	if (!(cjson_prefixes = cJSON_GetObjectItemCaseSensitive(cjson_interface, "prefixes"))) {
	}

	cJSON_ArrayForEach(cjson_prefix, cjson_prefixes)
	{

		if (!(cjson_prefix_addr = cJSON_GetObjectItemCaseSensitive(cjson_prefix, "addr"))) {
			continue;
		}
		if (!(addr_str = cJSON_GetStringValue(cjson_prefix_addr))) {
			continue;
		}

		struct in6_addr addr6;

		if (!inet_pton(AF_INET6, addr_str, &addr6)) {
			continue;
		}

		apply_64_netmask(&addr6);
		if ((cjson_prefix_ref = cJSON_GetObjectItemCaseSensitive(cjson_prefix, "ref"))) {
			if (cJSON_IsFalse(cjson_prefix_ref)) {
				iface->AdvPrefixList = delete_iface_prefix_by_addr(iface->AdvPrefixList, addr6);
				continue;
			}
		} else {
		}

		struct AdvPrefix *prefix = find_prefix_by_addr(iface->AdvPrefixList, addr6);

		if (!prefix) {
			prefix = create_prefix(addr6);
			if (!iface->AdvPrefixList) {
				iface->AdvPrefixList = prefix;
			} else {
				struct AdvPrefix *current = iface->AdvPrefixList;
				while (current->next) {
					current = current->next;
				}
				current->next = prefix;
			}
		}
		prefix = update_iface_prefix(prefix, cjson_prefix);
		if (cJSON_IsTrue(cjson_prefix_ref)) {
			prefix->ref++;
			char addr_str[INET6_ADDRSTRLEN];
			addrtostr(&addr6, addr_str, sizeof(addr_str));
		}
	}

	cJSON_Delete(cjson_ra_config);

	return ifaces;
}

char *load_json_file(const char *file_path)
{
	FILE *f = NULL;
	long len = 0;
	size_t th;
	char *data = NULL;

	/* open in read binary mode */
	f = fopen(file_path, "rb");
	if (f == NULL) {
		return NULL;
	}
	/* get the length */
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);

	data = (char *)malloc(len + 1);

	th = fread(data, 1, len, f);
	data[len] = '\0';
	fclose(f);

	cJSON *json = cJSON_Parse(data);
	char *out = cJSON_Print(json);

	cJSON_Delete(json);

	return out;
}