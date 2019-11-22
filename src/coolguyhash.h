#include "util.h"
#include "node.h"

#ifndef COOLGUYHASH_H
#define COOLGUYHASH_H

void hash_init();
Node* lookup_or_insert(int level, Node* nw, Node* ne, Node* sw, Node* se);

extern Node* single_alive;
extern Node* single_dead;

#endif