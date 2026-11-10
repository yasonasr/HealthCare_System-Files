#include "../include/SecondaryIndex.h"
#include "../include/Utils.h"
#include <fstream>
#include <algorithm>

using namespace std;

// Secondary index instances
SecondaryIndex doctors_name;
SecondaryIndex doctors_specialty;
SecondaryIndex appointments_doctor;

// Alias references
SecondaryIndex& doctors_name_index = doctors_name;
SecondaryIndex& doctors_specialty_index = doctors_specialty;
SecondaryIndex& appointments_doctor_index = appointments_doctor;

// Helper function to clean a string:
// 1. Truncates at the first null character ('\0') if present
// 2. Trims leading and trailing whitespace (' ', '\t', '\r', '\n')
static string clean_string(const string& s) {
    if (s.empty()) return "";

    size_t first_null = s.find('\0');
    string temp = (first_null != string::npos) ? s.substr(0, first_null) : s;

    size_t start = temp.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";

    size_t end = temp.find_last_not_of(" \t\r\n");
    return temp.substr(start, end - start + 1);
}

// secondary_insert: inserts or finds the secondary key and prepends/links the primary key to its chain.
bool secondary_insert(SecondaryIndex& index, const string& sec_key, const string& primary_key) {
    string sk = clean_string(sec_key);
    string pk = clean_string(primary_key);
    if (pk.length() > 15) pk = pk.substr(0, 15);
    if (sk.empty() || pk.empty()) return false;

    int pos = binary_search(index, sk);
    if (pos != -1) {
        // Key already exists in index.
        // Check for duplicate primary key in this chain
        if (index[pos].head >= 0) {
            vector<string> chain = ll_traverse(index[pos].head);
            for (size_t i = 0; i < chain.size(); ++i) {
                if (chain[i] == pk) {
                    return false; // Already present in chain
                }
            }
        }
        int new_head = ll_insert(index[pos].head, pk);
        if (new_head < 0) return false;
        index[pos].head = new_head;
        return true;
    }

    // Key not found in index: create new chain
    int new_head = ll_insert(-1, pk);
    if (new_head < 0) return false;

    int insert_pos = binary_insert_pos(index, sk);
    SecondaryEntry entry = { sk, new_head };
    index.insert(index.begin() + insert_pos, entry);
    return true;
}

// Helper function to check if a primary key exists anywhere in a secondary index
static bool primary_key_exists_in_index(const SecondaryIndex& index, const string& pk) {
    for (size_t i = 0; i < index.size(); ++i) {
        if (index[i].head >= 0) {
            vector<string> ch = ll_traverse(index[i].head);
            for (size_t k = 0; k < ch.size(); ++k) {
                if (clean_string(ch[k]) == pk) return true;
            }
        }
    }
    return false;
}

// secondary_remove: removes a primary key from the chain of the given secondary key.
bool secondary_remove(SecondaryIndex& index, const string& sec_key, const string& primary_key) {
    string sk = clean_string(sec_key);
    string pk = clean_string(primary_key);
    if (pk.length() > 15) pk = pk.substr(0, 15);
    if (sk.empty() || pk.empty()) return false;

    int pos = binary_search(index, sk);
    if (pos == -1 || index[pos].head < 0) return false;

    // Verify primary key exists in the chain
    vector<string> chain = ll_traverse(index[pos].head);
    bool exists = false;
    for (size_t i = 0; i < chain.size(); ++i) {
        if (chain[i] == pk) {
            exists = true;
            break;
        }
    }
    if (!exists) return false;

    int new_head = ll_remove(index[pos].head, pk);
    if (chain.size() == 1) {
        if (new_head >= 0) return false; // ll_remove failed on 1-element chain
        // Chain is now empty; remove entry to keep index compact and sorted
        index.erase(index.begin() + pos);
    } else {
        if (new_head < 0) return false; // ll_remove failed on non-empty chain
        if (chain[0] == pk && new_head == index[pos].head) return false; // head node failed to unlink
        index[pos].head = new_head;
    }
    return true;
}

