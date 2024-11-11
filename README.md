# SimpleDB

`SimpleDB` is a lightweight, file-based database project built in C++. It demonstrates basic operations such as storing, retrieving, updating, and deleting data in a structured format. The project is intended for educational purposes, providing insights into fundamental database operations without the complexity of a full-scale database system.

## Features

- Store data in a structured file format
- Retrieve data based on unique keys
- Update existing data entries
- Delete data entries
- Simple, efficient storage and retrieval mechanisms

## Commands
```bash
   >> CREATE TABLE tablename ( columnname datatype .. )
   >> INSERT INTO tablename VALUES data
   >> SELECT * FROM tablename
   >> SELECT * FROM tablename where columnname = data
   >> UPDATE tablename SET columnmname = newdata WHERE columnname = olddata
   >> DELETE FROM tablename WHERE columnname = data
   >> COPY FROM oldtable TO newtable
```

## Prerequisites

To compile and run `simpleDB`, you need:

- A C++ compiler (g++ or similar)
- Basic familiarity with C++ standard libraries

### Installation

1. Clone the repository:
```bash
   >> git clone https://github.com/your-username/simpleDB.git
   >> cd simpleDB
```

2. Compile the project:
```bash
   >> g++ main.cpp -o simpleDB
```
3. Run the program:
```bash
   >> ./simpleDB
```
