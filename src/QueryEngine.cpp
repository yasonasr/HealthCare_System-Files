#include "../include/QueryEngine.h"
#include "../include/DoctorsManager.h"
#include "../include/AppointmentsManager.h"
#include "../include/PrimaryIndex.h"
#include "../include/SecondaryIndex.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
using namespace std;

namespace {
    struct FieldDef  { string display, key; };
    struct Condition { string key, value; };

    string lower(string s) { for (size_t i = 0; i < s.size(); ++i) s[i] = (char)tolower((unsigned char)s[i]); return s; }
    string trim(const string& s) {
        size_t a = s.find_first_not_of(" \t\r\n");
        if (a == string::npos) return "";
        return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
    }
    // "Doctor ID" / "doctor_id" / "DOCTORID" -> "doctorid"
    string field_key(const string& s) {
        string r;
        for (size_t i = 0; i < s.size(); ++i)
            if (!isspace((unsigned char)s[i]) && s[i] != '_') r += (char)tolower((unsigned char)s[i]);
        return r;
    }
    // collapses runs of whitespace into one space (outside quotes)
    string collapse_ws(const string& s) {
        string r; char q = 0; bool pending = false;
        for (size_t i = 0; i < s.size(); ++i) {
            char c = s[i];
            if (q == 0 && isspace((unsigned char)c)) { pending = true; continue; }
            if (pending && !r.empty()) r += ' ';
            pending = false;
            if (q == 0 && (c == '\'' || c == '"')) q = c;
            else if (c == q) q = 0;
            r += c;
        }
        return r;
    }
    // splits at " and " outside quotes
    vector<string> split_and(const string& s) {
        vector<string> parts; string cur; char q = 0;
        for (size_t i = 0; i < s.size(); ++i) {
            char c = s[i];
            if (q == 0 && (c == '\'' || c == '"')) q = c;
            else if (c == q) q = 0;
            if (q == 0 && i + 5 <= s.size() && lower(s.substr(i, 5)) == " and ") {
                parts.push_back(cur); cur.clear(); i += 4; continue;
            }
            cur += c;
        }
        parts.push_back(cur);
        return parts;
    }
    bool parse_condition(const string& part, Condition& c) {
        char q = 0; size_t eq = string::npos;
        for (size_t i = 0; i < part.size(); ++i) {
            if (q == 0 && (part[i] == '\'' || part[i] == '"')) q = part[i];
            else if (part[i] == q) q = 0;
            else if (q == 0 && part[i] == '=') { eq = i; break; }
        }
        if (eq == string::npos) return false;
        c.key = field_key(part.substr(0, eq));
        string v = trim(part.substr(eq + 1));
        if (v.size() >= 2 && (v[0] == '\'' || v[0] == '"') && v[v.size() - 1] == v[0]) v = v.substr(1, v.size() - 2);
        c.value = v;
        return !c.key.empty();
    }

    map<string, string> doctor_row(const DoctorRecord& r) {
        map<string, string> m;
        m["doctorid"] = get_field(r.id, DOC_ID_SIZE);
        m["doctorname"] = get_field(r.name, DOC_NAME_SIZE);
        m["doctorspecialty"] = get_field(r.specialty, DOC_SPEC_SIZE);
        m["doctoraddress"] = get_field(r.address, DOC_ADDR_SIZE);
        return m;
    }
    map<string, string> appointment_row(const AppointmentRecord& r) {
        map<string, string> m;
        m["appointmentid"] = get_field(r.id, APPT_ID_SIZE);
        m["doctorid"] = get_field(r.doctor_id, APPT_DOCID_SIZE);
        m["appointmentdate"] = get_field(r.date, APPT_DATE_SIZE);
        return m;
    }
    bool has_key(const vector<FieldDef>& fields, const string& key) {
        for (size_t i = 0; i < fields.size(); ++i) if (fields[i].key == key) return true;
        return false;
    }
    vector<string> sorted_unique(vector<string> v) {
        sort(v.begin(), v.end());
        v.erase(unique(v.begin(), v.end()), v.end());
        return v;
    }
}

