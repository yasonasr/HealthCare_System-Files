#ifndef OPERATIONS_H
#define OPERATIONS_H
// Owner: P5 - coordinates only, never re-implements functions of P1..P4.
#include <string>
using namespace std;

// Changes a Doctor ID (primary key) and cascades to the doctors file, Primary Index,
// Name/Specialty Secondary Indexes, the appointments file and the Appointments Doctor ID index.
bool update_doctor_id(const string& old_id, const string& new_id);
#endif
