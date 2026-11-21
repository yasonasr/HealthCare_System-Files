#include "../include/AppointmentsManager.h"
#include "../include/FileManager.h"
#include "../include/PrimaryIndex.h"
#include "../include/SecondaryIndex.h"
#include <iostream>
using namespace std;

bool create_appointments_file() { return init_file(APPOINTMENTS_FILE); }

bool get_appointment(const string& id, AppointmentRecord& out) {
    if (id.empty()) return false;
    int offset = primary_search_appointment(id);
    if (offset < 0) return false;
    if (!read_record(APPOINTMENTS_FILE, offset, reinterpret_cast<char*>(&out), APPT_RECORD_SIZE))
        return false;
    return out.id[0] != '*';
}

bool add_appointment(const string& id, const string& doctor_id, const string& date) {
    if (id.empty() || id.size() > (size_t)APPT_ID_SIZE || doctor_id.empty() ||
        doctor_id.size() > (size_t)APPT_DOCID_SIZE || date.empty() || date.size() > (size_t)APPT_DATE_SIZE) {
        cout << "Error: invalid input or field too long.\n";
        return false;
    }
    if (id[0] == '*') { cout << "Error: Appointment ID cannot start with '*'.\n"; return false; }
    if (primary_search_appointment(id) != -1) {                       // duplicate Appointment ID
        cout << "Error: Appointment ID '" << id << "' already exists.\n";
        return false;
    }
    if (primary_search_doctor(doctor_id) == -1) {                     // foreign key validation
        cout << "Error: Doctor ID '" << doctor_id << "' does not exist.\n";
        return false;
    }
    AppointmentRecord rec{};
    set_field(rec.id, APPT_ID_SIZE, id);
    set_field(rec.doctor_id, APPT_DOCID_SIZE, doctor_id);
    set_field(rec.date, APPT_DATE_SIZE, date);

    int offset = add_record(APPOINTMENTS_FILE, reinterpret_cast<const char*>(&rec), APPT_RECORD_SIZE);
    if (offset < 0) { cout << "Error: failed to write appointment record.\n"; return false; }
    primary_insert(appointments_primary, id, offset);
    secondary_insert(appointments_doctor_index, doctor_id, id);
    return true;
}

bool update_appointment_date(const string& id, const string& new_date) {
    if (new_date.empty() || new_date.size() > (size_t)APPT_DATE_SIZE) {
        cout << "Error: invalid date.\n";
        return false;
    }
    int offset = primary_search_appointment(id);
    if (offset < 0) { cout << "Warning: Appointment ID '" << id << "' not found.\n"; return false; }
    if (!update_in_place(APPOINTMENTS_FILE, offset, APPT_DATE_OFF, APPT_DATE_SIZE, new_date)) {
        cout << "Error: failed to update the appointment date.\n";
        return false;
    }
    return true;
}

bool delete_appointment(const string& id) {
    int offset = primary_search_appointment(id);
    if (offset < 0) { cout << "Warning: Appointment ID '" << id << "' not found.\n"; return false; }
    AppointmentRecord rec{};
    if (!get_appointment(id, rec)) { cout << "Error: could not read the appointment.\n"; return false; }
    string doctor_id = get_field(rec.doctor_id, APPT_DOCID_SIZE);

    if (!delete_record(APPOINTMENTS_FILE, offset)) {
        cout << "Error: failed to delete the appointment from the data file.\n";
        return false;
    }
    secondary_remove(appointments_doctor_index, doctor_id, id);
    primary_remove(appointments_primary, id);
    return true;
}

bool print_appointment(const string& id) {
    AppointmentRecord rec{};
    if (!get_appointment(id, rec)) { cout << "Warning: Appointment ID '" << id << "' not found.\n"; return false; }
    cout << "Appointment ID : " << get_field(rec.id, APPT_ID_SIZE) << '\n'
         << "Doctor ID      : " << get_field(rec.doctor_id, APPT_DOCID_SIZE) << '\n'
         << "Date           : " << get_field(rec.date, APPT_DATE_SIZE) << '\n';
    return true;
}

bool update_appointments_doctor_id(const string& old_id, const string& new_id) {
    vector<string> ids = secondary_search(appointments_doctor_index, old_id);   // copy first: index changes below
    bool ok = true;
    for (size_t i = 0; i < ids.size(); ++i) {
        int offset = primary_search_appointment(ids[i]);
        if (offset < 0 ||
            !update_in_place(APPOINTMENTS_FILE, offset, APPT_DOCID_OFF, APPT_DOCID_SIZE, new_id)) {
            ok = false;
            continue;
        }
        secondary_update(appointments_doctor_index, old_id, new_id, ids[i]);
    }
    return ok;
}