bool run_query(const string& raw) {
    string q = collapse_ws(trim(raw));
    if (!q.empty() && q[q.size() - 1] == ';') q = trim(q.substr(0, q.size() - 1));
    string lq = lower(q);

    if (lq.compare(0, 7, "select ") != 0) { cout << "Syntax error: query must start with SELECT.\n"; return false; }
    size_t pFrom = lq.find(" from ");
    if (pFrom == string::npos) { cout << "Syntax error: missing FROM.\n"; return false; }
    size_t pWhere = lq.find(" where ", pFrom);

    string selectPart = trim(q.substr(7, pFrom - 7));
    size_t tableStart = pFrom + 6;
    string table = lower(trim(q.substr(tableStart, pWhere == string::npos ? string::npos : pWhere - tableStart)));
    string wherePart = pWhere == string::npos ? "" : trim(q.substr(pWhere + 7));

    // ---- table and its fields ----
    bool doctors;
    vector<FieldDef> fields;
    if (table == "doctors") {
        doctors = true;
        fields = { {"Doctor ID", "doctorid"}, {"Doctor Name", "doctorname"},
                   {"Doctor Specialty", "doctorspecialty"}, {"Doctor Address", "doctoraddress"} };
    } else if (table == "appointments") {
        doctors = false;
        fields = { {"Appointment ID", "appointmentid"}, {"Doctor ID", "doctorid"},
                   {"Appointment Date", "appointmentdate"} };
    } else { cout << "Error: unknown table '" << table << "' (use Doctors or Appointments).\n"; return false; }

    // ---- SELECT list ----
    vector<FieldDef> selected;
    if (lower(selectPart) == "all" || selectPart == "*") selected = fields;
    else {
        string cur; selectPart += ',';
        for (size_t i = 0; i < selectPart.size(); ++i) {
            if (selectPart[i] != ',') { cur += selectPart[i]; continue; }
            string k = field_key(cur); cur.clear();
            bool found = false;
            for (size_t j = 0; j < fields.size(); ++j)
                if (fields[j].key == k) { selected.push_back(fields[j]); found = true; }
            if (!found) { cout << "Error: unknown field in SELECT list.\n"; return false; }
        }
    }

    // ---- WHERE conditions ----
    vector<Condition> conds;
    if (pWhere != string::npos) {
        vector<string> parts = split_and(wherePart);
        for (size_t i = 0; i < parts.size(); ++i) {
            Condition c;
            if (!parse_condition(trim(parts[i]), c)) { cout << "Syntax error in WHERE condition.\n"; return false; }
            if (!has_key(fields, c.key)) { cout << "Error: unknown field in WHERE.\n"; return false; }
            conds.push_back(c);
        }
    }

    // ---- choose indexes ----
    vector<vector<string> > sets;
    vector<string> used;
    for (size_t i = 0; i < conds.size(); ++i) {
        const Condition& c = conds[i];
        if (doctors && c.key == "doctorid") {
            sets.push_back(primary_search_doctor(c.value) != -1 ? vector<string>(1, c.value) : vector<string>());
            used.push_back("Primary Index (Doctor ID)");
        } else if (doctors && c.key == "doctorname") {
            sets.push_back(secondary_search(doctors_name_index, c.value));
            used.push_back("Secondary Index (Doctor Name)");
        } else if (doctors && c.key == "doctorspecialty") {
            sets.push_back(secondary_search(doctors_specialty_index, c.value));
            used.push_back("Secondary Index (Doctor Specialty)");
        } else if (!doctors && c.key == "appointmentid") {
            sets.push_back(primary_search_appointment(c.value) != -1 ? vector<string>(1, c.value) : vector<string>());
            used.push_back("Primary Index (Appointment ID)");
        } else if (!doctors && c.key == "doctorid") {
            sets.push_back(secondary_search(appointments_doctor_index, c.value));
            used.push_back("Secondary Index (Doctor ID)");
        }
    }
    vector<string> ids;
    if (sets.empty()) {                                   // nothing indexed: all records via the Primary Index
        const PrimaryIndex& pi = doctors ? doctors_primary : appointments_primary;
        for (size_t i = 0; i < pi.size(); ++i) ids.push_back(pi[i].key);
        used.push_back("Primary Index (full scan)");
    } else {                                              // intersection of all index results
        ids = sorted_unique(sets[0]);
        for (size_t i = 1; i < sets.size(); ++i) {
            vector<string> next = sorted_unique(sets[i]), both;
            set_intersection(ids.begin(), ids.end(), next.begin(), next.end(), back_inserter(both));
            ids = both;
        }
    }

    // ---- read the records, re-check every condition, print ----
    vector<vector<string> > rows;
    for (size_t i = 0; i < ids.size(); ++i) {
        map<string, string> row;
        if (doctors) { DoctorRecord r{}; if (!get_doctor(ids[i], r)) continue; row = doctor_row(r); }
        else { AppointmentRecord r{}; if (!get_appointment(ids[i], r)) continue; row = appointment_row(r); }
        bool ok = true;
        for (size_t k = 0; k < conds.size(); ++k) if (row[conds[k].key] != conds[k].value) ok = false;
        if (!ok) continue;
        vector<string> out;
        for (size_t k = 0; k < selected.size(); ++k) out.push_back(row[selected[k].key]);
        rows.push_back(out);
    }

    cout << "[Index used:";
    for (size_t i = 0; i < used.size(); ++i) cout << (i ? " + " : " ") << used[i];
    cout << (used.size() > 1 && sets.size() > 1 ? ", intersection]" : "]") << "\n";
    for (size_t i = 0; i < selected.size(); ++i) cout << (i ? " | " : "") << selected[i].display;
    cout << "\n";
    for (size_t r = 0; r < rows.size(); ++r) {
        for (size_t i = 0; i < rows[r].size(); ++i) cout << (i ? " | " : "") << rows[r][i];
        cout << "\n";
    }
    cout << rows.size() << " row(s).\n";
    return true;
}
