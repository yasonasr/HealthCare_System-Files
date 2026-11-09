#include "FileManager.h"
#include "RecordStructures.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <cstring>

using namespace std;
namespace fs = std::filesystem;

namespace {
    constexpr int IO_ERROR = -2;

    bool valid_offset(int offset, long long total_size, int bytes_needed) {
        return offset >= HEADER_SIZE &&
               total_size >= 0 &&
               static_cast<long long>(offset) + bytes_needed <= total_size;
    }

    bool get_file_size(const string& file, long long& result) {
        error_code ec;
        result = static_cast<long long>(fs::file_size(file, ec));
        return !ec;
    }

    fstream open_rw(const string& file) {
        return fstream(file, ios::in | ios::out | ios::binary);
    }
}

bool init_file(const string& file) {
    error_code ec;

    if (fs::exists(file, ec) && !ec) {
        long long size = 0;
        if (get_file_size(file, size) && size >= HEADER_SIZE)
            return true;
    }

    fs::path p(file);
    if (p.has_parent_path()) {
        fs::create_directories(p.parent_path(), ec);
        if (ec) return false;
    }

    ofstream out(file, ios::binary | ios::trunc);
    if (!out) return false;

    const int empty_head = -1;
    out.write(reinterpret_cast<const char*>(&empty_head), sizeof(empty_head));
    out.flush();
    return static_cast<bool>(out);
}

int read_header(const string& file) {
    if (!init_file(file)) return IO_ERROR;

    ifstream in(file, ios::binary);
    if (!in) return IO_ERROR;

    int head = -1;
    in.read(reinterpret_cast<char*>(&head), sizeof(head));
    if (!in) return IO_ERROR;

    return head;
}

bool write_header(const string& file, int value) {
    if (!init_file(file)) return false;

    fstream io = open_rw(file);
    if (!io) return false;

    io.seekp(0, ios::beg);
    if (!io) return false;

    io.write(reinterpret_cast<const char*>(&value), sizeof(value));
    io.flush();
    return static_cast<bool>(io);
}

bool read_record(const string& file, int offset, char* buf, int size) {
    if (buf == nullptr || size <= 0 || offset < HEADER_SIZE) return false;

    long long total_size = 0;
    if (!get_file_size(file, total_size) ||
        !valid_offset(offset, total_size, size))
        return false;

    ifstream in(file, ios::binary);
    if (!in) return false;

    in.seekg(offset, ios::beg);
    if (!in) return false;

    in.read(buf, size);
    return static_cast<bool>(in);
}

bool write_record(const string& file, int offset, const char* data, int size) {
    if (data == nullptr || size <= 0 || offset < HEADER_SIZE) return false;

    long long total_size = 0;
    if (!get_file_size(file, total_size) ||
        !valid_offset(offset, total_size, size))
        return false;

    fstream io = open_rw(file);
    if (!io) return false;

    io.seekp(offset, ios::beg);
    if (!io) return false;

    io.write(data, size);
    io.flush();
    return static_cast<bool>(io);
}

int add_record(const string& file, const char* data, int size) {
    if (data == nullptr || size <= 0 || !init_file(file))
        return -1;

    long long total_size = 0;
    if (!get_file_size(file, total_size) || total_size < HEADER_SIZE)
        return -1;

    const int head = read_header(file);
    if (head == IO_ERROR) return -1;

    // Reuse the first logically deleted record.
    if (head != -1) {
        // A deleted-list node needs at least marker + sizeof(int).
        if (!valid_offset(head, total_size, 1 + static_cast<int>(sizeof(int))))
            return -1;

        char marker = '\0';
        int next = -1;

        if (!read_record(file, head, &marker, 1) ||
            !read_record(file, head + 1, reinterpret_cast<char*>(&next), sizeof(next)))
            return -1;

        if (marker != '*') return -1;

        if (next != -1 &&
            !valid_offset(next, total_size, 1 + static_cast<int>(sizeof(int))))
            return -1;

        // Write the new record first. If updating the header fails, restore
        // the deleted-list node so the AVAIL LIST is not lost.
        char old_node[1 + sizeof(int)];
        old_node[0] = marker;
        memcpy(old_node + 1, &next, sizeof(next));

        if (!write_record(file, head, data, size))
            return -1;

        if (!write_header(file, next)) {
            // Best-effort rollback.
            write_record(file, head, old_node, sizeof(old_node));
            return -1;
        }

        return head;
    }

    // AVAIL LIST empty: append at end.
    if (total_size > numeric_limits<int>::max() ||
        total_size + size > numeric_limits<int>::max())
        return -1;

    ofstream out(file, ios::binary | ios::app);
    if (!out) return -1;

    const int offset = static_cast<int>(total_size);
    out.write(data, size);
    out.flush();

    if (!out) return -1;
    return offset;
}

bool delete_record(const string& file, int offset) {
    if (!init_file(file)) return false;

    long long total_size = 0;
    if (!get_file_size(file, total_size) ||
        !valid_offset(offset, total_size, 1 + static_cast<int>(sizeof(int))))
        return false;

    char marker = '\0';
    if (!read_record(file, offset, &marker, 1) || marker == '*')
        return false;

    const int old_head = read_header(file);
    if (old_head == IO_ERROR) return false;

    if (old_head != -1 &&
        !valid_offset(old_head, total_size, 1 + static_cast<int>(sizeof(int))))
        return false;

    // Save the bytes that will be overwritten, allowing rollback if the
    // header update fails.
    char old_bytes[1 + sizeof(int)];
    if (!read_record(file, offset, old_bytes, sizeof(old_bytes)))
        return false;

    char deleted_node[1 + sizeof(int)];
    deleted_node[0] = '*';
    memcpy(deleted_node + 1, &old_head, sizeof(old_head));

    if (!write_record(file, offset, deleted_node, sizeof(deleted_node)))
        return false;

    if (!write_header(file, offset)) {
        // Best-effort rollback.
        write_record(file, offset, old_bytes, sizeof(old_bytes));
        return false;
    }

    return true;
}

bool update_in_place(const string& file, int offset,
                     int field_offset, int field_size,
                     const string& value) {
    if (offset < HEADER_SIZE || field_offset < 0 || field_size <= 0 ||
        value.size() > static_cast<size_t>(field_size))
        return false;

    // Ensure the target record exists and is not logically deleted.
    char marker = '\0';
    if (!read_record(file, offset, &marker, 1) || marker == '*')
        return false;

    if (field_offset > numeric_limits<int>::max() - offset ||
        field_size > numeric_limits<int>::max() - (offset + field_offset))
        return false;

    const int write_offset = offset + field_offset;

    long long total_size = 0;
    if (!get_file_size(file, total_size) ||
        !valid_offset(write_offset, total_size, field_size))
        return false;

    string padded(static_cast<size_t>(field_size), '\0');
    if (!value.empty())
        memcpy(&padded[0], value.data(), value.size());

    return write_record(file, write_offset, padded.data(), field_size);
}
