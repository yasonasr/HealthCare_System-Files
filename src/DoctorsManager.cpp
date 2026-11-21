#include "../include/DoctorsManager.h"
#include "../include/FileManager.h"
#include "../include/PrimaryIndex.h"
#include "../include/SecondaryIndex.h"
#include <iostream>
using namespace std;

static bool fits(const string& s, int size) { return !s.empty() && s.size() <= (size_t)size; }

bool create_doctors_file() { return init_file(DOCTORS_FILE); }

bool get_doctor(const string& id, DoctorRecord& out) {
    int offset = primary_search_doctor(id);
    if (offset < 0) return false;
    return read_record(DOCTORS_FILE, offset, reinterpret_cast<char*>(&out), DOCTOR_RECORD_SIZE)
           && out.id[0] != '*';
}

bool add_doctor(const string& id, const string& name, const string& specialty, const string& address) {
    if (!fits(id, DOC_ID_SIZE) || !fits(name, DOC_NAME_SIZE) || !fits(specialty, DOC_SPEC_SIZE) ||
        address.size() > (size_t)DOC_ADDR_SIZE || id[0] == '*') {
        cout << "Error: invalid input or field too long.\n";
        return false;
    }
    if (primary_search_doctor(id) != -1) {                                    // prevent duplicate Doctor ID
        cout << "Error: Doctor ID '" << id << "' already exists.\n";
        return false;
    }
    DoctorRecord rec{};
    set_field(rec.id, DOC_ID_SIZE, id);
    set_field(rec.name, DOC_NAME_SIZE, name);
    set_field(rec.specialty, DOC_SPEC_SIZE, specialty);
    set_field(rec.address, DOC_ADDR_SIZE, address);

    int offset = add_record(DOCTORS_FILE, reinterpret_cast<char*>(&rec), DOCTOR_RECORD_SIZE);   // 1
    if (offset < 0) { cout << "Error: failed to write the doctor record.\n"; return false; }
    primary_insert(doctors_primary, id, offset);                                                // 2
    secondary_insert(doctors_name_index, name, id);                                             // 3
    secondary_insert(doctors_specialty_index, specialty, id);
    return true;
}

bool update_doctor_name(const string& id, const string& new_name) {
    if (!fits(new_name, DOC_NAME_SIZE)) { cout << "Error: invalid name.\n"; return false; }
    DoctorRecord rec{};
    if (!get_doctor(id, rec)) { cout << "Warning: Doctor ID '" << id << "' not found.\n"; return false; }
    string old_name = get_field(rec.name, DOC_NAME_SIZE);

    if (!update_in_place(DOCTORS_FILE, primary_search_doctor(id), DOC_NAME_OFF, DOC_NAME_SIZE, new_name)) {  // 1
        cout << "Error: failed to update the name.\n";
        return false;
    }
    secondary_update(doctors_name_index, old_name, new_name, id);                                            // 2
    return true;
}

bool delete_doctor(const string& id) {
    DoctorRecord rec{};
    if (!get_doctor(id, rec)) { cout << "Warning: Doctor ID '" << id << "' not found.\n"; return false; }
    if (!secondary_search(appointments_doctor_index, id).empty()) {            // reject if appointments exist
        cout << "Error: cannot delete doctor '" << id << "' because the doctor still has appointments.\n";
        return false;
    }
    string name = get_field(rec.name, DOC_NAME_SIZE);                          // 1. read the secondary keys
    string specialty = get_field(rec.specialty, DOC_SPEC_SIZE);
    int offset = primary_search_doctor(id);

    secondary_remove(doctors_name_index, name, id);                            // 2
    secondary_remove(doctors_specialty_index, specialty, id);
    primary_remove(doctors_primary, id);                                       // 3
    if (!delete_record(DOCTORS_FILE, offset)) {                                // 4
        cout << "Error: failed to delete the record from the data file.\n";
        return false;
    }
    return true;
}

bool print_doctor(const string& id) {
    DoctorRecord rec{};
    if (!get_doctor(id, rec)) { cout << "Warning: Doctor ID '" << id << "' not found.\n"; return false; }
    cout << "Doctor ID : " << get_field(rec.id, DOC_ID_SIZE) << '\n'
         << "Name      : " << get_field(rec.name, DOC_NAME_SIZE) << '\n'
         << "Specialty : " << get_field(rec.specialty, DOC_SPEC_SIZE) << '\n'
         << "Address   : " << get_field(rec.address, DOC_ADDR_SIZE) << '\n';
    return true;
}
