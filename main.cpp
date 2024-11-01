#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <exception>

using namespace std;

// Struct to represent parsed queries
struct ParsedQuery {
    string command;
    string table;
    vector<string> columns;
    vector<string> values;
    string conditionColumn;
    string conditionOperator;
    string conditionValue;
};

// Tokenize function to split the query into tokens
vector<string> tokenize(const string& query) {
    istringstream stream(query);
    vector<string> tokens;
    string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

class Table {
private:
    string name;
    vector<string> columns;
    vector<vector<string>> rows;

    void writeToFile() {
        ofstream file(name + ".csv");
        if (!file.is_open()) {
            cerr << "Error: Could not open file for writing.\n";
            return;
        }
        for (size_t i = 0; i < columns.size(); ++i) {
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
        if (values.size() != columns.size()) {
            cerr << "Error: Row size does not match column count.\n";
            return;
        }
        rows.push_back(values);
        writeToFile();
    }

   // Update rows based on a condition
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

    // Delete rows based on a condition
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

        // Get indexes of the selected columns
        if (selectedCols.size() == 1 && selectedCols[0] == "*") {
            // If SELECT *, use all columns
            for (int i = 0; i < columns.size(); ++i) selectedIndexes.push_back(i);
        } else {
            // Otherwise, get indexes of specified columns
            for (const auto &col : selectedCols) {
                auto it = find(columns.begin(), columns.end(), col);
                if (it == columns.end()) {
                    cerr << "Error: Column not found.\n";
                    return {};
                }
                selectedIndexes.push_back(distance(columns.begin(), it));
            }
        }

        // Get condition column index if condition is specified
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

};

class Database {
private:
    unordered_map<string, Table> tables;

public:
    // void createTable(const string &tableName) {
    //     tables.emplace(tableName, Table(tableName));
    //     tables.at(tableName).createFile();  // Ensure file is created immediately
    //     tables.at(tableName).loadFromFile();
    // }
      
    void createTable(const string &tableName) {
        if (tables.find(tableName) == tables.end()) {
            // Use emplace to construct the Table object in place with tableName
            tables.emplace(tableName, Table(tableName));
            
            // Call methods after the table is created
            tables.at(tableName).createFile();
            tables.at(tableName).loadFromFile();
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
};

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
        if (parsedQuery.command == "CREATE") {
            if (tokens[i++] != "TABLE") throw invalid_argument("Expected TABLE keyword after CREATE.");
            parsedQuery.table = tokens[i++];
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
            if (tokens[i++] != "FROM") throw invalid_argument("Expected FROM keyword after DELETE.");
            parsedQuery.table = tokens[i++];

            // Optional WHERE clause
            if (i < tokens.size() && tokens[i] == "WHERE") {
                i++;
                parsedQuery.conditionColumn = tokens[i++];
                parsedQuery.conditionOperator = tokens[i++];
                parsedQuery.conditionValue = tokens[i++];
            }
        } 
        else {
            throw invalid_argument("Unsupported command.");
        }

        return parsedQuery;
    }

    void execute(const ParsedQuery &parsedQuery) {
        // cout<<parsedQuery.table<<endl;
        if(parsedQuery.command=="CREATE"){
            db.createTable(parsedQuery.table);
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
            for (const auto &col : parsedQuery.columns) {
                cout << col << "\t";
            }
            cout << "\n";

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
            table.deleteRows(parsedQuery.conditionColumn, parsedQuery.conditionOperator, parsedQuery.conditionValue);
        }
    }

};

int main() {
    Database db;
    QueryParser parser(db);
    string command;

    cout << "Welcome to SimpleDB! Type 'EXIT' to quit.\n";
    cout << "Available commands: CREATE, SELECT, INSERT, UPDATE, DELETE\n\n";

    while (true) {
        cout << "DB> ";
        getline(cin, command);
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

    cout << "Goodbye!\n";
    return 0;
}