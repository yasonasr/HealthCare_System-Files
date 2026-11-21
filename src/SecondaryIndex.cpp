#include "../include/SecondaryIndex.h"
#include "../include/LinkedList.h"
#include "../include/Utils.h"
#include <fstream>
using namespace std;

SecondaryIndex doctors_name_index;
SecondaryIndex doctors_specialty_index;
SecondaryIndex appointments_doctor_index;

bool secondary_insert(SecondaryIndex& index, const string& sec_key, const string& primary_key) {
    if (sec_key.empty() || primary_key.empty()) return false;
    int pos = binary_search(index, sec_key);
    if (pos == -1) {                                        // new secondary key, keep ascending order
        SecondaryEntry e = { sec_key, -1 };
        pos = binary_insert_pos(index, sec_key);
        index.insert(index.begin() + pos, e);
    }
    int head = ll_insert(index[pos].head, primary_key);
    if (head < 0) return false;
    index[pos].head = head;
    return true;
}

bool secondary_remove(SecondaryIndex& index, const string& sec_key, const string& primary_key) {
    int pos = binary_search(index, sec_key);
    if (pos == -1 || !ll_contains(index[pos].head, primary_key)) return false;
    index[pos].head = ll_remove(index[pos].head, primary_key);
    if (index[pos].head == -1) index.erase(index.begin() + pos);   // empty chain -> remove entry
    return true;
}

bool secondary_update(SecondaryIndex& index, const string& old_sec_key,
                      const string& new_sec_key, const string& primary_key) {
    if (old_sec_key == new_sec_key) return true;
    if (!secondary_remove(index, old_sec_key, primary_key)) return false;
    return secondary_insert(index, new_sec_key, primary_key);
}

vector<string> secondary_search(const SecondaryIndex& index, const string& sec_key) {
    int pos = binary_search(index, sec_key);
    if (pos == -1) return vector<string>();
    return ll_traverse(index[pos].head);
}

bool secondary_change_primary_ref(SecondaryIndex& index, const string& sec_key,
                                  const string& old_primary, const string& new_primary) {
    int pos = binary_search(index, sec_key);
    if (pos == -1) return false;
    return ll_replace_key(index[pos].head, old_primary, new_primary);
}

bool build_secondary_indexes(const string& doctors_path, const string& appointments_path) {
    ifstream df(doctors_path.c_str(), ios::binary);
    ifstream af(appointments_path.c_str(), ios::binary);
    if (!df || !af) return false;

    ll_reset();
    doctors_name_index.clear();
    doctors_specialty_index.clear();
    appointments_doctor_index.clear();

    int header;
    df.read(reinterpret_cast<char*>(&header), sizeof(int));
    DoctorRecord d;
    while (df.read(reinterpret_cast<char*>(&d), DOCTOR_RECORD_SIZE)) {
        if (d.id[0] == '*') continue;                       // deleted record
        string id = get_field(d.id, DOC_ID_SIZE);
        secondary_insert(doctors_name_index, get_field(d.name, DOC_NAME_SIZE), id);
        secondary_insert(doctors_specialty_index, get_field(d.specialty, DOC_SPEC_SIZE), id);
    }
    af.read(reinterpret_cast<char*>(&header), sizeof(int));
    AppointmentRecord a;
    while (af.read(reinterpret_cast<char*>(&a), APPT_RECORD_SIZE)) {
        if (a.id[0] == '*') continue;
        secondary_insert(appointments_doctor_index, get_field(a.doctor_id, APPT_DOCID_SIZE),
                         get_field(a.id, APPT_ID_SIZE));
    }
    return true;
}
