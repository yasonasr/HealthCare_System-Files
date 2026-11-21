#ifndef QUERY_ENGINE_H
#define QUERY_ENGINE_H
// Owner: P5 - SELECT <fields|all> FROM <Doctors|Appointments> [WHERE f='v' [AND f='v']...];
#include <string>
using namespace std;

// Parses and runs one query, printing the result. Returns false on a syntax/semantic error.
bool run_query(const string& query);
#endif
