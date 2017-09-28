#ifndef __LLIST_H__
#define __LLIST_H__

#include <stdint.h>
#include "heap.h"

void add_node(bin_t *bin, node_t *node);
void remove_node(bin_t *bin, node_t *node);

node_t *get_best_fit(bin_t *list, size_t size);
node_t *get_last_node(bin_t *list);

node_t *next(node_t *current);
node_t *prev(node_t *current);

#endif /* __LLIST_H__ */
