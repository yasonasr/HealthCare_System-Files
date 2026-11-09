#include "DoctorsManager.h"
#include "FileManager.h"
#include "PrimaryIndex.h"
#include "SecondaryIndex.h"

#include <iostream>

using namespace std;

bool create_doctors_file() {
    return init_file(DOCTORS_FILE);
}

bool get_doctor(const string& id, DoctorRecord& out) {
    if (id.empty()) return false;

    const int offset = primary_search_doctor(id);
    if (offset < 0) return false;

    if (!read_record(DOCTORS_FILE, offset,
                     reinterpret_cast<char*>(&out), DOCTOR_RECORD_SIZE))
        return false;

    // Protect callers from stale/corrupt primary-index entries.
    return out.id[0] != '*';
}

bool add_doctor(const string& id, const string& name,
                const string& specialty, const string& address) {
    if (id.empty() || id.size() > DOC_ID_SIZE ||
        name.empty() || name.size() > DOC_NAME_SIZE ||
        specialty.empty() || specialty.size() > DOC_SPEC_SIZE ||
        address.size() > DOC_ADDR_SIZE) {
        cout << "Error: invalid input or field too long.\n";
        return false;
    }

    // '*' is the logical-deletion marker in the first byte of a record.
    if (id[0] == '*') {
        cout << "Error: Doctor ID cannot start with '*'.\n";
        return false;
    }

    if (primary_search_doctor(id) != -1) {
        cout << "Error: Doctor ID '" << id << "' already exists. Record not added.\n";
        return false;
    }

    DoctorRecord rec{};
    set_field(rec.id, DOC_ID_SIZE, id);
    set_field(rec.name, DOC_NAME_SIZE, name);
    set_field(rec.specialty, DOC_SPEC_SIZE, specialty);
    set_field(rec.address, DOC_ADDR_SIZE, address);

    // Add hooks: data file -> primary index -> secondary indexes.
    const int offset = add_record(
        DOCTORS_FILE,
        reinterpret_cast<const char*>(&rec),
        DOCTOR_RECORD_SIZE
    );

    if (offset < 0) {
        cout << "Error: failed to write doctor record.\n";
        return false;
    }

    // The index functions are owned by P3 and P4.
    primary_insert(doctors_primary, id, offset);
    secondary_insert(doctors_name_index, name, id);
    secondary_insert(doctors_specialty_index, specialty, id);

    return true;
}

bool update_doctor_name(const string& id, const string& new_name) {
    if (new_name.empty() || new_name.size() > DOC_NAME_SIZE) {
        cout << "Error: doctor name is empty or exceeds "
             << DOC_NAME_SIZE << " characters.\n";
        return false;
    }

    const int offset = primary_search_doctor(id);
    if (offset < 0) {
        cout << "Warning: Doctor ID '" << id << "' not found.\n";
        return false;
    }

    DoctorRecord rec{};
    if (!read_record(DOCTORS_FILE, offset,
                     reinterpret_cast<char*>(&rec), DOCTOR_RECORD_SIZE) ||
        rec.id[0] == '*') {
        cout << "Error: could not read the doctor record.\n";
        return false;
    }

    const string old_name = get_field(rec.name, DOC_NAME_SIZE);
    if (old_name == new_name) return true;

    if (!update_in_place(DOCTORS_FILE, offset,
                         DOC_NAME_OFF, DOC_NAME_SIZE, new_name)) {
        cout << "Error: failed to update doctor name in the data file.\n";
        return false;
    }

    // Update the secondary index only after the data-file update succeeds.
    secondary_update(doctors_name_index, old_name, new_name, id);
    return true;
}

bool delete_doctor(const string& id) {
    const int offset = primary_search_doctor(id);
    if (offset < 0) {
        cout << "Warning: Doctor ID '" << id << "' not found.\n";
        return false;
    }

    // Do not delete a doctor who still has appointments.
    if (!secondary_search(appointments_doctor_index, id).empty()) {
        cout << "Error: cannot delete doctor '" << id
             << "' because the doctor still has appointments.\n";
        return false;
    }

    // Read all indexed fields before the record is marked as deleted.
    DoctorRecord rec{};
    if (!read_record(DOCTORS_FILE, offset,
                     reinterpret_cast<char*>(&rec), DOCTOR_RECORD_SIZE) ||
        rec.id[0] == '*') {
        cout << "Error: could not read the doctor record.\n";
        return false;
    }

    const string name = get_field(rec.name, DOC_NAME_SIZE);
    const string specialty = get_field(rec.specialty, DOC_SPEC_SIZE);

    // First perform the logical deletion. Do not remove index entries if
    // the data-file deletion fails.
    if (!delete_record(DOCTORS_FILE, offset)) {
        cout << "Error: failed to delete doctor record from the data file.\n";
        return false;
    }

    // Delete hooks: remove all secondary references, then primary reference.
    secondary_remove(doctors_name_index, name, id);
    secondary_remove(doctors_specialty_index, specialty, id);
    primary_remove(doctors_primary, id);

    return true;
}

bool print_doctor(const string& id) {
    DoctorRecord rec{};
    if (!get_doctor(id, rec)) {
        cout << "Warning: Doctor ID '" << id << "' not found.\n";
        return false;
    }

    cout << "Doctor ID : " << get_field(rec.id, DOC_ID_SIZE) << '\n'
         << "Name      : " << get_field(rec.name, DOC_NAME_SIZE) << '\n'
         << "Specialty : " << get_field(rec.specialty, DOC_SPEC_SIZE) << '\n'
         << "Address   : " << get_field(rec.address, DOC_ADDR_SIZE) << '\n';

    return true;
}