// secondary_update: moves one record from one secondary key to another with rollback protection against data loss.
bool secondary_update(SecondaryIndex& index, const string& old_sec_key, const string& new_sec_key, const string& primary_key) {
    string old_sk = clean_string(old_sec_key);
    string new_sk = clean_string(new_sec_key);
    string pk = clean_string(primary_key);
    if (pk.length() > 15) pk = pk.substr(0, 15);

    if (old_sk.empty() || new_sk.empty() || pk.empty()) return false;

    // If old and new secondary keys are identical, verify existence
    if (old_sk == new_sk) {
        int pos = binary_search(index, old_sk);
        if (pos == -1 || index[pos].head < 0) return false;
        vector<string> chain = ll_traverse(index[pos].head);
        for (size_t i = 0; i < chain.size(); ++i) {
            if (chain[i] == pk) return true;
        }
        return false;
    }

    // Verify pk exists in old_sk before doing any removal
    int old_pos = binary_search(index, old_sk);
    if (old_pos == -1 || index[old_pos].head < 0) return false;
    vector<string> old_chain = ll_traverse(index[old_pos].head);
    bool in_old = false;
    for (size_t i = 0; i < old_chain.size(); ++i) {
        if (old_chain[i] == pk) {
            in_old = true;
            break;
        }
    }
    if (!in_old) return false;

    // Verify pk does not already exist in new_sk (prevent duplicate primary key in new chain)
    int new_pos = binary_search(index, new_sk);
    if (new_pos != -1 && index[new_pos].head >= 0) {
        vector<string> new_chain = ll_traverse(index[new_pos].head);
        for (size_t i = 0; i < new_chain.size(); ++i) {
            if (new_chain[i] == pk) {
                return false; // Already present in target chain
            }
        }
    }

    // Remove from old key
    if (!secondary_remove(index, old_sk, pk)) {
        return false;
    }

    // Insert into new key; if it fails, roll back to preserve data integrity
    if (!secondary_insert(index, new_sk, pk)) {
        secondary_insert(index, old_sk, pk);
        return false;
    }
    return true;
}

// secondary_search: returns the list of primary keys in the chain for the given secondary key.
vector<string> secondary_search(const SecondaryIndex& index, const string& sec_key) {
    string sk = clean_string(sec_key);
    if (sk.empty()) return vector<string>();

    int pos = binary_search(index, sk);
    if (pos == -1 || index[pos].head < 0) {
        return vector<string>();
    }
    return ll_traverse(index[pos].head);
}

// secondary_change_primary_ref: replaces a primary key inside a chain (using ll_replace_key) when a doctor ID changes.
bool secondary_change_primary_ref(SecondaryIndex& index, const string& sec_key, const string& old_primary, const string& new_primary) {
    string sk = clean_string(sec_key);
    string old_pk = clean_string(old_primary);
    string new_pk = clean_string(new_primary);
    if (old_pk.length() > 15) old_pk = old_pk.substr(0, 15);
    if (new_pk.length() > 15) new_pk = new_pk.substr(0, 15);
    if (sk.empty() || old_pk.empty() || new_pk.empty()) return false;

    int pos = binary_search(index, sk);
    if (pos == -1 || index[pos].head < 0) return false;

    if (old_pk == new_pk) {
        vector<string> chain = ll_traverse(index[pos].head);
        for (size_t i = 0; i < chain.size(); ++i) {
            if (chain[i] == old_pk) return true;
        }
        return false;
    }

    vector<string> chain = ll_traverse(index[pos].head);
    bool found_old = false;
    for (size_t i = 0; i < chain.size(); ++i) {
        if (chain[i] == old_pk) {
            found_old = true;
            break;
        }
    }
    if (!found_old) return false;

    // Check if new_pk already exists anywhere in this index (prevent duplicate primary key collision)
    if (primary_key_exists_in_index(index, new_pk)) return false;

    return ll_replace_key(index[pos].head, old_pk, new_pk);
}

