#ifndef APPOINTMENTS_MANAGER_H
#define APPOINTMENTS_MANAGER_H
// Owner: P2 - Appointments operations (built on P1's FileManager).
#include <string>
#include "RecordStructures.h"
using namespace std;

bool create_appointments_file();
bool add_appointment(const string& id, const string& doctor_id, const string& date);
bool update_appointment_date(const string& id, const string& new_date);   // non-key, not indexed
bool delete_appointment(const string& id);
bool print_appointment(const string& id);
bool get_appointment(const string& id, AppointmentRecord& out);            // used by P5

// Doctor ID changed: updates every appointment of old_id (record + Appointments Doctor ID index).
bool update_appointments_doctor_id(const string& old_id, const string& new_id);
#endif
