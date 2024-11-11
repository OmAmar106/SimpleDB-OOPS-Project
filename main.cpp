#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>
#include <unordered_map>
#include <algorithm>
#include <exception>
#include <windows.h>

using namespace std;

struct ParsedQuery {
    bool delete1=false;
    string command;
    string table;
    vector<string> columns;
    vector<string> columnTypes;
    vector<string> values;
    string conditionColumn;
    string conditionOperator;
    string conditionValue;
    string other;
};

vector<string> tokenize(const string& query) {
    istringstream stream(query);
    vector<string> tokens;
    string token;
    while (stream >> token){
        tokens.push_back(token);
    }
    return tokens;
}

class Table {
private:
    string name;
    vector<string> columns;
    vector<string> columnTypes;
    vector<vector<string>> rows;

    void writeToFile() {
        ofstream file(name + ".csv");
        if (!file.is_open()) {
            cerr << "Error: Could not open file for writing.\n";
            return;
        }
        for (size_t i = 0; i < columns.size(); ++i) {
            // cout<<columns[i]<<endl;
            file << columns[i] << (i < columns.size() - 1 ? "," : "\n");
        }
        for (const auto& row : rows) {
            for (size_t i = 0; i < row.size(); ++i) {
                file << row[i] << (i < row.size() - 1 ? "," : "\n");
            }
        }
        file.close();
    }

public:
    Table(const string &name1,const Table &copytable){
        this->name = name1;

        for(int i=0;i<copytable.columns.size();i++){
            // cout<<i<<endl;
            this->columns.push_back(copytable.columns[i]);
        }
        for(int i=0;i<copytable.columnTypes.size();i++){
            this->columnTypes.push_back(copytable.columnTypes[i]);
        }
        for(int i=0;i<copytable.rows.size();i++){
            this->rows.push_back(copytable.rows[i]);
        }
        createFile();
    }
    //Table(const string &name) : name(name) {}
    Table(const string &name, const vector<string> &cols, const vector<string> &types) : name(name), columns(cols), columnTypes(types) {}
    Table(const string &name) : name(name) {}
    void createFile() {
        ofstream file(name + ".csv");
        if (!file.is_open()) {
            cerr << "Error: Could not create table file.\n";
            return;
        }
        writeToFile();
    }
    
    void loadFromFile() {
        ifstream file(name + ".csv");
        if (!file.is_open()) return;

        string line;
        bool isFirstLine = true;

        while (getline(file, line)) {
            istringstream stream(line);
            string cell;
            vector<string> row;

            while (getline(stream, cell, ',')) {
                if (isFirstLine) {
                    columns.push_back(cell);
                } else {
                    row.push_back(cell);
                }
            }
            if (!isFirstLine) rows.push_back(row);
            isFirstLine = false;
        }
        file.close();
    }

    void addColumn(const string &columnName) {
        columns.push_back(columnName);
        writeToFile();
    }

    void addRow(const vector<string> &values) {
        // cout<<values.size()<<columns.size()<<endl;
        // for(int i=0;i<values.size();i++){
        //     cout<<values[i]<<endl;
        // }
        if (values.size() != columns.size()) {
            cerr << "Error: Row size does not match column count.\n";
            return;
        }
        rows.push_back(values);
        writeToFile();
    }

    void updateRows(const string &column, const string &newValue, const string &conditionColumn, const string &conditionOperator, const string &conditionValue) {
        int colIndex = find(columns.begin(), columns.end(), column) - columns.begin();
        int condIndex = find(columns.begin(), columns.end(), conditionColumn) - columns.begin();

        if (colIndex >= columns.size() || condIndex >= columns.size()) {
            cerr << "Error: Column not found.\n";
            return;
        }

        for (auto &row : rows) {
            if ((conditionOperator == "=" && row[condIndex] == conditionValue) ||
                (conditionOperator == "!=" && row[condIndex] != conditionValue) ||
                (conditionOperator == "<" && row[condIndex] < conditionValue) ||
                (conditionOperator == ">" && row[condIndex] > conditionValue) ||
                (conditionOperator == "<=" && row[condIndex] <= conditionValue) ||
                (conditionOperator == ">=" && row[condIndex] >= conditionValue)) {
                row[colIndex] = newValue;
            }
        }
        writeToFile();
    }