// Overload: updates the primary key reference in both Doctor Name and Doctor Specialty indexes when a Doctor ID changes.
bool secondary_change_primary_ref(const string& old_primary, const string& new_primary) {
    string old_pk = clean_string(old_primary);
    string new_pk = clean_string(new_primary);
    if (old_pk.length() > 15) old_pk = old_pk.substr(0, 15);
    if (new_pk.length() > 15) new_pk = new_pk.substr(0, 15);
    if (old_pk.empty() || new_pk.empty()) return false;

    // Locate matching entry in doctors_name
    int name_idx = -1;
    for (size_t i = 0; i < doctors_name.size(); ++i) {
        if (doctors_name[i].head >= 0) {
            vector<string> ch = ll_traverse(doctors_name[i].head);
            for (size_t k = 0; k < ch.size(); ++k) {
                if (ch[k] == old_pk) {
                    name_idx = (int)i;
                    break;
                }
            }
            if (name_idx != -1) break;
        }
    }

    // Locate matching entry in doctors_specialty
    int spec_idx = -1;
    for (size_t i = 0; i < doctors_specialty.size(); ++i) {
        if (doctors_specialty[i].head >= 0) {
            vector<string> ch = ll_traverse(doctors_specialty[i].head);
            for (size_t k = 0; k < ch.size(); ++k) {
                if (ch[k] == old_pk) {
                    spec_idx = (int)i;
                    break;
                }
            }
            if (spec_idx != -1) break;
        }
    }

    if (name_idx == -1 && spec_idx == -1) return false;

    if (old_pk == new_pk) return true;

    // Pre-validate that new_pk does not already exist anywhere in either index (prevent duplicate key collision)
    if (primary_key_exists_in_index(doctors_name, new_pk) ||
        primary_key_exists_in_index(doctors_specialty, new_pk)) {
        return false;
    }

    // Atomic execution across both indexes with rollback protection
    if (name_idx != -1 && spec_idx != -1) {
        bool r1 = ll_replace_key(doctors_name[name_idx].head, old_pk, new_pk);
        if (!r1) return false;
        bool r2 = ll_replace_key(doctors_specialty[spec_idx].head, old_pk, new_pk);
        if (!r2) {
            // Roll back doctors_name to preserve cross-index consistency
            ll_replace_key(doctors_name[name_idx].head, new_pk, old_pk);
            return false;
        }
        return true;
    }

    if (name_idx != -1) {
        return ll_replace_key(doctors_name[name_idx].head, old_pk, new_pk);
    }
    return ll_replace_key(doctors_specialty[spec_idx].head, old_pk, new_pk);
}

