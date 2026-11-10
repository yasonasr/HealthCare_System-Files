#pragma once
// Owner: P1
#include "RecordStructures.h"
#include <string>

bool create_doctors_file();
bool add_doctor(const std::string& id, const std::string& name, const std::string& specialty, const std::string& address);
bool update_doctor_name(const std::string& id, const std::string& new_name);
bool delete_doctor(const std::string& id);
bool print_doctor(const std::string& id);
bool get_doctor(const std::string& id, DoctorRecord& out);   // used by P5
