#ifndef UTILS_H
#define UTILS_H
#include <string>
#include <vector>
using namespace std;

template <typename Entry, typename Key>
int binary_search(const vector<Entry>& index, const Key& key) {
    int lo = 0, hi = (int)index.size() - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (index[mid].key == key) return mid;
        if (index[mid].key < key)  lo = mid + 1;
        else {hi = mid - 1;}
    }
    return -1;
}
template <typename Entry, typename Key>
int binary_insert_pos(const vector<Entry>& index, const Key& key) {
    int lo = 0, hi = (int)index.size();
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (index[mid].key < key) 
         {lo = mid + 1;}
        else{hi = mid;}
    }
    return lo;
}
string field_to_string(const char* field, int max_len);
#endif