    void deleteRows(const string &conditionColumn, const string &conditionOperator, const string &conditionValue) {
        int condIndex = find(columns.begin(), columns.end(), conditionColumn) - columns.begin();
        if (condIndex >= columns.size()) {
            cerr << "Error: Column not found.\n";
            return;
        }

        auto it = rows.begin();
        while (it != rows.end()) {
            bool conditionMet = (conditionOperator == "=" && (*it)[condIndex] == conditionValue) ||
                                (conditionOperator == "!=" && (*it)[condIndex] != conditionValue) ||
                                (conditionOperator == "<" && (*it)[condIndex] < conditionValue) ||
                                (conditionOperator == ">" && (*it)[condIndex] > conditionValue) ||
                                (conditionOperator == "<=" && (*it)[condIndex] <= conditionValue) ||
                                (conditionOperator == ">=" && (*it)[condIndex] >= conditionValue);
            if (conditionMet) {
                it = rows.erase(it);
            } else {
                ++it;
            }
        }
        writeToFile();
    }
    vector<vector<string>> selectRows(const vector<string> &selectedCols, const string &conditionColumn = "", const string &conditionOperator = "", const string &conditionValue = "") {
        vector<vector<string>> result;
        vector<int> selectedIndexes;

        if (selectedCols.size() == 1 && selectedCols[0] == "*") {
            // If SELECT *, use all columns
            for (int i = 0; i < columns.size(); ++i) selectedIndexes.push_back(i);
        } else {
            for (const auto &col : selectedCols) {
                auto it = find(columns.begin(), columns.end(), col);
                if (it == columns.end()) {
                    cerr << "Error: Column not found.\n";
                    return {};
                }
                selectedIndexes.push_back(distance(columns.begin(), it));
            }
        }

        int condIndex = -1;
        if (!conditionColumn.empty()) {
            auto it = find(columns.begin(), columns.end(), conditionColumn);
            if (it == columns.end()) {
                cerr << "Error: Condition column not found.\n";
                return {};
            }
            condIndex = distance(columns.begin(), it);
        }

        // Select rows that meet the condition (if any)
        for (const auto &row : rows) {
            if (condIndex == -1 || // No condition specified
                (conditionOperator == "=" && row[condIndex] == conditionValue) ||
                (conditionOperator == "!=" && row[condIndex] != conditionValue) ||
                (conditionOperator == "<" && row[condIndex] < conditionValue) ||
                (conditionOperator == ">" && row[condIndex] > conditionValue) ||
                (conditionOperator == "<=" && row[condIndex] <= conditionValue) ||
                (conditionOperator == ">=" && row[condIndex] >= conditionValue)) {
                
                vector<string> selectedRow;
                for (int index : selectedIndexes) {
                    selectedRow.push_back(row[index]);
                }
                result.push_back(selectedRow);
            }
        }
        return result;
    }
    vector<string> getcols(){
        return columns;
    }
    bool operator==(const Table& tablename){
        // string name;
        // vector<string> columns;
        // vector<string> columnTypes;
        // vector<vector<string>> rows;
        if (columns.size()==tablename.columns.size() && columnTypes.size()==tablename.columnTypes.size() && rows.size()==tablename.rows.size()){
            for (int i=0;i<columns.size();i++){
                if(columns[i]!=tablename.columns[i]){
                    return false;
                }
            }
            for (int i=0;i<columnTypes.size();i++){
                if(columnTypes[i]!=tablename.columnTypes[i]){
                    return false;
                }
            }
            for(int i=0;i<rows.size();i++){
                if(rows[i]!=tablename.rows[i]){
                    return false;
                }
                for(int j=0;j<rows[i].size();j++){
                    if(rows[i][j]!=tablename.rows[i][j]){
                        return false;
                    }
                }
            }
        }
        else{
            return false;
        }

        return true;
    }
};

