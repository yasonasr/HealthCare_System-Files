#ifndef SECONDARY_INDEX_H
#define SECONDARY_INDEX_H

#include <string>
#include <vector>

using namespace std;

// Secondary index entry structure
// Each entry stores the secondary key and the head of its chain as an integer
struct SecondaryEntry {
    string key;   // Secondary key (Doctor Name, Doctor Specialty, or Doctor ID in Appointments)
    int head;     // Head pointer / record index in secondary_links.dat (-1 if chain is empty)
};

typedef vector<SecondaryEntry> SecondaryIndex;

// The three secondary indexes required by R1:
// 1. Doctor Name (Doctors file) mapping to chains of Doctor IDs.
// 2. Doctor Specialty (Doctors file) mapping to chains of Doctor IDs.
// 3. Doctor ID (Appointments file) mapping to chains of Appointment IDs.
extern SecondaryIndex doctors_name;
extern SecondaryIndex doctors_specialty;
extern SecondaryIndex appointments_doctor;

// Alias references for alternate naming conventions
extern SecondaryIndex& doctors_name_index;
extern SecondaryIndex& doctors_specialty_index;
extern SecondaryIndex& appointments_doctor_index;

// External LinkedList interface declarations (as required by R3)
int ll_insert(int head, const string& primary_key);
int ll_remove(int head, const string& primary_key);
vector<string> ll_traverse(int head);
bool ll_replace_key(int head, const string& old_key, const string& new_key);
void ll_reset();

// Secondary Index Operations (as required by R2)

// Inserts or finds the secondary key and prepends/links the primary key to its chain.
// Secondary index is maintained in ascending sorted order at all times using binary insert position.
bool secondary_insert(SecondaryIndex& index, const string& sec_key, const string& primary_key);

// Removes a primary key from the chain of the given secondary key.
// If the chain becomes empty, the secondary key entry is cleanly removed from the index.
bool secondary_remove(SecondaryIndex& index, const string& sec_key, const string& primary_key);

// Moves one record from one secondary key to another (remove from old_sec_key, insert into new_sec_key).
bool secondary_update(SecondaryIndex& index, const string& old_sec_key, const string& new_sec_key, const string& primary_key);

// Returns the list of primary keys in the chain for the given secondary key using binary search.
vector<string> secondary_search(const SecondaryIndex& index, const string& sec_key);

// Replaces a primary key inside a chain (using ll_replace_key) when a doctor ID changes.
bool secondary_change_primary_ref(SecondaryIndex& index, const string& sec_key, const string& old_primary, const string& new_primary);

// Overloads: updates the primary key reference in both Doctor Name and Doctor Specialty indexes when a Doctor ID changes.
bool secondary_change_primary_ref(const string& old_primary, const string& new_primary);
bool secondary_change_primary_ref(const string& doctor_name, const string& doctor_specialty, const string& old_primary, const string& new_primary);

// Rebuilds all three secondary indexes and secondary_links.dat at startup by calling ll_reset() and scanning data files.
int build_secondary_indexes(const string& doctors_path = "data/doctors.dat", const string& appointments_path = "data/appointments.dat");

// Individual builders for doctors and appointments data files
int build_doctors_secondary(const string& path = "data/doctors.dat");
int build_appointments_secondary(const string& path = "data/appointments.dat");

#endif // SECONDARY_INDEX_H