// Overload: updates both indexes given doctor_name and doctor_specialty.
// If both are provided, both are validated first to preserve consistency across indexes.
bool secondary_change_primary_ref(const string& doctor_name, const string& doctor_specialty, const string& old_primary, const string& new_primary) {
    string dn = clean_string(doctor_name);
    string ds = clean_string(doctor_specialty);
    string old_pk = clean_string(old_primary);
    string new_pk = clean_string(new_primary);
    if (old_pk.length() > 15) old_pk = old_pk.substr(0, 15);
    if (new_pk.length() > 15) new_pk = new_pk.substr(0, 15);
    if (old_pk.empty() || new_pk.empty()) return false;
    if (dn.empty() && ds.empty()) return false;

    if (!dn.empty() && !ds.empty()) {
        int pos_name = binary_search(doctors_name, dn);
        if (pos_name == -1 || doctors_name[pos_name].head < 0) return false;
        vector<string> name_chain = ll_traverse(doctors_name[pos_name].head);
        bool found_in_name = false;
        for (size_t i = 0; i < name_chain.size(); ++i) {
            if (name_chain[i] == old_pk) {
                found_in_name = true;
                break;
            }
        }
        if (!found_in_name) return false;

        int pos_spec = binary_search(doctors_specialty, ds);
        if (pos_spec == -1 || doctors_specialty[pos_spec].head < 0) return false;
        vector<string> spec_chain = ll_traverse(doctors_specialty[pos_spec].head);
        bool found_in_spec = false;
        for (size_t i = 0; i < spec_chain.size(); ++i) {
            if (spec_chain[i] == old_pk) {
                found_in_spec = true;
                break;
            }
        }
        if (!found_in_spec) return false;

        if (old_pk == new_pk) return true;

        // Prevent duplicate primary key collision across both indexes
        if (primary_key_exists_in_index(doctors_name, new_pk) ||
            primary_key_exists_in_index(doctors_specialty, new_pk)) {
            return false;
        }

        bool r1 = secondary_change_primary_ref(doctors_name, dn, old_pk, new_pk);
        if (!r1) return false;
        bool r2 = secondary_change_primary_ref(doctors_specialty, ds, old_pk, new_pk);
        if (!r2) {
            // Roll back doctors_name to maintain cross-index consistency
            secondary_change_primary_ref(doctors_name, dn, new_pk, old_pk);
            return false;
        }
        return true;
    }

    if (!dn.empty()) {
        if (primary_key_exists_in_index(doctors_name, new_pk) ||
            primary_key_exists_in_index(doctors_specialty, new_pk)) {
            return false;
        }
        return secondary_change_primary_ref(doctors_name, dn, old_pk, new_pk);
    }
    if (primary_key_exists_in_index(doctors_name, new_pk) ||
        primary_key_exists_in_index(doctors_specialty, new_pk)) {
        return false;
    }
    return secondary_change_primary_ref(doctors_specialty, ds, old_pk, new_pk);
}

// Helper parser for a Doctor record from doctors.dat
static bool parse_doctor_record(const char* buf, int rec_size, string& doc_id, string& doc_name, string& doc_specialty) {
    if (buf[0] == '*') return false; // Deleted marker

    bool has_delim = false;
    for (int i = 0; i < rec_size; ++i) {
        if (buf[i] == '|') {
            has_delim = true;
            break;
        }
    }

    if (has_delim) {
        vector<string> tokens;
        string cur;
        for (int i = 0; i < rec_size && buf[i] != '\0'; ++i) {
            if (buf[i] == '|') {
                tokens.push_back(clean_string(cur));
                cur.clear();
            } else {
                cur.push_back(buf[i]);
            }
        }
        if (!cur.empty()) tokens.push_back(clean_string(cur));

        if (tokens.size() >= 3) {
            doc_id = tokens[0];
            doc_name = tokens[1];
            doc_specialty = tokens[2];
        } else if (tokens.size() == 2) {
            doc_id = tokens[0];
            doc_name = tokens[1];
            doc_specialty = "";
        } else if (tokens.size() == 1) {
            doc_id = tokens[0];
            doc_name = "";
            doc_specialty = "";
        }
        if (doc_id.length() > 15) doc_id = doc_id.substr(0, 15);
        if (doc_name.length() > 30) doc_name = doc_name.substr(0, 30);
        if (doc_specialty.length() > 30) doc_specialty = doc_specialty.substr(0, 30);
    } else {
        // Fixed-length fields according to IS321 schema (rec_size 95):
        // Doctor ID: 15 bytes (offset 0..14)
        // Doctor Name: 30 bytes (offset 15..44)
        // Doctor Specialty: 30 bytes (offset 45..74)
        // Doctor Address/Padding: 20 bytes (offset 75..94)
        doc_id = clean_string(field_to_string(buf, 15));
        doc_name = clean_string(field_to_string(buf + 15, 30));
        doc_specialty = clean_string(field_to_string(buf + 45, 30));
    }
    return !doc_id.empty();
}