class Database {
private:
    unordered_map<string, Table> tables;
    static int count;

public:
    Database() {
        loadExistingTables();
        // for(auto it:tables){
        //     cout<<it.first<<endl;
        // }
    }

    static int Tablecount(){
        return count;
    }

    void print(){
        int count1 = 1;
        cout<<endl<<"Number of Tables : "<<count<<endl;
        for(auto it:tables){
            cout<<count1<<". "<<it.first<<endl;
            count1 += 1;
        }
        cout<<endl;
    }
    // Load all tables from existing files
    void loadExistingTables(){
        string directoryPath = "./*.csv";
        WIN32_FIND_DATAA findFileData;    // Use ANSI version
        HANDLE hFind = FindFirstFileA(directoryPath.c_str(), &findFileData);  // Use FindFirstFileA for ANSI

        if (hFind == INVALID_HANDLE_VALUE) {
            cerr << "Failed to open directory or no CSV files found." << endl;
            return;
        }

        do {
            count += 1;
            string fileName = findFileData.cFileName;
            
            if (fileName.size() >= 4 && fileName.substr(fileName.size() - 4) == ".csv") {
                string tableName = fileName.substr(0, fileName.find_last_of('.'));
                
                tables.emplace(tableName, Table(tableName));
                tables.at(tableName).loadFromFile();
            }
            
        } while (FindNextFileA(hFind, &findFileData) != 0);

        FindClose(hFind);
    }
    // void createTable(const string &tableName) {
    //     tables.emplace(tableName, Table(tableName));
    //     tables.at(tableName).createFile();  // Ensure file is created immediately
    //     tables.at(tableName).loadFromFile();
    // }
      
    // void createTable(const string &tableName) {
    //     if (tables.find(tableName) == tables.end()) {
    //         // Use emplace to construct the Table object in place with tableName
    //         tables.emplace(tableName, Table(tableName));
            
    //         // Call methods after the table is created
    //         tables.at(tableName).createFile();
    //         tables.at(tableName).loadFromFile();
    //     } else {
    //         cout << "Error: Table '" << tableName << "' already exists.\n";
    //     }
    // }
    void createTable(const string &tableName, const vector<string> &columns, const vector<string> &columnTypes) {
        if (tables.find(tableName) == tables.end()) {
            tables.emplace(tableName, Table(tableName, columns, columnTypes));
            tables.at(tableName).createFile();
        } else {
            cout << "Error: Table '" << tableName << "' already exists.\n";
        }
    }

    Table& getTable(const string &tableName) {
        if (tables.find(tableName) == tables.end()) {
            throw invalid_argument("Table does not exist.");
        }
        return tables.at(tableName);
    }

    void deletetable(string table){
        auto it = tables.find(table);
        if(it != tables.end()){
            tables.erase(it);
            remove((table+".csv").c_str());
        }
    }
};

int Database::count = 0;

class QueryParser {
private:
    Database &db;

public:
    QueryParser(Database &database) : db(database) {}

