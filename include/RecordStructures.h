#ifndef RECORD_STRUCTURES_H
#define RECORD_STRUCTURES_H
using namespace std;
#include <algorithm>
#include <cstring>
#include <string>

// ---- files ----
const int HEADER_SIZE = (int)sizeof(int);                 // every data file starts with an int header
const std::string DOCTORS_FILE = "data/doctors.dat";
const std::string APPOINTMENTS_FILE = "data/appointments.dat";
const std::string LINKS_FILE = "indexes/secondary_links.dat";

// ---- field sizes ----
const int ID_SIZE = 15;
const int DOC_ID_SIZE = 15;
const int DOC_NAME_SIZE = 30;
const int DOC_SPEC_SIZE = 20;
const int DOC_ADDR_SIZE = 30;
const int APPT_ID_SIZE = 15, APPT_DOCID_SIZE = 15, APPT_DATE_SIZE = 30;

// ---- records (only char arrays => no padding) ----
struct DoctorRecord {
    char id[DOC_ID_SIZE];
    char name[DOC_NAME_SIZE];
    char specialty[DOC_SPEC_SIZE];
    char address[DOC_ADDR_SIZE];
};

struct AppointmentRecord {
    char id[APPT_ID_SIZE];
    char date[APPT_DATE_SIZE];
    char doctor_id[APPT_DOCID_SIZE];
};
const int DOCTOR_RECORD_SIZE = (int)sizeof(DoctorRecord);
const int APPT_RECORD_SIZE   = (int)sizeof(AppointmentRecord);
static_assert(sizeof(DoctorRecord) == 95, "Doctors record must be 95 bytes");
static_assert(sizeof(AppointmentRecord) == 60, "Appointments record must be 60 bytes");

// ---- field offsets inside a record (used by update_in_place) ----
const int DOC_ID_OFF = 0;
const int DOC_NAME_OFF = 15;
const int DOC_SPEC_OFF = 45;
const int DOC_ADDR_OFF = 65;

const int APPT_ID_OFF = 0;
const int APPT_DATE_OFF = 15;
const int APPT_DOCID_OFF = 45;
// ---- helpers: fixed-size char fields <-> std::string (padding = '\0') ----
inline void set_field(char* dst, int size, const std::string& value) {
    std::memset(dst, 0, size);
    std::memcpy(dst, value.data(), std::min<size_t>(value.size(), (size_t)size));
}
inline std::string get_field(const char* src, int size) {
    int len = 0;
    while (len < size && src[len] != '\0') len++;
    while (len > 0 && src[len - 1] == ' ') len--;
    return std::string(src, len);
}

// Index entry structures live next to their owners:
//   PrimaryEntry   -> PrimaryIndex.h (P3)      SecondaryEntry -> SecondaryIndex.h (P4)
//   LLNode         -> LinkedList.cpp (P2)
#endif
