
#include "test/print_safe_buffer.h"
#include <check.h>

/*
 * http://check.sourceforge.net/doc/check_html/check_3.html
 *
 * http://entrenchant.blogspot.com/2010/08/unit-testing-in-c.html
 */

START_TEST(test_decrement_lifetime)
{
	uint32_t lifetime = 10;
	decrement_lifetime(7, &lifetime);
	ck_assert_int_eq(lifetime, 3);
	decrement_lifetime(7, &lifetime);
	ck_assert_int_eq(lifetime, 0);
}
END_TEST

static struct Interface *iface = 0;

static void iface_setup(void)
{
	ck_assert_ptr_eq(0, iface);
	iface = readin_config("test/test1.conf");
	ck_assert_ptr_ne(0, iface);
}

static void iface_teardown(void)
{
	ck_assert_ptr_ne(0, iface);
	free_ifaces(iface);
	iface = 0;
}

START_TEST(test_add_ra_header)
{

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_ra_header(&sb, &iface->ra_header_info, iface->state_info.cease_adv);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", &buf);
#else
	unsigned char expected[] = {
	    0x86, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_prefix)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	add_ra_options_prefix(sbl, iface, iface->props.name, iface->AdvPrefixList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", &buf);
#else
	unsigned char expected[] = {
	    0x03, 0x04, 0x40, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
	    0xfe, 0x80, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x03, 0x04, 0x30, 0x80, 0x00, 0x00, 0x27, 0x10, 0x00, 0x00, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00,
	    0xfe, 0x80, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x03, 0x04, 0x40, 0xc0, 0x00, 0x01, 0x51, 0x80, 0x00, 0x00, 0x38, 0x40, 0x00, 0x00, 0x00, 0x00,
	    0xfe, 0x80, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_route)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	add_ra_options_route(sbl, iface, iface->AdvRouteList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", &buf);
#else
	unsigned char expected[] = {
	    0x18, 0x03, 0x30, 0x18, 0x00, 0x00, 0x27, 0x10, 0xfe, 0x80, 0x00, 0x0f, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x03, 0x28, 0x08, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x80, 0x00, 0x0f,
	    0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x03, 0x20, 0x00, 0x00, 0x00,
	    0x0b, 0xb8, 0xfe, 0x80, 0x00, 0x0f, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif
	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_rdnss)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	add_ra_options_rdnss(sbl, iface, iface->AdvRDNSSList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", &buf);
#else
	unsigned char expected[] = {
	    0x19, 0x07, 0x00, 0x00, 0x00, 0x00, 0x04, 0xd2, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x02, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_rdnss2)
{
	static struct Interface *iface = 0;
	iface = readin_config("test/test_rdnss.conf");
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	add_ra_options_rdnss(sbl, iface, iface->AdvRDNSSList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);
	free_ifaces(iface);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", &buf);
#else
	unsigned char expected[] = {
	    0x19, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d, 0x12, 0x34, 0x04, 0x23,
	    0xfe, 0xfe, 0x04, 0x93, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_dnssl)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	add_ra_options_dnssl(sbl, iface, iface->AdvDNSSLList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", &buf);
#else
	unsigned char expected[] = {
	    0x1f, 0x09, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe8, 0x06, 0x6f, 0x66, 0x66, 0x69, 0x63, 0x65, 0x06, 0x62, 0x72, 0x61,
	    0x6e, 0x63, 0x68, 0x07, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x06, 0x62, 0x72,
	    0x61, 0x6e, 0x63, 0x68, 0x07, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x07, 0x65,
	    0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x09, 0x00, 0x00,
	    0x00, 0x00, 0x04, 0x4b, 0x06, 0x6f, 0x66, 0x66, 0x69, 0x63, 0x65, 0x06, 0x62, 0x72, 0x61, 0x6e, 0x63, 0x68, 0x07,
	    0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x03, 0x6e, 0x65, 0x74, 0x00, 0x06, 0x62, 0x72, 0x61, 0x6e, 0x63, 0x68,
	    0x07, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x03, 0x6e, 0x65, 0x74, 0x00, 0x07, 0x65, 0x78, 0x61, 0x6d, 0x70,
	    0x6c, 0x65, 0x03, 0x6e, 0x65, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x04, 0x4c,
	    0x06, 0x6f, 0x66, 0x66, 0x69, 0x63, 0x65, 0x06, 0x62, 0x72, 0x61, 0x6e, 0x63, 0x68, 0x07, 0x65, 0x78, 0x61, 0x6d,
	    0x70, 0x6c, 0x65, 0x00, 0x06, 0x62, 0x72, 0x61, 0x6e, 0x63, 0x68, 0x07, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65,
	    0x00, 0x07, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x00,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_option_mtu)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_ra_option_mtu(&sb, iface->AdvLinkMTU);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", &buf);
#else
	unsigned char expected[] = {
	    0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0xd2,
	};

	ck_assert_int_eq(sb.used, sizeof(expected));
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sizeof(expected)));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_option_sllao)
{
	struct sllao sllao48 = {
	    {1, 2, 3, 4, 5, 6, 7, 8}, 48, 64, 1500,
	};

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_ra_option_sllao(&sb, &sllao48);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", &buf);
#else
	unsigned char expected48[] = {
	    0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	};

	ck_assert_int_eq(sizeof(expected48), sb.used);
	ck_assert_int_eq(0, memcmp(sb.buffer, expected48, sizeof(expected48)));
#endif

	safe_buffer_free(&sb);

	struct sllao sllao64 = {
	    {1, 2, 3, 4, 5, 6, 7, 8}, 64, 64, 1500,
	};

	sb = SAFE_BUFFER_INIT;
	add_ra_option_sllao(&sb, &sllao64);

#ifdef PRINT_SAFE_BUFFER
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", &buf);
#else
	unsigned char expected64[] = {
	    0x01, 0x02, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	ck_assert_int_eq(sizeof(expected64), sb.used);
	ck_assert_int_eq(0, memcmp(sb.buffer, expected64, sizeof(expected64)));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_option_lowpanco)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_ra_option_lowpanco(&sb, iface->AdvLowpanCoList);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", &buf);
#else
	unsigned char expected[] = {
	    0x22, 0x03, 0x32, 0x14, 0x00, 0x00, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};
	ck_assert_int_eq(sb.used, sizeof(expected));
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sizeof(expected)));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_option_abro)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_ra_option_abro(&sb, iface->AdvAbroList);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", &buf);
#else
	unsigned char expected[] = {
	    0x23, 0x03, 0x00, 0x0a, 0x00, 0x02, 0x00, 0x02, 0xfe, 0x80, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0xa2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	};

	ck_assert_int_eq(sb.used, sizeof(expected));
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sizeof(expected)));
#endif

	safe_buffer_free(&sb);
}
END_TEST

Suite *send_suite(void)
{
	TCase *tc_update = tcase_create("update");
	tcase_add_test(tc_update, test_decrement_lifetime);

	TCase *tc_build = tcase_create("build");
	tcase_add_unchecked_fixture(tc_build, iface_setup, iface_teardown);
	tcase_add_test(tc_build, test_add_ra_header);
	tcase_add_test(tc_build, test_add_ra_options_prefix);
	tcase_add_test(tc_build, test_add_ra_options_route);
	tcase_add_test(tc_build, test_add_ra_options_rdnss);
	tcase_add_test(tc_build, test_add_ra_options_rdnss2);
	tcase_add_test(tc_build, test_add_ra_options_dnssl);
	tcase_add_test(tc_build, test_add_ra_option_mtu);
	tcase_add_test(tc_build, test_add_ra_option_sllao);
	tcase_add_test(tc_build, test_add_ra_option_lowpanco);
	tcase_add_test(tc_build, test_add_ra_option_abro);

	Suite *s = suite_create("send");
	suite_add_tcase(s, tc_update);
	suite_add_tcase(s, tc_build);

	return s;
}
