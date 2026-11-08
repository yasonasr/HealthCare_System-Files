#ifndef PRIMARY_INDEX_H
#define PRIMARY_INDEX_H
#include <string>
#include <vector>
using namespace std;

const int ID_SIZE= 15;   
const int DOCTOR_REC_SIZE= 95;
const int APPOINT_REC_SIZE= 60;
const int HEADER_SIZE= (int)sizeof(int); 

struct PrimaryEntry {
    string key;   
    int offset;  
};

typedef vector<PrimaryEntry> PrimaryIndex;

extern PrimaryIndex doctors_primary;
extern PrimaryIndex appointments_primary;

bool primary_insert(PrimaryIndex& index, const string& key, int offset); 
bool primary_remove(PrimaryIndex& index, const string& key);            
bool primary_change_key(PrimaryIndex& index, const string& old_key,const string& new_key);

int primary_search_doctor(const string& id);       
int primary_search_appointment(const string& id);  
int build_doctors_primary(const string& path = "data/doctors.dat");
int build_appointments_primary(const string& path = "data/appointments.dat");

#endif