// Healthcare Management System - IS321 Assignment 1  (Owner: P5)
#include "include/RecordStructures.h"
#include "include/FileManager.h"
#include "include/DoctorsManager.h"
#include "include/AppointmentsManager.h"
#include "include/PrimaryIndex.h"
#include "include/SecondaryIndex.h"
#include "include/Operations.h"
#include "include/QueryEngine.h"
#include <iostream>
using namespace std;

// All modules are included here, so a plain  g++ main.cpp -o main  (VS Code Run button) builds the whole project.
// Do NOT also compile src/*.cpp separately, or every function would be defined twice.
#include "src/Utils.cpp"
#include "src/FileManager.cpp"
#include "src/LinkedList.cpp"
#include "src/PrimaryIndex.cpp"
#include "src/SecondaryIndex.cpp"
#include "src/DoctorsManager.cpp"
#include "src/AppointmentsManager.cpp"
#include "src/Operations.cpp"
#include "src/QueryEngine.cpp"

// menu numbers (change here together with print_menu if needed)
enum MenuChoice
{
    ADD_DOCTOR = 1,
    ADD_APPOINTMENT = 2,
    UPDATE_DOCTOR_NAME = 3,
    UPDATE_DOCTOR_ID = 4,
    UPDATE_APPT_DATE = 5,
    DELETE_APPOINTMENT = 6,
    DELETE_DOCTOR = 7,
    PRINT_DOCTOR = 8,
    PRINT_APPOINTMENT = 9,
    WRITE_QUERY = 10,
    EXIT_PROGRAM = 11
};

static bool ask(const string &prompt, string &out)
{
    cout << prompt;
    if (!getline(cin, out))
        return false;
    size_t a = out.find_first_not_of(" \t\r");
    size_t b = out.find_last_not_of(" \t\r");
    out = (a == string::npos) ? "" : out.substr(a, b - a + 1);
    return true;
}

// Initial data: 5 Doctors + 10 Appointments (only when the files are empty)
static void load_initial_data()
{
    if (!doctors_primary.empty() || !appointments_primary.empty())
        return;
    add_doctor("D001", "Ahmed Ali", "Cardiology", "Cairo");
    add_doctor("D002", "Sara Hassan", "Dermatology", "Giza");
    add_doctor("D003", "Omar Khaled", "Cardiology", "Alexandria");
    add_doctor("D004", "Mona Samir", "Pediatrics", "Cairo");
    add_doctor("D005", "Youssef Nabil", "Neurology", "Giza");
    const char *appts[10][3] = {
        {"A001", "D001", "2026-11-01"}, {"A002", "D001", "2026-11-02"}, {"A003", "D002", "2026-11-03"}, {"A004", "D002", "2026-11-04"}, {"A005", "D003", "2026-11-05"}, {"A006", "D003", "2026-11-06"}, {"A007", "D004", "2026-11-07"}, {"A008", "D004", "2026-11-08"}, {"A009", "D005", "2026-11-09"}, {"A010", "D001", "2026-11-10"}};
    for (int i = 0; i < 10; ++i)
        add_appointment(appts[i][0], appts[i][1], appts[i][2]);
    cout << "Initial data loaded (5 doctors, 10 appointments).\n";
}

static void print_menu()
{
    cout << "\n===== Healthcare Management System =====\n"
         << " 1. Add New Doctor\n 2. Add New Appointment\n 3. Update Doctor Name\n 4. Update Doctor ID\n"
         << " 5. Update Appointment Date\n 6. Delete Appointment\n 7. Delete Doctor\n"
         << " 8. Print Doctor Info\n 9. Print Appointment Info\n10. Write Query\n11. Exit\n"
         << "Choice: ";
}

int main()
{
    // ---- startup: files, then build every index from the data files (order matters) ----
    if (!create_doctors_file() || !create_appointments_file())
    {
        cout << "Cannot create the data files. Run the program from the project folder (it must contain data/ and indexes/).\n";
        return 1;
    }
    build_doctors_primary();      // P3
    build_appointments_primary(); // P3
    build_secondary_indexes();    // P4 (calls ll_reset first)

    cout << "Doctor Size: " << sizeof(DoctorRecord) << endl;
    cout << "Appointment Size: " << sizeof(AppointmentRecord) << endl;
    cout << "Header Size: " << sizeof(int) << endl;

    load_initial_data();

    string line, a, b, c, d;
    while (true)
    {
        print_menu();
        if (!getline(cin, line))
            break;
        int choice = -1;
        try
        {
            choice = stoi(line);
        }
        catch (...)
        {
            cout << "Please enter a number.\n";
            continue;
        }

        switch (choice)
        {
        case ADD_DOCTOR:
            if (!ask("Doctor ID: ", a) || !ask("Name: ", b) || !ask("Specialty: ", c) || !ask("Address: ", d))
                return 0;
            if (add_doctor(a, b, c, d))
                cout << "Doctor added successfully.\n";
            break;
        case ADD_APPOINTMENT:
            if (!ask("Appointment ID: ", a) || !ask("Doctor ID: ", b) || !ask("Date: ", c))
                return 0;
            if (add_appointment(a, b, c))
                cout << "Appointment added successfully.\n";
            break;
        case UPDATE_DOCTOR_NAME:
            if (!ask("Doctor ID: ", a) || !ask("New name: ", b))
                return 0;
            if (update_doctor_name(a, b))
                cout << "Doctor name updated.\n";
            break;
        case UPDATE_DOCTOR_ID:
            if (!ask("Old Doctor ID: ", a) || !ask("New Doctor ID: ", b))
                return 0;
            if (update_doctor_id(a, b))
                cout << "Doctor ID updated (appointments and indexes too).\n";
            break;
        case UPDATE_APPT_DATE:
            if (!ask("Appointment ID: ", a) || !ask("New date: ", b))
                return 0;
            if (update_appointment_date(a, b))
                cout << "Appointment date updated.\n";
            break;
        case DELETE_APPOINTMENT:
            if (!ask("Appointment ID: ", a))
                return 0;
            if (delete_appointment(a))
                cout << "Appointment deleted.\n";
            break;
        case DELETE_DOCTOR:
            if (!ask("Doctor ID: ", a))
                return 0;
            if (delete_doctor(a))
                cout << "Doctor deleted.\n";
            break;
        case PRINT_DOCTOR:
            if (!ask("Doctor ID: ", a))
                return 0;
            print_doctor(a);
            break;
        case PRINT_APPOINTMENT:
            if (!ask("Appointment ID: ", a))
                return 0;
            print_appointment(a);
            break;
        case WRITE_QUERY:
            if (!ask("Query: ", a))
                return 0;
            run_query(a);
            break;
        case EXIT_PROGRAM:
            cout << "Goodbye.\n";
            return 0;
        default:
            cout << "Invalid choice.\n";
        }
    }
    return 0;
}