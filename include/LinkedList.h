#ifndef LINKED_LIST_H
#define LINKED_LIST_H
// Owner: P2 - chains of Primary Keys stored in indexes/secondary_links.dat.
// Node = Primary Key (char[15]) + next (int node number, -1 = end). A chain is identified by its head.
#include <string>
#include <vector>
using namespace std;

int ll_insert(int head, const string& primary_key);    // adds at the front, returns new head
int ll_remove(int head, const string& primary_key);    // returns new head (-1 if chain empty)
bool ll_contains(int head, const string& primary_key);
vector<string> ll_traverse(int head);                             // all Primary Keys of the chain
bool ll_replace_key(int head, const string& old_key, const string& new_key);
void ll_reset();                                        // empties the file (startup rebuild)

#endif
