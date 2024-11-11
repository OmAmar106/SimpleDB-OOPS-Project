   void deleteRows(const string &conditionColumn, const string &conditionOperator, const string &conditionValue) {
        int condIndex = find(columns.begin(), columns.end(), conditionColumn) - columns.begin();
        if (condIndex >= columns.size()) {
            cerr << "Error: Column not found.\n";
            return;
        }
