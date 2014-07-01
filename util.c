/*
 *
 *   Authors:
 *    Lars Fenneberg		<lf@elemental.net>
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s),
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <reubenhwk@gmail.com>.
 *
 */

#include "config.h"
#include "includes.h"
#include "radvd.h"

#ifdef UNIT_TEST

#include <check.h>

/*
 * http://check.sourceforge.net/doc/check_html/check_3.html
 *
 * http://entrenchant.blogspot.com/2010/08/unit-testing-in-c.html
 */

START_TEST (test_safe_buffer)
{
	struct safe_buffer sb = SAFE_BUFFER_INIT;
	ck_assert_ptr_eq(0, sb.buffer);
	ck_assert_int_eq(0, sb.allocated);
	ck_assert_int_eq(0, sb.used);
	ck_assert_int_eq(0, sb.should_free);
	safe_buffer_free(&sb);

	struct safe_buffer * sbptr = new_safe_buffer();
	ck_assert_ptr_eq(0, sbptr->buffer);
	ck_assert_int_eq(0, sbptr->allocated);
	ck_assert_int_eq(0, sbptr->used);
	ck_assert_int_ne(0, sbptr->should_free);
	safe_buffer_free(sbptr);
}
END_TEST

START_TEST (test_safe_buffer_append)
{
	struct safe_buffer sb = SAFE_BUFFER_INIT;
	char array[] = {"This is a test"};
	safe_buffer_append(&sb, array, sizeof(array));
	ck_assert_str_eq(sb.buffer, array);
	ck_assert_int_eq(sb.used, sizeof(array));
	safe_buffer_free(&sb);
}
END_TEST

START_TEST (test_safe_buffer_append2)
{
	struct safe_buffer sb = SAFE_BUFFER_INIT;
	char array[] = {"This is a test"};
	safe_buffer_append(&sb, array, sizeof(array));
	safe_buffer_append(&sb, array, sizeof(array));
	ck_assert_str_eq(sb.buffer, array);
	ck_assert_str_eq(sb.buffer + sizeof(array), array);
	ck_assert_int_eq(sb.used, 2*sizeof(array));
	
	safe_buffer_free(&sb);
}
END_TEST

START_TEST (test_safe_buffer_pad)
{
	struct safe_buffer sb = SAFE_BUFFER_INIT;
	char array[] = {"This is a test"};
	safe_buffer_append(&sb, array, sizeof(array));
	safe_buffer_pad(&sb, 10);
	ck_assert_str_eq(sb.buffer, array);
	ck_assert_int_eq(sb.used, 10+sizeof(array));
	
	safe_buffer_free(&sb);
}
END_TEST

START_TEST (test_addrtostr)
{
	char buffer[INET6_ADDRSTRLEN] = {""};
	struct in6_addr addr = {
		{
			0xfe, 0x80, 0xfe, 0x80,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0xff, 0x00, 0x12, 0x34,
		},
	};
	addrtostr(&addr, buffer, sizeof(buffer));
	ck_assert_str_eq(buffer, "fe80:fe80::ff00:1234");
}
END_TEST

START_TEST (test_addrtostr_overflow)
{
	char buffer[18] = {""};
	struct in6_addr addr = {
		{
			0xfe, 0x80, 0xfe, 0x80,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0xff, 0x00, 0x12, 0x34,
		},
	};
	addrtostr(&addr, buffer, sizeof(buffer));
	ck_assert_str_eq(buffer, "[invalid address]");
}
END_TEST

START_TEST (test_strdupf)
{
	char * str = strdupf("%d %s %zu %c %% char", 1234, "str", (size_t)10, 'c');

	ck_assert_str_eq(str, "1234 str 10 c % char");

	free(str);
}
END_TEST

START_TEST (test_readn)
{
	int fd = open("/dev/zero", O_RDONLY);

	ck_assert_int_ne(fd, -1);

	char buffer[10000];

	memset(buffer, 1, sizeof(buffer));

	int count = readn(fd, buffer, sizeof(buffer));

	ck_assert_int_eq(count, 10000);
	
	for (int i = 0; i < sizeof(buffer); ++i) {
		ck_assert_int_eq(buffer[i], 0);
	}
}
END_TEST