// Helper parser for an Appointment record from appointments.dat
static bool parse_appointment_record(const char* buf, int rec_size, string& app_id, string& doc_id) {
    if (buf[0] == '*') return false; // Deleted marker

    bool has_delim = false;
    for (int i = 0; i < rec_size; ++i) {
        if (buf[i] == '|') {
            has_delim = true;
            break;
        }
    }

    if (has_delim) {
        vector<string> tokens;
        string cur;
        for (int i = 0; i < rec_size && buf[i] != '\0'; ++i) {
            if (buf[i] == '|') {
                tokens.push_back(clean_string(cur));
                cur.clear();
            } else {
                cur.push_back(buf[i]);
            }
        }
        if (!cur.empty()) tokens.push_back(clean_string(cur));

        if (tokens.size() >= 2) {
            app_id = tokens[0];
            doc_id = tokens[1];
        } else if (tokens.size() == 1) {
            app_id = tokens[0];
            doc_id = "";
        }
        if (app_id.length() > 15) app_id = app_id.substr(0, 15);
        if (doc_id.length() > 15) doc_id = doc_id.substr(0, 15);
    } else {
        // Fixed-length fields according to IS321 schema (rec_size 60):
        // Appointment ID: 15 bytes (offset 0..14)
        // Doctor ID: 15 bytes (offset 15..29)
        // Appointment Date: 30 bytes (offset 30..59)
        app_id = clean_string(field_to_string(buf, 15));
        doc_id = clean_string(field_to_string(buf + 15, 15));
    }
    return !app_id.empty() && !doc_id.empty();
}

// Rebuilds doctors secondary indexes by scanning doctors data file
int build_doctors_secondary(const string& path) {
    doctors_name.clear();
    doctors_specialty.clear();

    ifstream f(path.c_str(), ios::binary);
    if (!f) return -1;

    int header = 0;
    f.read(reinterpret_cast<char*>(&header), sizeof(int));
    if (!f) return 0;

    int count = 0;
    const int rec_size = 95;
    vector<char> buf(rec_size);
    while (f.read(buf.data(), rec_size)) {
        string doc_id, doc_name, doc_specialty;
        if (parse_doctor_record(buf.data(), rec_size, doc_id, doc_name, doc_specialty)) {
            if (!doc_name.empty()) {
                secondary_insert(doctors_name, doc_name, doc_id);
            }
            if (!doc_specialty.empty()) {
                secondary_insert(doctors_specialty, doc_specialty, doc_id);
            }
            count++;
        }
    }
    return count;
}

// Rebuilds appointments secondary index by scanning appointments data file
int build_appointments_secondary(const string& path) {
    appointments_doctor.clear();

    ifstream f(path.c_str(), ios::binary);
    if (!f) return -1;

    int header = 0;
    f.read(reinterpret_cast<char*>(&header), sizeof(int));
    if (!f) return 0;

    int count = 0;
    const int rec_size = 60;
    vector<char> buf(rec_size);
    while (f.read(buf.data(), rec_size)) {
        string app_id, doc_id;
        if (parse_appointment_record(buf.data(), rec_size, app_id, doc_id)) {
            secondary_insert(appointments_doctor, doc_id, app_id);
            count++;
        }
    }
    return count;
}

// Rebuilds all three secondary indexes and secondary_links.dat at startup
int build_secondary_indexes(const string& doctors_path, const string& appointments_path) {
    // 1. Reset the linked list file
    ll_reset();

    // 2. Clear all three in-memory indexes (build_* also clears them)
    doctors_name.clear();
    doctors_specialty.clear();
    appointments_doctor.clear();

    // 3. Scan and rebuild from data files
    int doc_count = build_doctors_secondary(doctors_path);
    int app_count = build_appointments_secondary(appointments_path);

    int total = 0;
    if (doc_count > 0) total += doc_count;
    if (app_count > 0) total += app_count;
    return total;
}
