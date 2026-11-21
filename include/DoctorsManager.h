#ifndef DOCTORS_MANAGER_H
#define DOCTORS_MANAGER_H
// Owner: P1 - Doctors operations.
#include <string>
#include "RecordStructures.h"
using namespace std;

bool create_doctors_file();
bool add_doctor(const string& id, const string& name, const string& specialty, const string& address);
bool update_doctor_name(const string& id, const string& new_name);   // non-key
bool delete_doctor(const string& id);                                // rejected if the doctor has appointments
bool print_doctor(const string& id);
bool get_doctor(const string& id, DoctorRecord& out);                // via the Primary Index (used by P5)
#endif