START_TEST (test_writen)
{
	int fd = open("/dev/null", O_WRONLY);

	ck_assert_int_ne(fd, -1);

	char buffer[10000];

	memset(buffer, 1, sizeof(buffer));

	int count = writen(fd, buffer, sizeof(buffer));

	ck_assert_int_eq(count, 10000);
}
END_TEST

START_TEST (test_check_dnssl_presence)
{
	struct Interface * ifaces = readin_config("test/test1.conf");
	ck_assert_ptr_ne(0, ifaces);

	int rc = check_dnssl_presence(ifaces->AdvDNSSLList, "example.com");
	ck_assert_int_ne(0, rc);

	rc = check_dnssl_presence(ifaces->AdvDNSSLList, "office.branch.example.net");
	ck_assert_int_ne(0, rc);

	rc = check_dnssl_presence(ifaces->AdvDNSSLList, "example.au");
	ck_assert_int_eq(0, rc);

	free_ifaces(ifaces);
}
END_TEST

START_TEST (test_check_rdnss_presence)
{
	struct Interface * ifaces = readin_config("test/test1.conf");
	ck_assert_ptr_ne(0, ifaces);

	struct in6_addr addr;
	int rc;

	/* The next three should be found */
	addr = (struct in6_addr){ 0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
	rc = check_rdnss_presence(ifaces->AdvRDNSSList, &addr);
	ck_assert_int_ne(0, rc);

	addr = (struct in6_addr){ 0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 };
	rc = check_rdnss_presence(ifaces->AdvRDNSSList, &addr);
	ck_assert_int_ne(0, rc);

	addr = (struct in6_addr){ 0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3 };
	rc = check_rdnss_presence(ifaces->AdvRDNSSList, &addr);
	ck_assert_int_ne(0, rc);

	/* The next one should *not* be found */
	addr = (struct in6_addr){ 0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6 };
	rc = check_rdnss_presence(ifaces->AdvRDNSSList, &addr);
	ck_assert_int_eq(0, rc);

	free_ifaces(ifaces);
}
END_TEST

START_TEST (test_rand_between)
{
	int const RAND_TEST_MAX = 1000;
	for (int i = 0; i < RAND_TEST_MAX; ++i) {
		double const min = -1;
		double const max = 1;
		double d = rand_between(min, max);
		ck_assert(min <= d);
		ck_assert(max >= d);
	}
	for (int i = 0; i < RAND_TEST_MAX; ++i) {
		double const min = -1000;
		double const max = 1000;
		double d = rand_between(min, max);
		ck_assert(min <= d);
		ck_assert(max >= d);
	}
	for (int i = 0; i < RAND_TEST_MAX; ++i) {
		double const min = 100;
		double const max = 200;
		double d = rand_between(min, max);
		ck_assert(min <= d);
		ck_assert(max >= d);
	}
}
END_TEST

Suite * util_suite(void)
{
	TCase * tc_safe_buffer = tcase_create("safe buffer");
	tcase_add_test(tc_safe_buffer, test_safe_buffer);
	tcase_add_test(tc_safe_buffer, test_safe_buffer_append);
	tcase_add_test(tc_safe_buffer, test_safe_buffer_append2);
	tcase_add_test(tc_safe_buffer, test_safe_buffer_pad);

	TCase * tc_str = tcase_create("str");
	tcase_add_test(tc_str, test_addrtostr);
	tcase_add_test(tc_str, test_addrtostr_overflow);
	tcase_add_test(tc_str, test_strdupf);

	TCase * tc_ion = tcase_create("ion");
	tcase_add_test(tc_ion, test_readn);
	tcase_add_test(tc_ion, test_writen);

	TCase * tc_misc = tcase_create("misc");
	tcase_add_test(tc_misc, test_rand_between);
	tcase_add_test(tc_misc, test_check_dnssl_presence);
	tcase_add_test(tc_misc, test_check_rdnss_presence);

	Suite *s = suite_create("util");
	suite_add_tcase(s, tc_safe_buffer);
	suite_add_tcase(s, tc_str);
	suite_add_tcase(s, tc_ion);
	suite_add_tcase(s, tc_misc);

	return s;	
}
#endif /* UNIT_TEST */

struct safe_buffer * new_safe_buffer(void)
{
	struct safe_buffer * sb = malloc(sizeof(struct safe_buffer));
	*sb = SAFE_BUFFER_INIT;
	sb->should_free = 1;
	return sb;
}

void safe_buffer_free(struct safe_buffer * sb)
{
	if (sb->buffer) {
		free(sb->buffer);
	}

	if (sb->should_free) {
		free(sb);
	}
}

size_t safe_buffer_pad(struct safe_buffer * sb, size_t count)
{
	size_t rc = 0;
	unsigned char zero = 0;

	while (count--) {
		rc += safe_buffer_append(sb, &zero, 1);
	}

	return rc;
}

size_t safe_buffer_append(struct safe_buffer * sb, void const * v, size_t count)
{
	if (sb) {
		unsigned const char * m = (unsigned const char *)v;
		if (sb->allocated <= sb->used + count) {
			sb->allocated = sb->used + count + MSG_SIZE_SEND;
			sb->buffer = realloc(sb->buffer, sb->allocated);
		}
		memcpy(&sb->buffer[sb->used], m, count);
		sb->used += count;

		if (sb->used >= MSG_SIZE_SEND) {
			flog(LOG_ERR, "Too many prefixes, routes, rdnss or dnssl to fit in buffer.  Exiting.");
			exit(1);
		}
	}

	return count;
}

__attribute__ ((format(printf, 1, 2)))
char * strdupf(char const * format, ...)
{
	va_list va;
	va_start(va, format);
	char * strp = 0;
	int rc = vasprintf(&strp, format, va);
	if (rc == -1 || !strp) {
		flog(LOG_ERR, "vasprintf failed: %s", strerror(errno));
		exit(-1);
	}
	va_end(va);

	return strp;
}

double rand_between(double lower, double upper)
{
	return ((upper - lower) / (RAND_MAX + 1.0) * rand() + lower);
}

/* This assumes that str is not null and str_size > 0 */
void addrtostr(struct in6_addr *addr, char *str, size_t str_size)
{
	const char *res;

	res = inet_ntop(AF_INET6, (void *)addr, str, str_size);

	if (res == NULL) {
		flog(LOG_ERR, "addrtostr: inet_ntop: %s", strerror(errno));
		strncpy(str, "[invalid address]", str_size);
		str[str_size - 1] = '\0';
	}
}

/* Check if an in6_addr exists in the rdnss list */
int check_rdnss_presence(struct AdvRDNSS *rdnss, struct in6_addr *addr)
{
	while (rdnss) {
		if (!memcmp(&rdnss->AdvRDNSSAddr1, addr, sizeof(struct in6_addr))
		    || !memcmp(&rdnss->AdvRDNSSAddr2, addr, sizeof(struct in6_addr))
		    || !memcmp(&rdnss->AdvRDNSSAddr3, addr, sizeof(struct in6_addr)))
			return 1;	/* rdnss address found in the list */
		rdnss = rdnss->next;
	}
	return 0;
}

/* Check if a suffix exists in the dnssl list */
int check_dnssl_presence(struct AdvDNSSL *dnssl, const char *suffix)
{
	while (dnssl) {
		for (int i = 0; i < dnssl->AdvDNSSLNumber; ++i) {
			if (0 == strcmp(dnssl->AdvDNSSLSuffixes[i], suffix))
				return 1;	/* suffix found in the list */
		}
		dnssl = dnssl->next;
	}
	return 0;
}

/* Like read(), but retries in case of partial read */
ssize_t readn(int fd, void *buf, size_t count)
{
	size_t n = 0;
	while (count > 0) {
		int r = read(fd, buf, count);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			return r;
		}
		if (r == 0)
			return n;
		buf = (char *)buf + r;
		count -= r;
		n += r;
	}
	return n;
}

/* Like write(), but retries in case of partial write */
ssize_t writen(int fd, const void *buf, size_t count)
{
	size_t n = 0;
	while (count > 0) {
		int r = write(fd, buf, count);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			return r;
		}
		if (r == 0)
			return n;
		buf = (const char *)buf + r;
		count -= r;
		n += r;
	}
	return n;
}
