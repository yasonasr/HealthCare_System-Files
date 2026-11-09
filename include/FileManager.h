#pragma once
// Owner: P1 - shared by P1 (Doctors) and P2 (Appointments).
// All offsets are byte offsets from the beginning of the file.
// The first record starts immediately after the 4-byte header.

#include <string>

bool init_file(const std::string& file);  // Creates the file with header = -1 if missing/invalid.
int  read_header(const std::string& file); // AVAIL LIST head (-1 = empty); -2 = read/error.
bool write_header(const std::string& file, int value);

bool read_record(const std::string& file, int offset, char* buf, int size);
bool write_record(const std::string& file, int offset, const char* data, int size);

// Returns the byte offset, or -1 on failure.
int add_record(const std::string& file, const char* data, int size);

// Logical deletion: writes '*' + previous header (int) into the deleted record.
// Returns false if the offset is invalid, the record is already deleted, or I/O fails.
bool delete_record(const std::string& file, int offset);

// Updates one field without changing the record's size.
// The new value is padded with null characters.
bool update_in_place(const std::string& file, int offset,
                     int field_offset, int field_size,
                     const std::string& value);
