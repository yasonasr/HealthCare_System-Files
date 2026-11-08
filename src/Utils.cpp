#include "../include/Utils.h"
using namespace std;

string field_to_string(const char* field, int max_len) {
    int len = 0;
    while (len < max_len && field[len] != '\0'){ 
        len++; }
    while (len > 0 && field[len - 1] == ' '){ 
        len--;}  
    return string(field, len);
}