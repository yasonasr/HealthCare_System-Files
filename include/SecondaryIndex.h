#ifndef SECONDARY_INDEX_H
#define SECONDARY_INDEX_H
// Owner: P4 - the ONLY module that modifies Secondary Indexes.
// Entry = Secondary Key + head (int) of its chain in secondary_links.dat (chains handled by P2's LinkedList).
#include <string>
#include <vector>
#include "RecordStructures.h"
using namespace std;

struct SecondaryEntry {
    string key;   // Doctor Name / Doctor Specialty / Doctor ID (appointments)
    int head;     // first node of the chain
};
typedef vector<SecondaryEntry> SecondaryIndex;       // sorted ascending by key

extern SecondaryIndex doctors_name_index;            // Doctor Name      -> Doctor IDs
extern SecondaryIndex doctors_specialty_index;       // Doctor Specialty -> Doctor IDs
extern SecondaryIndex appointments_doctor_index;     // Doctor ID        -> Appointment IDs

bool secondary_insert(SecondaryIndex& index, const string& sec_key, const string& primary_key);
bool secondary_remove(SecondaryIndex& index, const string& sec_key, const string& primary_key);
bool secondary_update(SecondaryIndex& index, const string& old_sec_key,const string& new_sec_key, const string& primary_key);
vector<string> secondary_search(const SecondaryIndex& index, const string& sec_key);
bool secondary_change_primary_ref(SecondaryIndex& index, const string& sec_key,const string& old_primary, const string& new_primary);

// Startup: ll_reset(), then scan both data files and rebuild the three indexes.
bool build_secondary_indexes(const string& doctors_path = DOCTORS_FILE,const string& appointments_path = APPOINTMENTS_FILE);
#endif
