#include <iostream>
#include <iomanip>
#include <mysql.h>
#include <vector>
#include <cstdlib>
#include <sstream>
#include<set>
#include <fstream>
#include <string>
#include <limits>
#include <stdexcept>
#include <ctime>

using namespace std;

class MySQL;





class Room {
public:
    int Type;
    int Room_No;
    bool Food;

    Room() {}

    Room(int type, int room_no, bool food = false)
        : Type(type), Room_No(room_no), Food(food) {}

    void Out() const {
        cout << "Room No: " << Room_No << endl;
        cout << "Type: " << Type << endl;
        cout << "Food Included: " << (Food ? "Yes" : "No") << endl;
    }

    void SetData(int type, int room_no, bool food = false) {
        Type = type;
        Room_No = room_no;
        Food = food;
    }
};

class Customer {
public:
    string Name;
    Room Obj;
    string Date_Entry;
    string Date_Exit;
    int Id;
    int PhoneNo;

    Customer() {}

    Customer(const string& name, const Room& room, const string& entryDate, const string& exitDate, int id, int phoneNo)
        : Name(name), Obj(room), Date_Entry(entryDate), Date_Exit(exitDate), Id(id), PhoneNo(phoneNo) {}

    // Output function
    void Out() {
        cout << "Customer ID: " << Id << endl;
        cout << "Name: " << Name << endl;
        cout << "Phone No: " << PhoneNo << endl;
        cout << "Date of Entry: " << Date_Entry << endl;
        cout << "Date of Exit: " << Date_Exit << endl;
        Obj.Out(); // Call the Room's Out method
    }

    void SetDetails(const Room& room) {
        Obj = room;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clean the input buffer
        cout << "Enter Name: ";
        getline(cin, Name);
        cout << "Enter Entry Date (YYYY-MM-DD): ";
        cin >> Date_Entry;
        cout << "Enter Exit Date (YYYY-MM-DD): ";
        cin >> Date_Exit;
        cout << "Enter Id: ";
        cin >> Id;
        cout << "Enter Phone No: ";
        cin >> PhoneNo;

        // Validate Date Format (if necessary)
        if (!isValidDate(Date_Entry) || !isValidDate(Date_Exit)) {
            cout << "Invalid date format. Please use YYYY-MM-DD." << endl;
        }
    }

    bool isValidDate(const string& date) {
        // Simple date format validation (e.g., YYYY-MM-DD)
        if (date.length() != 10 || date[4] != '-' || date[7] != '-') {
            return false;
        }
        // Further validation logic can be added (e.g., checking if the date is valid)
        return true;
    }
};



void Save_Customer(const Customer& C, const string& filename, MySQL& db) {
    ofstream outFile(filename.c_str(), ios::binary | ios::app);
    if (outFile.is_open()) {
        // Write Customer data to file (same as previous code)
        size_t nameLength = C.Name.size();
        outFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        outFile.write(C.Name.c_str(), nameLength);

        size_t entryDateLength = C.Date_Entry.size();
        outFile.write(reinterpret_cast<const char*>(&entryDateLength), sizeof(entryDateLength));
        outFile.write(C.Date_Entry.c_str(), entryDateLength);

        size_t exitDateLength = C.Date_Exit.size();
        outFile.write(reinterpret_cast<const char*>(&exitDateLength), sizeof(exitDateLength));
        outFile.write(C.Date_Exit.c_str(), exitDateLength);

        outFile.write(reinterpret_cast<const char*>(&C.Id), sizeof(C.Id));
        outFile.write(reinterpret_cast<const char*>(&C.PhoneNo), sizeof(C.PhoneNo));

        outFile.write(reinterpret_cast<const char*>(&C.Obj.Type), sizeof(C.Obj.Type));
        outFile.write(reinterpret_cast<const char*>(&C.Obj.Room_No), sizeof(C.Obj.Room_No));
        outFile.write(reinterpret_cast<const char*>(&C.Obj.Food), sizeof(C.Obj.Food));

        cout << "Customer object saved to file successfully!\n";

        // Save to MySQL database as well
        db.saveCustomerToDatabase(C);

    } else {
        cout << "Error opening file for writing.\n";
    }
}


