#pragma once

#include "../includes.h"

struct Interface *process_command(char *json_message, struct Interface *ifaces);
char *load_json_file(const char *file_path);