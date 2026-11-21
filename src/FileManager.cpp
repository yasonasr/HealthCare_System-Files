#include "../include/FileManager.h"
#include "../include/RecordStructures.h"
#include <cstring>
#include <fstream>
using namespace std;

bool init_file(const string& file) {
    ifstream in(file, ios::binary | ios::ate);
    if (in && in.tellg() >= HEADER_SIZE) return true;          // already exists with a header
    in.close();
    ofstream out(file, ios::binary | ios::trunc);               // the data/ folder must already exist
    int head = -1;
    out.write(reinterpret_cast<char*>(&head), sizeof(int));
    return (bool)out;
}

int read_header(const string& file) {
    ifstream in(file, ios::binary);
    int head = -2;
    in.read(reinterpret_cast<char*>(&head), sizeof(int));
    return in ? head : -2;
}

bool write_header(const string& file, int value) {
    return write_record(file, 0, reinterpret_cast<char*>(&value), sizeof(int));
}

bool read_record(const string& file, int offset, char* buf, int size) {
    if (offset < 0 || size <= 0) return false;
    ifstream in(file, ios::binary);
    in.seekg(offset);
    in.read(buf, size);
    return (bool)in;
}

bool write_record(const string& file, int offset, const char* data, int size) {
    if (offset < 0 || size <= 0) return false;
    fstream io(file, ios::in | ios::out | ios::binary);
    io.seekp(offset);
    io.write(data, size);
    io.flush();
    return (bool)io;
}

int add_record(const string& file, const char* data, int size) {
    int head = read_header(file);
    if (head == -2) return -1;

    if (head != -1) {                                          // AVAIL LIST not empty: reuse the first deleted record
        int next;
        if (!read_record(file, head + 1, reinterpret_cast<char*>(&next), sizeof(int))) return -1;
        if (!write_record(file, head, data, size)) return -1;
        return write_header(file, next) ? head : -1;           // header now points to the next deleted record
    }
    fstream io(file, ios::in | ios::out | ios::binary);        // AVAIL LIST empty: append at the end
    io.seekp(0, ios::end);
    int offset = (int)io.tellp();
    io.write(data, size);
    io.flush();
    return io ? offset : -1;
}

bool delete_record(const string& file, int offset) {
    char mark;
    if (offset < HEADER_SIZE || !read_record(file, offset, &mark, 1) || mark == '*') return false;
    int oldHead = read_header(file);
    if (oldHead == -2) return false;
    char node[1 + sizeof(int)];
    node[0] = '*';
    memcpy(node + 1, &oldHead, sizeof(int));
    return write_record(file, offset, node, sizeof(node)) && write_header(file, offset);
}

bool update_in_place(const string& file, int offset, int field_offset, int field_size, const string& value) {
    char mark;
    if (offset < HEADER_SIZE || field_offset < 0 || value.size() > (size_t)field_size) return false;
    if (!read_record(file, offset, &mark, 1) || mark == '*') return false;   // must be an active record
    string padded(field_size, '\0');
    memcpy(&padded[0], value.data(), value.size());
    return write_record(file, offset + field_offset, padded.data(), field_size);
}
