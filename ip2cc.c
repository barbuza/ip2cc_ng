#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ip2cc.h"

void parse_ip(const char *raw_ip, ip_t ip) {
  size_t index = 0;
  while (*raw_ip) {
    if (*raw_ip == '.') {
      index++;
    } else {
      ip[index] = ip[index] * 10 + *raw_ip - '0';
    }
    raw_ip++;
  }
}

void free_node(node_t *node) {
  switch (node->type) {
    case nt_subtree:
      free_tree(node->ref.subtree);
      break;
    case nt_value:
      free(node->ref.value);
      break;
    default:
      break;
  }
}

void free_tree(node_t *tree)
{
  for (int i = 0; i < 256; i++) {
    free_node(tree + i);
  }
  free(tree);
}

node_t *make_tree()
{
  node_t *tree = malloc(sizeof(node_t) * 256);
  bzero(tree, sizeof(node_t) * 256);
  return tree;
}

void _add_ip(node_t *tree, ip_t ip, unsigned char step, const char *value)
{
  node_t *node = tree + ip[3 - step];
  if (node->type == nt_value) {
    printf("add intersection\n");
    exit(1);
  }
  if (! step) {
    node->type = nt_value;
    size_t len = strlen(value);
    node->ref.value = malloc(len + 1);
    strcpy(node->ref.value, value);
  } else {
    if (node->type == nt_undefined) {
      node->type = nt_subtree;
      node->ref.subtree = make_tree();
    }
    _add_ip(node->ref.subtree, ip, step - 1, value);
  }
}

void add_ip(node_t *tree, const char *raw_ip, const char *value)
{
  ip_t ip = {0};
  parse_ip(raw_ip, ip);
  _add_ip(tree, ip, 3, value);
}

char *_lookup(node_t *tree, ip_t ip, int step)
{
  node_t *node = tree + ip[3 - step];
  if (node->type == nt_value) {
    return node->ref.value;
  }
  if (step && node->type == nt_subtree) {
    return _lookup(node->ref.subtree, ip, step - 1);
  }
  return NULL;
}

char *lookup(node_t *tree, const char *raw_ip)
{
  ip_t ip = {0};
  parse_ip(raw_ip, ip);
  return _lookup(tree, ip, 3);
}

size_t tree_size(node_t *tree, size_t value_len)
{
  node_t node;
  size_t len = 256 * (2 + value_len);
  for (int i = 0; i < 256; i++) {
    node = tree[i];
    if (node.type == nt_subtree) {
      len += tree_size(node.ref.subtree, value_len);
    }
  }
  return len;
}

void dump_tree_data(node_t *tree, size_t value_len, size_t offset, char level, unsigned char *data)
{
  node_t node;
  unsigned char item_size;
  if (level) {
    item_size = 2 + value_len;
  } else {
    item_size = value_len;
  }
  size_t header_size = 256 * item_size;
  unsigned char *write_to;
  size_t end = offset + header_size;
  size_t subtree_size;
  for (int i = 0; i < 256; i++) {
    node = tree[i];
    write_to = data + offset + i * item_size;
    if (level) {
      switch (node.type) {
        case nt_value:
          *(write_to) = 0xff;
          *(write_to + 1) = 0xff;
          memcpy(write_to + 2, node.ref.value, value_len);
          break;
        case nt_undefined:
          *(write_to) = 0xff;
          *(write_to + 1) = 0xff;
          break;
        case nt_subtree:
          subtree_size = tree_size(node.ref.subtree, value_len);
          dump_tree_data(node.ref.subtree, value_len, end, level - 1, data);
          *(write_to) = (end & 0xff000000) >> 24;
          *(write_to + 1) = (end & 0xff0000) >> 16;
          *(write_to + 2) = (end & 0xff00) >> 8;
          *(write_to + 3) = end & 0xff;
          end += subtree_size;
          break;
      }
    } else if (node.type == nt_value) {
      memcpy(write_to, node.ref.value, value_len);
    }
  }
}

void write_tree(node_t *tree, size_t value_len, FILE *fp)
{
  size_t data_size = tree_size(tree, value_len);
  unsigned char *data = malloc(data_size);
  bzero(data, data_size);
  dump_tree_data(tree, value_len, 0, 3, data);
  fwrite(data, 1, data_size, fp);
  free(data);
}

void _read_tree(node_t *tree, size_t value_len, FILE *fp, size_t offset, unsigned char level, unsigned char *buf)
{
  node_t *node;
  size_t subtree_addr;
  unsigned char item_size;
  if (level) {
    item_size = value_len + 2;
  } else {
    item_size = value_len;
  }
  for (int i = 0; i < 256; i++) {
    fseek(fp, offset + i * item_size, SEEK_SET);
    fread(buf, 1, item_size, fp);
    node = tree + i;
    if (node->type != nt_undefined) {
      printf("read error\n");
      exit(2);
    }
    if (level) {
      if (buf[0] == 0xff && buf[1] == 0xff) {
        if (buf[2] != 0) {
          node->type = nt_value;
          node->ref.value = malloc(value_len + 1);
          bzero(node->ref.value, value_len + 1);
          memcpy(node->ref.value, buf + 2, value_len);
        }
      } else {
        node->type = nt_subtree;
        node->ref.subtree = make_tree();
        subtree_addr = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
        _read_tree(node->ref.subtree, value_len, fp, subtree_addr, level - 1, buf);
      }
    } else {
      if (buf[0]) {
        node->type = nt_value;
        node->ref.value = malloc(value_len + 1);
        bzero(node->ref.value, value_len + 1);
        memcpy(node->ref.value, buf, value_len);
      }
    }
  }
}

void read_tree(node_t *tree, size_t value_len, FILE *fp)
{
  unsigned char *buf = malloc(value_len + 2);
  _read_tree(tree, value_len, fp, 0, 3, buf);
  free(buf);
}

