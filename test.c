#include <stdio.h>
#include <CUnit/Basic.h>

#include "ip2cc.h"


void test_parse_ip()
{
  ip2cc_ip_t ip = {0};
  ip2cc_parse_ip("192.168.0.1", ip);
  CU_ASSERT_EQUAL_FATAL(ip[0], 192);
  CU_ASSERT_EQUAL_FATAL(ip[1], 168);
  CU_ASSERT_EQUAL_FATAL(ip[2], 0);
  CU_ASSERT_EQUAL_FATAL(ip[3], 1);
}

void test_store_lookup()
{
  ip2cc_tree_t tree = ip2cc_make_tree();
  CU_ASSERT_PTR_NOT_NULL_FATAL(tree);
  ip2cc_store(tree, "192.168.0.1", "foo");
  ip2cc_store(tree, "127.0.0.1", "bar");
  CU_ASSERT_STRING_EQUAL_FATAL("foo", ip2cc_lookup(tree, "192.168.0.1"));
  CU_ASSERT_STRING_EQUAL_FATAL("bar", ip2cc_lookup(tree, "127.0.0.1"));
  ip2cc_free(tree);
}

void test_read_write()
{
  ip2cc_tree_t tree = ip2cc_make_tree();
  CU_ASSERT_PTR_NOT_NULL_FATAL(tree);
  ip2cc_store(tree, "192.168.0.1", "foo");
  ip2cc_store(tree, "127.0.0.1", "bar");
  FILE *fp = fopen("test.db", "w");
  CU_ASSERT_PTR_NOT_NULL_FATAL(fp);
  CU_ASSERT_EQUAL_FATAL(ip2cc_write_tree(tree, 3, fp), 7936);
  fclose(fp);
  ip2cc_free(tree);
  fp = fopen("test.db", "r");
  CU_ASSERT_PTR_NOT_NULL_FATAL(fp);
  tree = ip2cc_make_tree();
  CU_ASSERT_PTR_NOT_NULL_FATAL(tree);
  CU_ASSERT_FATAL(0 == ip2cc_read_tree(tree, 3, fp));
  fclose(fp);
  CU_ASSERT_STRING_EQUAL_FATAL("foo", ip2cc_lookup(tree, "192.168.0.1"));
  CU_ASSERT_STRING_EQUAL_FATAL("bar", ip2cc_lookup(tree, "127.0.0.1"));
  ip2cc_free(tree);
}

int main(int argc, char *argv[])
{
  CU_initialize_registry();
  CU_pSuite suite = CU_add_suite("ip2cc tests", NULL, NULL);
  CU_add_test(suite, "test parse ip", test_parse_ip);
  CU_add_test(suite, "test store lookup", test_store_lookup);
  CU_add_test(suite, "test read write", test_read_write);
  return CU_basic_run_tests();
}