void Load_Customer(const string& filename, int ID, Customer& C, MySQL& db) {
    ifstream inFile(filename.c_str(), ios::binary);
    if (!inFile.is_open()) {
        cout << "Error opening file for reading." << endl;
        return;
    }

    try {
        while (true) {
            size_t nameLength;
            if (!inFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength))) break;
            string name(nameLength, ' ');
            if (!inFile.read(&name[0], nameLength)) break;

            size_t entryDateLength, exitDateLength;
            if (!inFile.read(reinterpret_cast<char*>(&entryDateLength), sizeof(entryDateLength))) break;
            string entryDate(entryDateLength, ' ');
            if (!inFile.read(&entryDate[0], entryDateLength)) break;

            if (!inFile.read(reinterpret_cast<char*>(&exitDateLength), sizeof(exitDateLength))) break;
            string exitDate(exitDateLength, ' ');
            if (!inFile.read(&exitDate[0], exitDateLength)) break;

            int id, phoneNo;
            if (!inFile.read(reinterpret_cast<char*>(&id), sizeof(id))) break;
            if (!inFile.read(reinterpret_cast<char*>(&phoneNo), sizeof(phoneNo))) break;

            int type, roomNo;
            bool food;
            if (!inFile.read(reinterpret_cast<char*>(&type), sizeof(type))) break;
            if (!inFile.read(reinterpret_cast<char*>(&roomNo), sizeof(roomNo))) break;
            if (!inFile.read(reinterpret_cast<char*>(&food), sizeof(food))) break;

            Room loadedRoom(type, roomNo, food);
            Customer loadedCustomer(name, loadedRoom, entryDate, exitDate, id, phoneNo);

            // Check if the loaded customer matches the search ID
            if (loadedCustomer.Id == ID) {
                C = loadedCustomer;
                C.Out();
                cout << "Customer found in file!" << endl;
                return;
            }
        }

        // If not found in file, try loading from MySQL database
        db.loadCustomerFromDatabase(ID, C);

    } catch (const exception& e) {
        cout << "Exception during file operation: " << e.what() << endl;
    }
}



void delete_Customer(const string& filename, int searchID, MySQL& db) {
    ifstream inFile(filename.c_str(), ios::binary);
    string Filename = "temp.dat";
    ofstream tempFile(Filename.c_str(), ios::binary);

    if (!inFile.is_open() || !tempFile.is_open()) {
        cout << "Error opening file.\n";
        return;
    }

    bool customerFound = false;

    while (!inFile.eof()) {
        size_t nameLength;
        inFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
        if (inFile.eof()) break;  // Prevent reading beyond EOF
        string name(nameLength, ' ');
        inFile.read(&name[0], nameLength);

        size_t entryDateLength, exitDateLength;
        inFile.read(reinterpret_cast<char*>(&entryDateLength), sizeof(entryDateLength));
        string entryDate(entryDateLength, ' ');
        inFile.read(&entryDate[0], entryDateLength);

        inFile.read(reinterpret_cast<char*>(&exitDateLength), sizeof(exitDateLength));
        string exitDate(exitDateLength, ' ');
        inFile.read(&exitDate[0], exitDateLength);

        int id, phoneNo;
        inFile.read(reinterpret_cast<char*>(&id), sizeof(id));
        inFile.read(reinterpret_cast<char*>(&phoneNo), sizeof(phoneNo));

        int type, roomNo;
        bool food;
        inFile.read(reinterpret_cast<char*>(&type), sizeof(type));
        inFile.read(reinterpret_cast<char*>(&roomNo), sizeof(roomNo));
        inFile.read(reinterpret_cast<char*>(&food), sizeof(food));

        if (id == searchID) {
            customerFound = true;
            continue;  // Skip this record
        }

        tempFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        tempFile.write(name.c_str(), nameLength);
        tempFile.write(reinterpret_cast<const char*>(&entryDateLength), sizeof(entryDateLength));
        tempFile.write(entryDate.c_str(), entryDateLength);
        tempFile.write(reinterpret_cast<const char*>(&exitDateLength), sizeof(exitDateLength));
        tempFile.write(exitDate.c_str(), exitDateLength);
        tempFile.write(reinterpret_cast<const char*>(&id), sizeof(id));
        tempFile.write(reinterpret_cast<const char*>(&phoneNo), sizeof(phoneNo));
        tempFile.write(reinterpret_cast<const char*>(&type), sizeof(type));
        tempFile.write(reinterpret_cast<const char*>(&roomNo), sizeof(roomNo));
        tempFile.write(reinterpret_cast<const char*>(&food), sizeof(food));
    }

    // Copy the temp file back to the main file
    ifstream tempInFile("temp.dat", ios::binary);
    ofstream mainFile(filename.c_str(), ios::binary | ios::trunc);  // Overwrite the original file

    if (tempInFile.is_open() && mainFile.is_open()) {
        mainFile << tempInFile.rdbuf();

        if (customerFound) {
            cout << "Customer record deleted from file.\n";
            // Delete customer from MySQL database as well
            db.deleteCustomerFromDatabase(searchID);
        } else {
            cout << "Customer with ID " << searchID << " not found.\n";
        }
    } else {
        cout << "Error processing files.\n";
    }
}

