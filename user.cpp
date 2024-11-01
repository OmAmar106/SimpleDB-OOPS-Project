#include<bits/stdc++.h>
using namespace std;

class User {
public:
    string name;
    string username;
    string password;
    vector<string> history;  // Keeps track of login history

    User() {}

    User(string name, string username, string password) : name(name), username(username), password(password) {}

    void addHistory(string timestamp) {
        history.push_back(timestamp);
    }

    void displayUserInfo() {
        cout << "Name: " << name << endl;
        cout << "Username: " << username << endl;
        cout << "Login History: " << endl;
        for (const string& time : history) {
            cout << time << endl;
        }
    }

    // Method to display data for editing
    void editUserInfo() {
        cout << "Enter new name: ";
        cin >> name;
        cout << "Enter new password: ";
        cin >> password;
    }
};

class FileHandler {
public:
    static vector<User> readUsersFromFile(string fileName) {
        vector<User> users;
        ifstream file(fileName);
        if (!file.is_open()) {
            cerr << "Error opening file!" << endl;
            return users;
        }

        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string name, username, password, history;
            getline(ss, name, ',');
            getline(ss, username, ',');
            getline(ss, password, ',');
            
            User user(name, username, password);

            // Get user history (if any)
            while (getline(ss, history, ',')) {
                user.addHistory(history);
            }

            users.push_back(user);
        }
        file.close();
        return users;
    }

    static void writeUsersToFile(vector<User>& users, string fileName) {
        ofstream file(fileName);
        if (!file.is_open()) {
            cerr << "Error writing to file!" << endl;
            return;
        }

        for (User& user : users) {
            file << user.name << "," << user.username << "," << user.password;
            for (string& entry : user.history) {
                file << "," << entry;
            }
            file << endl;
        }

        file.close();
    }
};

class DBMS {
private:
    vector<User> users;
    string fileName;

    User* login(string username, string password) {
        for (User& user : users) {
            if (user.username == username && user.password == password) {
                return &user;
            }
        }
        return nullptr;
    }

public:
    DBMS(string fileName) : fileName(fileName) {
        users = FileHandler::readUsersFromFile(fileName);
    }

    ~DBMS() {
        FileHandler::writeUsersToFile(users, fileName);
    }

    void createUser() {
        string name, username, password;
        cout << "Enter name: ";
        cin >> name;
        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;

        User newUser(name, username, password);
        users.push_back(newUser);
        cout << "User created successfully!" << endl;
    }

    void viewUsers() {
        for (User& user : users) {
            user.displayUserInfo();
        }
    }

    void updateUser() {
        string username, password;
        cout << "Enter username to login: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;

        User* user = login(username, password);
        if (user) {
            cout << "Login successful!" << endl;
            string timestamp = "Login at XYZ time";  // Example timestamp (You can implement actual timestamp)
            user->addHistory(timestamp);
            user->editUserInfo();
            cout << "User information updated." << endl;
        } else {
            cout << "Invalid login credentials!" << endl;
        }
    }

    void deleteUser() {
        string username, password;
        cout << "Enter username to delete: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;

        for (auto it = users.begin(); it != users.end(); ++it) {
            if (it->username == username && it->password == password) {
                users.erase(it);
                cout << "User deleted successfully!" << endl;
                return;
            }
        }
        cout << "Invalid credentials! User not found." << endl;
    }
};

int main() {
    DBMS dbms("users.txt");

    int choice;
    do {
        cout << "\n=== DBMS Menu ===\n";
        cout << "1. Create User\n";
        cout << "2. View All Users\n";
        cout << "3. Update User\n";
        cout << "4. Delete User\n";
        cout << "5. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            dbms.createUser();
            break;
        case 2:
            dbms.viewUsers();
            break;
        case 3:
            dbms.updateUser();
            break;
        case 4:
            dbms.deleteUser();
            break;
        case 5:
            cout << "Exiting program..." << endl;
            break;
        default:
            cout << "Invalid choice!" << endl;
        }
    } while (choice != 5);

    return 0;
}