    ParsedQuery parse(const string &query) {
        ParsedQuery parsedQuery;
        auto tokens = tokenize(query);

        if (tokens.empty()) throw invalid_argument("Query cannot be empty.");

        // for(int i=0;i<tokens.size();i++){
        //     cout<<tokens[i]<<endl;
        // }
        parsedQuery.command = tokens[0];
        size_t i = 1;

        // Command-specific parsing logic
        // if (parsedQuery.command == "CREATE") {
        //     if (tokens[i++] != "TABLE") throw invalid_argument("Expected TABLE keyword after CREATE.");
        //     parsedQuery.table = tokens[i++];
        // } 
        if (parsedQuery.command == "CREATE") {
            if (tokens[i++] != "TABLE") throw invalid_argument("Expected TABLE keyword after CREATE.");
            parsedQuery.table = tokens[i++];

            if (tokens[i++] != "(") throw invalid_argument("Expected '(' after table name.");

                // Parse column names and types
                while (tokens[i] != ")") {
                    parsedQuery.columns.push_back(tokens[i++]);
                    parsedQuery.columnTypes.push_back(tokens[i++].substr(0, tokens[i].length() - 1));  // Strip trailing ','

                    if (tokens[i] == ")") break;  // End parsing on ')'
                    else if (tokens[i++] != ",") throw invalid_argument("Expected ',' between columns.");
                }
        }
        else if (parsedQuery.command == "INSERT") {
            if (tokens[i++] != "INTO") throw invalid_argument("Expected INTO keyword after INSERT.");
            parsedQuery.table = tokens[i++];
            if (tokens[i++] != "VALUES") throw invalid_argument("Expected VALUES keyword after table name.");

            // Collect values
            for (; i < tokens.size(); ++i) {
                parsedQuery.values.push_back(tokens[i]);
            }
        } 
        else if (parsedQuery.command == "SELECT") {
            // Check for columns or "*" for all
            while (tokens[i] != "FROM") {
                parsedQuery.columns.push_back(tokens[i++]);
            }
            i++;  // Skip "FROM"
            parsedQuery.table = tokens[i++];
            
            // Optional WHERE clause
            if (i < tokens.size() && tokens[i] == "WHERE") {
                i++;
                parsedQuery.conditionColumn = tokens[i++];
                parsedQuery.conditionOperator = tokens[i++];
                parsedQuery.conditionValue = tokens[i++];
            }
        } 
        else if (parsedQuery.command == "UPDATE") {
            parsedQuery.table = tokens[i++];
            if (tokens[i++] != "SET") throw invalid_argument("Expected SET keyword after table name.");
            
            // Column to update and new value
            parsedQuery.columns.push_back(tokens[i++]);
            if (tokens[i++] != "=") throw invalid_argument("Expected = after column name.");
            parsedQuery.values.push_back(tokens[i++]);

            // Optional WHERE clause
            if (i < tokens.size() && tokens[i] == "WHERE") {
                i++;
                parsedQuery.conditionColumn = tokens[i++];
                parsedQuery.conditionOperator = tokens[i++];
                parsedQuery.conditionValue = tokens[i++];
            }
        } 
        else if (parsedQuery.command == "DELETE") {
            if (tokens[i++] != "FROM"){
                parsedQuery.table = tokens[i-1];
                parsedQuery.delete1 = true;
                return parsedQuery;
            }
            parsedQuery.table = tokens[i++];

            // Optional WHERE clause
            if (i < tokens.size() && tokens[i] == "WHERE") {
                i++;
                parsedQuery.conditionColumn = tokens[i++];
                parsedQuery.conditionOperator = tokens[i++];
                parsedQuery.conditionValue = tokens[i++];
            }
        } 
        else if (parsedQuery.command == "COPY") {
            if (tokens[i++] != "FROM") throw invalid_argument("Expected FROM keyword after DELETE.");
            parsedQuery.table = tokens[i++];
            if (tokens[i++] != "TO") throw invalid_argument("Expected FROM keyword after DELETE.");
            // Optional WHERE clause
            parsedQuery.other = tokens[i++];
            //ab is table ko find karna hain
            
            // if (i < tokens.size() && tokens[i] == "TO") {
            //     i++;
            //     parsedQuery.conditionColumn = tokens[i++];
            //     parsedQuery.conditionOperator = tokens[i++];
            //     parsedQuery.conditionValue = tokens[i++];
            // }
        }
        else if(parsedQuery.command=="ISEQUAL"){
            parsedQuery.table = tokens[i++];
            parsedQuery.other = tokens[i++];
        }
        else if(parsedQuery.command=="SHOW"){
            parsedQuery.other = "TABLE";
        }
        else {
            throw invalid_argument("Unsupported command.");
        }

        return parsedQuery;
    }

