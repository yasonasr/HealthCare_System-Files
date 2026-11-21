#include "../include/LinkedList.h"
#include "../include/RecordStructures.h"
#include <fstream>
using namespace std;

namespace {
    struct LLNode {
        char key[ID_SIZE];
        int  next;
    };
    const int NODE_SIZE = (int)sizeof(LLNode);

    fstream open_links() {
        ifstream exists(LINKS_FILE, ios::binary);
        if (!exists) { ofstream create(LINKS_FILE, ios::binary); }      // indexes/ folder must already exist
        return fstream(LINKS_FILE, ios::in | ios::out | ios::binary);
    }
    bool read_node(fstream& f, int n, LLNode& node) {
        f.clear();
        f.seekg((streamoff)n * NODE_SIZE, ios::beg);
        f.read(reinterpret_cast<char*>(&node), NODE_SIZE);
        return f.gcount() == NODE_SIZE;
    }
    void write_node(fstream& f, int n, const LLNode& node) {
        f.clear();
        f.seekp((streamoff)n * NODE_SIZE, ios::beg);
        f.write(reinterpret_cast<const char*>(&node), NODE_SIZE);
        f.flush();
    }
}

int ll_insert(int head, const string& primary_key) {
    fstream f = open_links();
    if (!f) return -1;
    f.seekp(0, ios::end);
    int n = (int)(f.tellp() / NODE_SIZE);              // new node = appended at the end
    LLNode node;
    set_field(node.key, ID_SIZE, primary_key);
    node.next = head;
    write_node(f, n, node);
    return n;
}

int ll_remove(int head, const string& primary_key) {
    fstream f = open_links();
    int prev = -1, cur = head;
    while (cur != -1) {
        LLNode node;
        if (!read_node(f, cur, node)) break;
        if (get_field(node.key, ID_SIZE) == primary_key) {
            if (prev == -1) return node.next;           // removed the head
            LLNode p;
            read_node(f, prev, p);
            p.next = node.next;                         // bypass the node
            write_node(f, prev, p);
            return head;
        }
        prev = cur;
        cur = node.next;
    }
    return head;                                        // not found: unchanged
}

bool ll_contains(int head, const string& primary_key) {
    vector<string> keys = ll_traverse(head);
    return find(keys.begin(), keys.end(), primary_key) != keys.end();
}

vector<string> ll_traverse(int head) {
    vector<string> keys;
    fstream f = open_links();
    int cur = head;
    while (cur != -1) {
        LLNode node;
        if (!read_node(f, cur, node)) break;
        keys.push_back(get_field(node.key, ID_SIZE));
        cur = node.next;
    }
    return keys;
}

bool ll_replace_key(int head, const string& old_key, const string& new_key) {
    fstream f = open_links();
    bool replaced = false;
    int cur = head;
    while (cur != -1) {
        LLNode node;
        if (!read_node(f, cur, node)) break;
        if (get_field(node.key, ID_SIZE) == old_key) {
            set_field(node.key, ID_SIZE, new_key);
            write_node(f, cur, node);
            replaced = true;
        }
        cur = node.next;
    }
    return replaced;
}

void ll_reset() {
    ofstream out(LINKS_FILE, ios::binary | ios::trunc);
}
