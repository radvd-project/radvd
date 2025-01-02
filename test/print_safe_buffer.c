
#include "print_safe_buffer.h"
#include "../config.h"
#include "../radvd.h"

#include <stdio.h>

void print_safe_buffer(struct safe_buffer const *sb)
{
	char buf[4096];
	snprint_safe_buffer(buf, sizeof(buf), sb);
	printf("%s", buf);
}

size_t snprint_safe_buffer(char *s, size_t size, struct safe_buffer const *sb)
{
	size_t count = 0;
	int n;

	n = snprintf((s + count), (size - count), "unsigned char expected[] = { /* sb.allocated = %ld, sb.used = %ld */", sb->allocated, sb->used);
	if (n < 0 || n >= size - count) {
		return count;
	}
	count += n;

	char* nextline = "\n\t";
	char* nextbyte = " ";
	for (size_t i = 0; i < sb->used; ++i) {
		char* nextspace = (i % 8 == 0) ? nextline : nextbyte;
		n = snprintf((s + count), (size - count), "%s0x%02x,", nextspace, sb->buffer[i]);
		if (n < 0 || n >= size - count) {
			return count;
		}
		count += n;
	}
	/* Do not remove the final byte's comma. Only JSON requires the comma is
	 * removed, and this is not JSON. */
	n = snprintf((s + count), (size - count), "\n};\n");
	if (n < 0 || n >= size - count) {
		return count;
	}
	count += n;
	return count;
}