    void execute(const ParsedQuery &parsedQuery) {
        // cout<<parsedQuery.table<<endl;
        // if(parsedQuery.command=="CREATE"){
        //     db.createTable(parsedQuery.table);
        //     return;
        // }
        if (parsedQuery.command=="SHOW" && parsedQuery.other=="TABLE"){
            db.print();
            return;;
        }
        if (parsedQuery.command == "CREATE") {
            db.createTable(parsedQuery.table, parsedQuery.columns, parsedQuery.columnTypes);
            cout << "Table '" << parsedQuery.table << "' created successfully.\n";
            return;
        }

        if (parsedQuery.command == "COPY") {
            Table table(parsedQuery.other,db.getTable(parsedQuery.table));
            return;
        }

        Table &table = db.getTable(parsedQuery.table);

        if (parsedQuery.command == "SELECT") {
            // Call selectRows in Table and display the result
            auto selectedRows = table.selectRows(parsedQuery.columns, parsedQuery.conditionColumn,
                                                parsedQuery.conditionOperator, parsedQuery.conditionValue);
            if (selectedRows.empty()) {
                cout << "No results found.\n";
                return;
            }

            // Print the selected rows
            if (parsedQuery.columns[0]=="*"){
                for (const auto &col : table.getcols()) {
                    cout << col << "\t";
                }
                cout<<endl;
                for (const auto &col : table.getcols()) {
                    for(int j=0;j<col.size();j++){
                        cout<<"-";
                    }
                    cout << "\t";
                }
            }
            else{
                for (const auto &col : parsedQuery.columns) {
                    cout << col << "\t";
                }
                cout<<endl;
                for (const auto &col : parsedQuery.columns) {
                    for(int j=0;j<col.size();j++){
                        cout<<"-";
                    }
                    cout << "\t";
                }
            }
            cout<<endl;

            for (const auto &row : selectedRows) {
                for (const auto &cell : row) {
                    cout << cell << "\t";
                }
                cout << "\n";
            }
        } 
        else if (parsedQuery.command == "INSERT") {
            table.addRow(parsedQuery.values);
        } 
        else if (parsedQuery.command == "UPDATE") {
            table.updateRows(parsedQuery.columns[0], parsedQuery.values[0], parsedQuery.conditionColumn, 
                            parsedQuery.conditionOperator, parsedQuery.conditionValue);
        } 
        else if (parsedQuery.command == "DELETE") {
            if(parsedQuery.delete1){
                db.deletetable(parsedQuery.table);
                return;
            }
            table.deleteRows(parsedQuery.conditionColumn, parsedQuery.conditionOperator, parsedQuery.conditionValue);
        }
        else if(parsedQuery.command=="ISEQUAL"){
            if(db.getTable(parsedQuery.table)==db.getTable(parsedQuery.other)){
                cout<<"They are same."<<endl;
            }
            else{
                cout<<"They are Different."<<endl;
            }
        } 
    }

};

int main() {
    string command;

    cout << "Welcome to SimpleDB! Type 'EXIT' to quit.\n";
    cout << "Available commands: CREATE, SELECT, INSERT, UPDATE, DELETE\n\n";

    while (true) {
        Database db;
        QueryParser parser(db);
        cout << "DB> ";
        getline(cin, command);
        
        if(command.size()>=2 && command[0]=='-' && command[1]=='-'){

            cout << "Query executed successfully.\n";
            continue;
        }

        if (command == "EXIT") break;

        try {
            ParsedQuery parsedQuery = parser.parse(command);
            parser.execute(parsedQuery);
            cout << "Query executed successfully.\n";
        }
        catch (const invalid_argument &e) {
            cerr << "Error: " << e.what() << "\n";
        }
        catch (const exception &e) {
            cerr << "An error occurred: " << e.what() << "\n";
        }
    }

    cout << "Thank You for Using!\n";
    return 0;
}