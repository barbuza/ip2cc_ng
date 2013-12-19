enum node_type {
  nt_undefined = 0,
  nt_subtree = 1,
  nt_value = 2
};

struct node_s {
  enum node_type type;
  union {
    char *value;
    struct node_s *subtree;
  } ref;
};

typedef struct node_s node_t;

typedef unsigned char ip_t[4];


void free_tree(node_t *);

void parse_ip(const char *, ip_t);

void free_node(node_t *);

node_t *make_tree();

void add_ip(node_t *, const char *, const char *);

char *lookup(node_t *, const char *);

void write_tree(node_t *, size_t, FILE *);

void read_tree(node_t *, size_t, FILE *);