int stringToInt(const std::string& str, MySQL& db) {
    std::stringstream ss(str);
    int value;
    ss >> value;
    if (ss.fail() || !ss.eof()) {
        // Log invalid conversion to the database
        std::string query = "INSERT INTO error_log (error_type, error_message, timestamp) VALUES ('Invalid Integer Conversion', 'Failed to convert string: " + str + "', CURDATE())";
        if (mysql_query(db.con, query.c_str()) != 0) {
            std::cerr << "Error logging invalid conversion: " << mysql_error(db.con) << std::endl;
        }
        throw std::invalid_argument("Invalid integer value in string: " + str);
    }
    return value;
}
// Helper function to parse date strings (manual parsing without std::get_time or std::stoi)
std::tm parseDate(const std::string& date, MySQL& db) {
    if (date.length() != 10 || date[4] != '-' || date[7] != '-') {
        // Log invalid date format error to the database
        std::string query = "INSERT INTO error_log (error_type, error_message, timestamp) VALUES ('Invalid Date Format', 'Failed to parse date: " + date + "', CURDATE())";
        if (mysql_query(db.con, query.c_str()) != 0) {
            std::cerr << "Error logging invalid date format: " << mysql_error(db.con) << std::endl;
        }
        throw std::invalid_argument("Invalid date format. Use YYYY-MM-DD.");
    }

    std::tm tm = {};
    try {
        tm.tm_year = stringToInt(date.substr(0, 4), db) - 1900; // Year since 1900
        tm.tm_mon = stringToInt(date.substr(5, 2), db) - 1;     // Months are 0-based
        tm.tm_mday = stringToInt(date.substr(8, 2), db);        // Day of the month
    } catch (const std::invalid_argument& e) {
        // Log invalid date component error to the database
        std::string query = "INSERT INTO error_log (error_type, error_message, timestamp) VALUES ('Invalid Date Components', 'Failed to parse components of date: " + date + "', CURDATE())";
        if (mysql_query(db.con, query.c_str()) != 0) {
            std::cerr << "Error logging invalid date components: " << mysql_error(db.con) << std::endl;
        }
        throw std::invalid_argument("Invalid date components in the string: " + date);
    }

    return tm;
}

