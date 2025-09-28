#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

using namespace std;

// --- Structs ---
struct User {
    string username;
    string password;
    vector<string> genres;
    double membershipFee;
};

struct Title {
    string name;
    vector<string> genres;
    string language;
    double rating;
};

const string USERS_FILE = "users.txt";

// --- Utility Functions ---
static inline string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

vector<string> splitCSV(const string &s) {
    vector<string> out;
    string cur;
    stringstream ss(s);
    while (getline(ss, cur, ',')) {
        cur = trim(cur);
        if (!cur.empty()) out.push_back(cur);
    }
    return out;
}

string joinCSV(const vector<string> &arr) {
    string out;
    for (size_t i = 0; i < arr.size(); ++i) {
        if (i) out += ",";
        out += arr[i];
    }
    return out;
}

bool equalsIgnoreCase(const string &a, const string &b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); i++) {
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i]))
            return false;
    }
    return true;
}

string toLower(const string& s) {
    string result = s;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// --- User Management ---
vector<User> loadUsers() {
    vector<User> users;
    ifstream in(USERS_FILE);
    if (!in.is_open()) return users;
    string line;
    while (getline(in, line)) {
        if (trim(line).empty()) continue;
        stringstream ss(line);
        string username, password, genresCSV, feeStr;
        getline(ss, username, '|');
        getline(ss, password, '|');
        getline(ss, genresCSV, '|');
        getline(ss, feeStr, '|');
        User u;
        u.username = trim(username);
        u.password = trim(password);
        u.genres = splitCSV(genresCSV);
        try {
            u.membershipFee = stod(feeStr);
        } catch (const std::exception& e) {
            cerr << "Error parsing fee for user " << u.username << ": " << e.what() << endl;
            u.membershipFee = 0.0;
        }
        users.push_back(u);
    }
    return users;
}

void saveUsers(const vector<User> &users) {
    ofstream out(USERS_FILE);
    for (const auto &u : users) {
        out << u.username << "|"
            << u.password << "|"
            << joinCSV(u.genres) << "|"
            << u.membershipFee << "\n";
    }
}

// --- Catalogue ---
vector<Title> sampleCatalogue() {
    return {
        {"Midnight Runner", {"Action","Thriller", "Crime"}, "English", 8.1},
        {"Love in Kyoto", {"Romance","Drama"}, "Japanese", 7.4},
        {"Galaxy Guard", {"Sci-Fi","Action", "Adventure"}, "English", 8.7},
        {"Culinary Quest", {"Documentary", "Food"}, "Korean", 7.9},
        {"Haunted Apartment", {"Horror", "Thriller"}, "Malay", 6.9},
        {"The Last Samurai", {"Action", "History", "Drama"}, "Japanese", 8.5},
        {"Beyond the Stars", {"Sci-Fi", "Mystery"}, "English", 7.6},
        {"Rings of Power", {"Fantasy", "Adventure"}, "English", 8.0}
    };
}

// --- Authentication ---
bool usernameExists(const vector<User> &users, const string &username) {
    for (const auto &u : users) if (u.username == username) return true;
    return false;
}

void registerUser(vector<User> &users) {
    cout << "\n--- Register ---\n";
    string username, password;
    cout << "Enter username: ";
    getline(cin, username);
    if (usernameExists(users, username)) {
        cout << "Username already taken.\n";
        return;
    }
    cout << "Enter password: ";
    getline(cin, password);
    cout << "Enter preferred genres (comma-separated, e.g., Action, Drama, Sci-Fi): ";
    string g; getline(cin, g);

    double fee = 9.99;
    cout << "Membership fee is $" << fee << " per month (charged once at registration).\n";

    User u;
    u.username = username;
    u.password = password;
    u.genres = splitCSV(g);
    u.membershipFee = fee;

    users.push_back(u);
    saveUsers(users);
    cout << "Registration successful! Welcome, " << u.username << ".\n";
}

User* loginUser(vector<User> &users) {
    cout << "\n--- Login ---\n";
    string username, password;
    cout << "Username: "; getline(cin, username);
    cout << "Password: "; getline(cin, password);
    for (auto &u : users) {
        if (u.username == username && u.password == password) {
            cout << "Login successful.\n";
            return &u;
        }
    }
    cout << "Invalid login.\n";
    return nullptr;
}

// --- Features ---
void recommend(const User &user, const vector<Title> &catalogue) {
    cout << "\n--- Recommendations for " << user.username << " ---\n";
    int count = 0;
    for (const auto &t : catalogue) {
        bool match = false;
        for (const auto &g : t.genres) {
            for (const auto &ug : user.genres) {
                if (equalsIgnoreCase(g, ug)) {
                    match = true;
                    break;
                }
            }
            if (match) break;
        }

        if (match) {
            cout << "- " << t.name << " (Genres: " << joinCSV(t.genres) 
                 << ", Lang: " << t.language << ", Rating: " << t.rating << ")\n";
            count++;
        }
    }
    if (count == 0) {
        cout << "No current recommendations match your preferences. Try updating your genres!\n";
    }
}

void changePreferences(User &u, vector<User> &allUsers) {
    cout << "\n--- Change Preferences ---\n";
    cout << "Current genres: " << joinCSV(u.genres) << "\n";
    cout << "Enter new genres (comma-separated): ";
    string g; getline(cin, g);
    u.genres = splitCSV(g);

    saveUsers(allUsers);
    cout << "Preferences updated successfully.\n";
}

void searchCatalogue(const vector<Title> &catalogue) {
    cout << "\n--- Search & Filter Catalogue ---\n";
    string query, genreFilter;
    cout << "Enter title search term (e.g., 'Runner', leave blank to skip): ";
    getline(cin, query);
    cout << "Enter genre to filter by (e.g., 'Action', leave blank to skip): ";
    getline(cin, genreFilter);

    string lowerQuery = toLower(query);
    string lowerGenreFilter = toLower(genreFilter);

    vector<Title> results;

    for (const auto& t : catalogue) {
        bool matchesQuery = true;
        bool matchesGenre = true;

        if (!query.empty()) {
            string lowerName = toLower(t.name);
            if (lowerName.find(lowerQuery) == string::npos) {
                matchesQuery = false;
            }
        }

        if (!genreFilter.empty()) {
            matchesGenre = false;
            for (const auto& g : t.genres) {
                if (toLower(g).find(lowerGenreFilter) != string::npos) {
                    matchesGenre = true;
                    break;
                }
            }
        }

        if (matchesQuery && matchesGenre) {
            results.push_back(t);
        }
    }

    if (results.empty()) {
        cout << "\nNo titles found matching your criteria.\n";
    } else {
        cout << "\n--- Search Results (" << results.size() << " found) ---\n";
        sort(results.begin(), results.end(), [](const Title& a, const Title& b) {
            return a.rating > b.rating;
        });

        for (const auto& t : results) {
            cout << "- " << t.name << " | Genres: " << joinCSV(t.genres)
                 << " | Lang: " << t.language << " | Rating: " << t.rating << "\n";
        }
    }
}


// --- Menu ---
void userMenu(User &u, vector<User> &allUsers, const vector<Title> &catalogue) {
    while (true) {
        cout << "\nWelcome " << u.username << "!\n";
        cout << "1) See recommendations\n";
        cout << "2) Change preferences\n";
        cout << "3) Search/Filter catalogue\n";
        cout << "4) Logout\n";
        cout << "Choice: ";
        string c; getline(cin, c);
        if (c == "1") {
            recommend(u, catalogue);
        } else if (c == "2") {
            changePreferences(u, allUsers);
        } else if (c == "3") {
            searchCatalogue(catalogue);
        } else if (c == "4") {
            cout << "Logging out...\n";
            break;
        } else {
            cout << "Invalid choice. Please enter 1, 2, 3, or 4.\n";
        }
    }
}

// --- Main ---
int main() {
    vector<User> users = loadUsers();
    vector<Title> catalogue = sampleCatalogue();
    cout << "Welcome to Afterflix!\n";
    while (true) {
        cout << "\nMain menu:\n";
        cout << "1) Register\n2) Login\n3) Exit\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") {
            registerUser(users);
        } else if (c == "2") {
            User* u = loginUser(users);
            if (u) userMenu(*u, users, catalogue);
        } else if (c == "3") {
            cout << "Goodbye!\n";
            break;
        } else {
            cout << "Invalid choice.\n";
        }
    }
    return 0;
}
