#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H
// Owner: P1 - shared by P1 (Doctors) and P2 (Appointments).
// File layout: [int header][record 0][record 1]...   record offset = 4 + i * record_size
// Header = offset of the first deleted record (AVAIL LIST head), -1 if there is none.
// Deleted record = '*' + int (offset of the next deleted record, -1 = end).
#include <string>
using namespace std;

bool init_file(const string& file);                  // creates the file with header = -1 if missing
int  read_header(const string& file);                // AVAIL LIST head (-1 = empty, -2 = I/O error)
bool write_header(const string& file, int value);

bool read_record(const string& file, int offset, char* buf, int size);
bool write_record(const string& file, int offset, const char* data, int size);

int  add_record(const string& file, const char* data, int size);   // reuses a deleted record or appends; returns offset or -1
bool delete_record(const string& file, int offset);                // '*' + old header, header = offset

// Overwrites one field (null-padded); never writes more than field_size bytes.
bool update_in_place(const string& file, int offset, int field_offset, int field_size, const string& value);
#endif
