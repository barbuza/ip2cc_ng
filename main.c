#include <stdlib.h>
#include <stdio.h>
#include "ip2cc.h"

int main(int argc, const char * argv[])
{
  const char *filename = "test_db";
  if (argc == 2) {
    filename = argv[1];
  }
  node_t *tree = make_tree();
  printf("store 10.0.0.1 foo\n");
  printf("store 127.0.0.1 bar\n");
  printf("store 192.168.0.1 spam\n");
  add_ip(tree, "10.0.0.1", "foo");
  add_ip(tree, "127.0.0.1", "bar");
  add_ip(tree, "192.168.0.1", "spam");
  printf("writing to %s\n", filename);
  FILE *fp = fopen(filename, "w");
  if (! fp) {
    printf("failed to open db\n");
    exit(3);
  }
  write_tree(tree, 4, fp);
  fclose(fp);
  free_tree(tree);
  printf("reading %s\n", filename);
  fp = fopen(filename, "r");
  if (! fp) {
    printf("failed to open db\n");
    exit(4);
  }
  tree = make_tree();
  read_tree(tree, 4, fp);
  fclose(fp);
  printf("lookup 10.0.0.1 %s\n", lookup(tree, "10.0.0.1"));
  printf("lookup 127.0.0.1 %s\n", lookup(tree, "127.0.0.1"));
  printf("lookup 192.168.0.1 %s\n", lookup(tree, "192.168.0.1"));
  free_tree(tree);
  return 0;
}
