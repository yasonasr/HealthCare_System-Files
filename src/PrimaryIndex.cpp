#include "../include/PrimaryIndex.h"
#include "../include/Utils.h"
#include <algorithm>
#include <fstream>
using namespace std;

PrimaryIndex doctors_primary;
PrimaryIndex appointments_primary;

bool primary_insert(PrimaryIndex& index, const string& key, int offset) {
    int pos = binary_insert_pos(index, key);
    if (pos < (int)index.size() && index[pos].key == key) return false; 
    PrimaryEntry e = { key, offset };
    index.insert(index.begin() + pos, e);                              
    return true;
}

bool primary_remove(PrimaryIndex& index, const string& key) {
    int pos = binary_search(index, key);
    if (pos == -1) return false;
    index.erase(index.begin() + pos);                                  
    return true;
}

bool primary_change_key(PrimaryIndex& index, const string& old_key,
                        const string& new_key) {
    int pos = binary_search(index, old_key);
    if (pos == -1) return false;
    if (old_key == new_key) return true;
    if (binary_search(index, new_key) != -1) return false;
    int offset = index[pos].offset;
    index.erase(index.begin() + pos);
    primary_insert(index, new_key, offset);
    return true;
}

int primary_search_doctor(const string& id) {
    int pos = binary_search(doctors_primary, id);
    return pos == -1 ? -1 : doctors_primary[pos].offset;
}

int primary_search_appointment(const string& id) {
    int pos = binary_search(appointments_primary, id);
    return pos == -1 ? -1 : appointments_primary[pos].offset;
}

static int build_from_file(PrimaryIndex& index, const string& path, int rec_size) {
    ifstream f(path.c_str(), ios::binary);
    if (!f) return -1;
    index.clear();

    int header;
    f.read(reinterpret_cast<char*>(&header), sizeof(int));  
    if (!f) return 0;                                       

    vector<char> buf(rec_size);
    int offset = HEADER_SIZE;
    while (f.read(buf.data(), rec_size)) {
        if (buf[0] != '*') {                              
            PrimaryEntry e = { field_to_string(buf.data(), ID_SIZE), offset };
            index.push_back(e);
        }
        offset += rec_size;
    }
    sort(index.begin(), index.end(),
              [](const PrimaryEntry& a, const PrimaryEntry& b) { return a.key < b.key; });
    return (int)index.size();
}

int build_doctors_primary(const string& path) {
    return build_from_file(doctors_primary, path, DOCTOR_REC_SIZE);
}

int build_appointments_primary(const string& path) {
    return build_from_file(appointments_primary, path, APPOINT_REC_SIZE);
}