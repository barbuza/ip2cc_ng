#include <stdlib.h>
#include <stdio.h>
#include "ip2cc.h"

int main(int argc, const char * argv[])
{
  const char *filename = "test_db";
  if (argc == 2) {
    filename = argv[1];
  }
  ip2cc_tree_t tree = ip2cc_make_tree();
  printf("store 10.0.0.1 foo\n");
  printf("store 127.0.0.1 bar\n");
  printf("store 192.168.0.1 spam\n");
  ip2cc_store(tree, "10.0.0.1", "foo");
  ip2cc_store(tree, "127.0.0.1", "bar");
  ip2cc_store(tree, "192.168.0.1", "spam");
  printf("writing to %s\n", filename);
  FILE *fp = fopen(filename, "w");
  if (! fp) {
    printf("failed to open db\n");
    exit(3);
  }
  ip2cc_write_tree(tree, 4, fp);
  fclose(fp);
  ip2cc_free(tree);
  printf("reading %s\n", filename);
  fp = fopen(filename, "r");
  if (! fp) {
    printf("failed to open db\n");
    exit(4);
  }
  tree = ip2cc_make_tree();
  ip2cc_read_tree(tree, 4, fp);
  fclose(fp);
  printf("lookup 10.0.0.1 %s\n", ip2cc_lookup(tree, "10.0.0.1"));
  printf("lookup 127.0.0.1 %s\n", ip2cc_lookup(tree, "127.0.0.1"));
  printf("lookup 192.168.0.1 %s\n", ip2cc_lookup(tree, "192.168.0.1"));
  ip2cc_free(tree);
  return 0;
}
