#include "../include/Operations.h"
#include "../include/RecordStructures.h"
#include "../include/FileManager.h"
#include "../include/DoctorsManager.h"
#include "../include/AppointmentsManager.h"
#include "../include/PrimaryIndex.h"
#include "../include/SecondaryIndex.h"
#include <iostream>
using namespace std;

bool update_doctor_id(const string& old_id, const string& new_id) {
    if (new_id.empty() || new_id.size() > (size_t)DOC_ID_SIZE || new_id[0] == '*') {
        cout << "Error: new Doctor ID must be 1 to " << DOC_ID_SIZE << " characters and not start with '*'.\n";
        return false;
    }
    // 1. old_id must exist (P3)
    int offset = primary_search_doctor(old_id);
    if (offset == -1) { cout << "Error: Doctor ID '" << old_id << "' does not exist.\n"; return false; }
    if (old_id == new_id) return true;
    // 2. new_id must not exist (P3)
    if (primary_search_doctor(new_id) != -1) {
        cout << "Error: Doctor ID '" << new_id << "' already exists.\n";
        return false;
    }
    // 3. read Name and Specialty (P1)
    DoctorRecord d{};
    if (!get_doctor(old_id, d)) { cout << "Error: could not read the doctor record.\n"; return false; }
    string name = get_field(d.name, DOC_NAME_SIZE);
    string specialty = get_field(d.specialty, DOC_SPEC_SIZE);

    // 4. update the ID inside the record (P1's FileManager)
    if (!update_in_place(DOCTORS_FILE, offset, DOC_ID_OFF, DOC_ID_SIZE, new_id)) {
        cout << "Error: failed to update the Doctor ID in the data file.\n";
        return false;
    }
    // 5. primary index: change key with shifting (P3)
    primary_change_key(doctors_primary, old_id, new_id);
    // 6. secondary indexes of Doctors: replace the primary key in the chains (P4)
    secondary_change_primary_ref(doctors_name_index, name, old_id, new_id);
    secondary_change_primary_ref(doctors_specialty_index, specialty, old_id, new_id);
    // 7. appointments of this doctor + their secondary index (P2, which calls P4)
    update_appointments_doctor_id(old_id, new_id);
    return true;
}