int daysBetween(const std::string& startDate, const std::string& endDate, MySQL& db) {
    try {
        // Parse the dates into std::tm
        std::tm startTm = parseDate(startDate, db);
        std::tm endTm = parseDate(endDate, db);

        // Convert std::tm to time_t
        time_t start = std::mktime(&startTm);
        time_t end = std::mktime(&endTm);

        if (start == -1 || end == -1) {
            // Log time conversion error
            std::string query = "INSERT INTO error_log (error_type, error_message, timestamp) VALUES ('Date Conversion Error', 'Failed to convert date to time_t for start: " + startDate + " or end: " + endDate + "', CURDATE())";
            if (mysql_query(db.con, query.c_str()) != 0) {
                std::cerr << "Error logging date conversion error: " << mysql_error(db.con) << std::endl;
            }
            throw std::runtime_error("Failed to convert date to time_t.");
        }

        // Calculate the difference in days
        double difference = std::difftime(end, start) / (60 * 60 * 24);
        return static_cast<int>(difference);
    } catch (const std::invalid_argument& e) {
        // Log invalid argument error for dates
        std::string query = "INSERT INTO error_log (error_type, error_message, timestamp) VALUES ('Invalid Date Argument', 'Invalid argument: " + std::string(e.what()) + "', CURDATE())";
        if (mysql_query(db.con, query.c_str()) != 0) {
            std::cerr << "Error logging invalid date argument: " << mysql_error(db.con) << std::endl;
        }
        std::cerr << "Error: " << e.what() << std::endl;
        return -1; // Indicate error
    } catch (const std::runtime_error& e) {
        // Log runtime error for time difference calculation
        std::string query = "INSERT INTO error_log (error_type, error_message, timestamp) VALUES ('Runtime Error', 'Error in calculating time difference: " + std::string(e.what()) + "', CURDATE())";
        if (mysql_query(db.con, query.c_str()) != 0) {
            std::cerr << "Error logging runtime error: " << mysql_error(db.con) << std::endl;
        }
        std::cerr << "Error: " << e.what() << std::endl;
        return -1; // Indicate error
    }
}

