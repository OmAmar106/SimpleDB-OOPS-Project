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
```
```bash
   >> INSERT INTO tablename VALUES data
```
```bash
   >> SELECT * FROM tablename
```
```bash
   >> SELECT col1 , col2 .... FROM tablename
```
```bash
   >> SELECT * FROM tablename where columnname = data
```
```bash
   >> UPDATE tablename SET columnmname = newdata WHERE columnname = olddata
```
```bash
   >> DELETE FROM tablename WHERE columnname = data
```
```bash
   >> COPY FROM oldtable TO newtable
```
```bash
   >> DELETE tablename
```
```bash
   >> ISEQUAL tablename tablename1  
```
```bash
   >> SHOW TABLES
```
```bash
   >> --Comment
```
```bash
   >> EXIT
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
