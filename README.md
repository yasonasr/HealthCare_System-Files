# HealthCare_System-Files
Simple Healthcare Management System in which an administrator can manage doctors and their appointments
(IS321 Assignment 1: binary data files, AVAIL LIST, Primary/Secondary Indexes, linked lists, query engine).

## Build and run
Open the project folder in VS Code, open `main.cpp` and press Run (or from a terminal in the project folder):

    g++ main.cpp -o main
    ./main

`main.cpp` includes all the modules in `src/`, so no other flags or files are needed.
The folders `data/` and `indexes/` must exist next to `main.cpp` (they are in the repo); the files inside are created automatically
and all indexes are rebuilt from the data files at every start.

## Module owners
| Person | Files |
|---|---|
| P1 | FileManager, DoctorsManager |
| P2 | RecordStructures, LinkedList, AppointmentsManager |
| P3 | Utils (Binary Search), PrimaryIndex |
| P4 | SecondaryIndex |
| P5 | Operations, QueryEngine, main |