void Bill(Customer &C, const std::string &filename, MySQL& db) {
    int Cost, Days_Between, Advance, misc;
    float Total_Cost, Tax, Final_Total;
    std::ofstream myfile(filename.c_str());

    // Calculate days between entry and exit dates
    Days_Between = daysBetween(C.Date_Entry, C.Date_Exit, db);

    // Room pricing logic
    if (C.Obj.Type == 1) {
        misc = 0;
        Cost = 6778;
    } else if (C.Obj.Type == 2) {
        misc = 50;
        Cost = 12552;
    } else if (C.Obj.Type == 3) {
        misc = 100;
        Cost = 25562;
    } else {
        misc = 200;
        Cost = 35565;
    }

    // Calculate Total Cost
    Total_Cost = Cost * Days_Between;

    // Tax (20%)
    Tax = Total_Cost * 0.2;
    Total_Cost += Tax;

    // Miscellaneous maintenance fee
    if (misc != 0) {
        Total_Cost += misc;
    }

    // Final total cost
    Final_Total = Total_Cost;

    // Output Bill to file
    myfile << "Bill\n";
    myfile << "Customer: " << C.Name << "\n";
    myfile << "Contact: " << C.PhoneNo << "\n\n";
    myfile << "Room No: " << C.Obj.Room_No << "\n";
    myfile << "Date Entry: " << C.Date_Entry << "\n";
    myfile << "Date Exit: " << C.Date_Exit << "\n";
    myfile << "Days of Stay: " << Days_Between << "\n\n";
    myfile << "Cost per day: " << Cost << "\n";
    myfile << "Total for stay: " << Total_Cost << "\n";
    myfile << "Tax (20%): " << Tax << "\n";

    if (misc != 0) {
        myfile << "Misc. Maintenance Fee: " << misc << "\n";
    }

    myfile << "Final Total Cost: " << Final_Total << "\n\n\n";
    myfile << "  Thank you for choosing us!  \n";
    myfile << "";

    // Close the bill file
    myfile.close();

    std::cout << "Bill Printed in File!\n\n\n";

    // Save Billing Information to the Database
    try {
        std::string query = "INSERT INTO billing (customer_id, room_no, entry_date, exit_date, days_stayed, cost_per_day, total_cost, maintenance_fee, tax, final_total) "
                            "VALUES (" + std::to_string(C.Id) + ", " + std::to_string(C.Obj.Room_No) + ", STR_TO_DATE('" + C.Date_Entry + "', '%Y-%m-%d'), "
                            "STR_TO_DATE('" + C.Date_Exit + "', '%Y-%m-%d'), " + std::to_string(Days_Between) + ", " + std::to_string(Cost) + ", " +
                            std::to_string(Total_Cost) + ", " + std::to_string(misc) + ", " + std::to_string(Tax) + ", " + std::to_string(Final_Total) + ");";

        if (mysql_query(db.con, query.c_str()) != 0) {
            std::cerr << "Error saving billing information to database: " << mysql_error(db.con) << std::endl;
        } else {
            std::cout << "Billing information saved to the database.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception during database operation: " << e.what() << std::endl;
    }
}
void PrintRoomDetailsSideBySide(const string &room1, const string &room2, MySQL& db) {
    // Split room1 and room2 into lines and print them side by side
    size_t start1 = 0, start2 = 0;
    while (start1 != string::npos || start2 != string::npos) {
        // Find the end of the current line for each room
        size_t end1 = room1.find('\n', start1);
        size_t end2 = room2.find('\n', start2);

        // Extract the lines or empty strings
        string line1 = (start1 != string::npos) ? room1.substr(start1, end1 - start1) : "";
        string line2 = (start2 != string::npos) ? room2.substr(start2, end2 - start2) : "";

        // Print the two lines side by side with padding
        cout << line1;
        if (!line1.empty()) {
            cout << string(50 - line1.length(), ' '); // Adjust padding as necessary
        }
        cout << line2 << endl;

        // Move to the next line for each room
        start1 = (end1 == string::npos) ? string::npos : end1 + 1;
        start2 = (end2 == string::npos) ? string::npos : end2 + 1;
    }

    // Now, insert the room details into the database after printing
    string query = "INSERT INTO rooms (room_type, bed_type, bathroom_type, additional_features, cost) VALUES ";
    query += "('Standard Room', 'Double Bed', 'Bathroom with shower', 'TV, AC, Complimentary Wi-Fi', 6778), ";
    query += "('Deluxe Room', 'King-sized Bed', 'Upgraded Bathroom (bathtub)', 'Mini-fridge, High-speed internet, Balcony', 12552);";
    db.executeQuery(query);

    cout << "Room details saved to database!" << endl;
}

void PrintAllRooms(MySQL& db) {
    string room1 = 
        "Standard Room:\n"
        "-> Bed (Double)\n"
        "-> Bathroom with shower\n"
        "-> TV\n"
        "-> AC\n"
        "-> Complimentary Wi-Fi\n"
        "Cost: 6778\n";
    
    string room2 = 
        "Deluxe Room:\n"
        "-> King-sized Bed\n"
        "-> Upgraded Bathroom (bathtub)\n"
        "-> Mini-fridge\n"
        "-> High-speed internet\n"
        "-> Balcony\n"
        "Cost: 12552\n";
    
    string room3 = 
        "Suite:\n"
        "-> Separate Bedroom\n"
        "-> Luxurious Bathroom\n"
        "-> Sofa and dining table\n"
        "-> 2 or more TVs\n"
        "-> Workspace\n"
        "Cost: 25562\n";
    
    string room4 = 
        "Presidential Suite:\n"
        "-> Multiple Bedrooms and bathrooms\n"
        "-> Private pool\n"
        "-> In-room entertainment system\n"
        "-> Private Balcony\n"
        "-> Access to exclusive hotel services\n"
        "Cost: 35565\n";

    // Print rooms side by side and save them to the database
    PrintRoomDetailsSideBySide(room1, room2, db);
    cout << endl;
    PrintRoomDetailsSideBySide(room3, room4, db);
}


class MySQL {
    public:
        MYSQL* con;
    
        // Helper function to convert int to string
        string intToString(int value) {
            ostringstream oss;
            oss << value;
            return oss.str();
        }
    
        // Helper function to check the query result for errors
        void checkQueryResult(int queryResult) {
            if (queryResult != 0) {
                cout << "Error executing query: " << mysql_error(con) << endl;
                throw runtime_error("MySQL query failed.");
            }
        }
    
        // Mock function for executing queries returning integers (e.g., room numbers)
        vector<int> executeQuery(const string& query) {
            vector<int> result;
            if (mysql_query(con, query.c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(con);
                MYSQL_ROW row;
    
                while ((row = mysql_fetch_row(res))) {
                    result.push_back(atoi(row[0])); // Assuming first column has room numbers
                }
                mysql_free_result(res);
            } else {
                cout << "Query execution failed: " << mysql_error(con) << endl;
            }
            return result;
        }
    
        // Constructor - Initialize connection to MySQL database
        MySQL() {
            con = mysql_init(nullptr);
            con = mysql_real_connect(con, "localhost", "root", "krishna24#234", "vidwath", 0, nullptr, 0);
            if (!con) {
                cout << "Connection failed: " << mysql_error(con) << endl;
                exit(1);
            }
        }
    
        // Destructor - Close the MySQL connection
        ~MySQL() {
            if (con) {
                mysql_close(con);
            }
        }
    
        void insert(int Id, int Roomno, int Pno, int Food, string Name, int Type, string EntryDate, string ExitDate) {
            int avail = 1; // Assuming availability is being updated to 1
    
            string query = "UPDATE hotelmanagement SET "
                "Id = ?, "
                "Availability = ?, "
                "Name = ?, "
                "Pno = ?, "
                "Food = ?, "
                "EntryDate = ?, "
                "ExitDate = ? "
                "WHERE Roomno = ? AND Type = ?";
    
            MYSQL_STMT* stmt = mysql_stmt_init(con);
            if (!stmt) {
                cout << "Failed to initialize statement: " << mysql_error(con) << endl;
                return;
            }
    
            if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
                cout << "Prepare failed: " << mysql_stmt_error(stmt) << endl;
                mysql_stmt_close(stmt);
                return;
            }
    
            MYSQL_BIND bind[9];
    
            bind[0].buffer_type = MYSQL_TYPE_LONG;
            bind[0].buffer = &Id;
    
            bind[1].buffer_type = MYSQL_TYPE_LONG;
            bind[1].buffer = &avail;
    
            bind[2].buffer_type = MYSQL_TYPE_STRING;
            bind[2].buffer = &Name[0];
            bind[2].buffer_length = Name.length();
    
            bind[3].buffer_type = MYSQL_TYPE_LONG;
            bind[3].buffer = &Pno;
    
            bind[4].buffer_type = MYSQL_TYPE_LONG;
            bind[4].buffer = &Food;
    
            bind[5].buffer_type = MYSQL_TYPE_STRING;
            bind[5].buffer = &EntryDate[0];
            bind[5].buffer_length = EntryDate.length();
    
            bind[6].buffer_type = MYSQL_TYPE_STRING;
            bind[6].buffer = &ExitDate[0];
            bind[6].buffer_length = ExitDate.length();
    
            bind[7].buffer_type = MYSQL_TYPE_LONG;
            bind[7].buffer = &Roomno;
    
            bind[8].buffer_type = MYSQL_TYPE_LONG;
            bind[8].buffer = &Type;
    
            if (mysql_stmt_bind_param(stmt, bind)) {
                cout << "Binding parameters failed: " << mysql_stmt_error(stmt) << endl;
                mysql_stmt_close(stmt);
                return;
            }
    
            if (mysql_stmt_execute(stmt)) {
                cout << "Execute failed: " << mysql_stmt_error(stmt) << endl;
            } else {
                cout << "Record updated successfully!" << endl;
            }
    
            mysql_stmt_close(stmt);
        }
    
        void search() {
            int id;
            cout << "Enter ID to search: ";
            cin >> id;
    
            string query = "SELECT * FROM hotelmanagement WHERE id = ?";
    
            MYSQL_STMT* stmt = mysql_stmt_init(con);
            if (!stmt) {
                cout << "Failed to initialize statement: " << mysql_error(con) << endl;
                return;
            }
    
            if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
                cout << "Prepare failed: " << mysql_stmt_error(stmt) << endl;
                mysql_stmt_close(stmt);
                return;
            }
    
            MYSQL_BIND bind[1];
    
            bind[0].buffer_type = MYSQL_TYPE_LONG;
            bind[0].buffer = &id;
    
            if (mysql_stmt_bind_param(stmt, bind)) {
                cout << "Binding parameters failed: " << mysql_stmt_error(stmt) << endl;
                mysql_stmt_close(stmt);
                return;
            }
    
            if (mysql_stmt_execute(stmt)) {
                cout << "Execute failed: " << mysql_stmt_error(stmt) << endl;
                mysql_stmt_close(stmt);
                return;
            }
    
            MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                cout << "ID: " << row[0]
                    << "\nType: " << row[1]
                    << "\nRoom No: " << row[2]
                    << "\nAvailability: " << row[3]
                    << "\nName: " << row[4]
                    << "\nPhone No: " << row[5]
                    << "\nFood: " << row[6]
                    << "\nEntry Date: " << row[7]
                    << "\nExit Date: " << row[8] << endl;
            }
    
            mysql_free_result(result);
            mysql_stmt_close(stmt);
        }
    
        // Other functions like 'update', 'deleteRecord', etc., can similarly be converted to prepared statements.
    
        void availability(int type, set<int>& rooms) {
            // Query to fetch available rooms
            string query = "SELECT Roomno FROM hotelmanagement WHERE Type = ? AND Availability = 1";  // Availability = 1 means available
            
            // Prepare the statement
            MYSQL_STMT* stmt = mysql_stmt_init(con);
            if (!stmt) {
                cout << "Failed to initialize statement: " << mysql_error(con) << endl;
                return;
            }
        
            // Prepare the query
            if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
                cout << "Prepare failed: " << mysql_stmt_error(stmt) << endl;
                mysql_stmt_close(stmt);
                return;
            }
        
            // Bind the parameter (type)
            MYSQL_BIND bind[1];
            bind[0].buffer_type = MYSQL_TYPE_LONG;
            bind[0].buffer = &type;
        
            if (mysql_stmt_bind_param(stmt, bind)) {
                cout << "Binding parameters failed: " << mysql_stmt_error(stmt) << endl;
                mysql_stmt_close(stmt);
                return;
            }
        
            // Execute the statement
            if (mysql_stmt_execute(stmt)) {
                cout << "Execute failed: " << mysql_stmt_error(stmt) << endl;
                mysql_stmt_close(stmt);
                return;
            }
        
            // Get the result metadata
            MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
            if (!result) {
                cout << "Failed to retrieve result metadata: " << mysql_stmt_error(stmt) << endl;
                mysql_stmt_close(stmt);
                return;
            }
        
            // Bind result columns
            MYSQL_BIND result_bind[1];
            int room_no;
            result_bind[0].buffer_type = MYSQL_TYPE_LONG;
            result_bind[0].buffer = &room_no;
        
            if (mysql_stmt_bind_result(stmt, result_bind)) {
                cout << "Binding result columns failed: " << mysql_stmt_error(stmt) << endl;
                mysql_free_result(result);
                mysql_stmt_close(stmt);
                return;
            }
        
            // Fetch the rows and insert room numbers into the set
            while (mysql_stmt_fetch(stmt) == 0) {
                cout << "Room No: " << room_no << endl;
                rooms.insert(room_no);  // Insert the room number into the set
            }
        
            // Cleanup
            mysql_free_result(result);  // Free the result set
            mysql_stmt_close(stmt);     // Close the prepared statement
        }
        
        void saveCustomerToDatabase(const Customer& customer) {
            string query = "INSERT INTO hotelmanagement (Id, Name, Pno, EntryDate, ExitDate, Roomno) VALUES ("
                + intToString(customer.Id) + ", '"
                + customer.Name + "', "
                + intToString(customer.PhoneNo) + ", '"
                + customer.Date_Entry + "', '"
                + customer.Date_Exit + "', "
                + intToString(customer.Obj.Room_No) + ")";
    
            if (mysql_query(con, query.c_str()) == 0) {
                cout << "Customer saved to database successfully!" << endl;
            } else {
                cout << "Failed to save customer to database: " << mysql_error(con) << endl;
            }
        }
    
        // Function to load a customer from the database
        void loadCustomerFromDatabase(int id, Customer& customer) {
            string query = "SELECT * FROM hotelmanagement WHERE Id = " + intToString(id);
            if (mysql_query(con, query.c_str()) == 0) {
                MYSQL_RES* result = mysql_store_result(con);
                MYSQL_ROW row;
    
                if ((row = mysql_fetch_row(result))) {
                    customer = Customer(
                        row[1],           // Name
                        Room(0, atoi(row[5])), // Room object with Room_No
                        row[3],           // Entry Date
                        row[4],           // Exit Date
                        atoi(row[0]),     // ID
                        atoi(row[2])      // Phone Number
                    );
                    cout << "Customer loaded from database successfully!" << endl;
                } else {
                    cout << "Customer not found!" << endl;
                }
                mysql_free_result(result);
            } else {
                cout << "Failed to load customer from database: " << mysql_error(con) << endl;
            }
        }
    
        // Function to delete a customer from the database
        void deleteCustomerFromDatabase(int id) {
            string query = "DELETE FROM hotelmanagement WHERE Id = " + intToString(id);
            if (mysql_query(con, query.c_str()) == 0) {
                cout << "Customer deleted from database successfully!" << endl;
            } else {
                cout << "Failed to delete customer from database: " << mysql_error(con) << endl;
            }
        }
    
    friend void Save_Customer(const Customer& C, const string& filename);
    friend void Load_Customer(const string& filename, int ID, Customer& C);
    friend void delete_Customer(const string& filename, int searchID);
    friend int stringToInt(const std::string& str);
    friend std::tm parseDate(const std::string& date);
    friend int daysBetween(const std::string& startDate, const std::string& endDate);
    friend void Bill(Customer &C, const string &filename);
    friend void PrintRoomDetailsSideBySide(const string &room1, const string &room2);
    friend void PrintAllRooms();
    };
    
    


   
    
    int main() {
        cout << "<-----------** Welcome **----------->";
        int i;
        string name;
        cout << "\n\nYour Options : \n\t1) New Customer\n\t2) Billing\n\t3) Exit\n";
        cout << "Enter your choice? : ";
        cin >> i;
        MySQL db;
        Room R;
        Customer C1, C2;
    
        while (i != 3 && i < 3 && i > 0) {
            switch (i) {
                case 1: {
                    cout << "\nTypes of rooms present : \n\n";
                    PrintAllRooms(db);
                    
                    int t;
                RoomType:
                do {
                    cout << "Enter Room type: ";
                    cin >> t;
                    if (t < 1 || t > 4) {
                        cout << "\nInvalid Room type\n";
                    }
                } while (t < 1 || t > 4);
                    
                    int roomNo;
                RoomNo:
                    cout << "Enter Room No: ";
                    cin >> roomNo;
                    
                   
                     set<int> rooms; // Declare the variable
                     rooms.clear();
                     db.availability(t, rooms);
                     if (rooms.empty()) {
                         cout << endl << "Rooms not available... Choose another room type" << endl;
                         goto RoomType;
                     }
                     if (rooms.find(roomNo) == rooms.end()) {
                         cout << endl << "The room is not available" << endl;
                         goto RoomNo;
                     }
    
                    // Set the room details for the customer
                    R.SetData(t, roomNo);
                    C1.SetDetails(R);
    
                    // Save customer data to file
                    Save_Customer(C1, "file.dat", db);
                    break;
                }
                case 2: {
                    int id;
                    cout << "Enter Id: ";
                    cin >> id;
    
                    // Load customer details based on ID
                    Load_Customer("file.dat", id, C2, db);
    
                    // Print the bill for the customer
                    Bill(C2, "sample.txt", db);
    
                    break;
                }
                default:
                    cout << "\n\nInvalid choice\n\n";
            }
    
            cout << "\n\nYour Options : \n\t1) New Customer\n\t2) Billing\n\t3) Exit\n";
            cout << "Enter your choice? : ";
            cin >> i;
        }
    
        return 0;
    }
    