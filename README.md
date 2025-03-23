# hotel_management_systems


# **Hotel Management System**

## **Overview**
This project is a **Hotel Management System** implemented in C++ that integrates with a **MySQL database**. It allows users to manage hotel operations such as adding new customers, generating bills, and managing room details. The system uses both file-based storage and a MySQL database for data persistence.

---

## **Features**
1. **Room Management**:
   - Displays details of available room types (Standard, Deluxe, Suite, Presidential Suite).
   - Allows users to select room types and room numbers.

2. **Customer Management**:
   - Add new customers with details such as name, phone number, room type, and stay duration.
   - Save customer data to both a binary file and the MySQL database.

3. **Billing**:
   - Generate bills for customers based on room type, stay duration, and additional charges.
   - Save billing details to the MySQL database.

4. **Database Integration**:
   - Uses MySQL for storing customer, room, and billing information.
   - Logs errors and invalid operations to the database.

5. **File Operations**:
   - Saves and loads customer data from a binary file.
   - Deletes customer records from the file.

---

## **Technologies Used**
- **Programming Language**: C++
- **Database**: MySQL
- **Compiler**: MinGW-w64 (64-bit)
- **Libraries**:
  - `<mysql.h>`: MySQL C API for database operations.
  - Standard C++ libraries for file handling, string manipulation, and date parsing.

---

## **Setup Instructions**

### **1. Prerequisites**
- Install **MySQL Server** (version 9.2 or compatible).
- Install **MinGW-w64** (64-bit) for compiling the code.
- Install the **MySQL Connector/C** or use the development files from your MySQL Server installation.

### **2. Configure MySQL**
1. Create a database named `u23ai067` (or update the database name in the code).
2. Create the following tables in the database:
   ```sql
   CREATE TABLE hotelmanagement (
       Id INT PRIMARY KEY,
       Name VARCHAR(255),
       Pno BIGINT,
       EntryDate DATE,
       ExitDate DATE,
       Roomno INT,
       Type INT,
       Availability INT,
       Food INT
   );

   CREATE TABLE billing (
       customer_id INT,
       room_no INT,
       entry_date DATE,
       exit_date DATE,
       days_stayed INT,
       cost_per_day FLOAT,
       total_cost FLOAT,
       maintenance_fee FLOAT,
       tax FLOAT,
       final_total FLOAT
   );

   CREATE TABLE error_log (
       error_type VARCHAR(255),
       error_message TEXT,
       timestamp DATE
   );
   ```

### **3. Compile the Code**
Use the following command to compile the code:
```bash
g++ MainCode.cpp -o MainCode.exe -I"C:\Program Files\MySQL\MySQL Server 9.2\include" -L"C:\Program Files\MySQL\MySQL Server 9.2\lib" -lmysqlclient -lws2_32 -mconsole
```

### **4. Run the Program**
Run the compiled executable:
```bash
./MainCode.exe
```

---

## **How to Use**
1. **Start the Program**:
   - Run the program and follow the menu options.

2. **Options**:
   - **1) New Customer**:
     - View available room types.
     - Enter customer details and save them to the file and database.
   - **2) Billing**:
     - Enter the customer ID to load their details.
     - Generate and save the bill to a file and the database.
   - **3) Exit**:
     - Exit the program.

3. **File Storage**:
   - Customer data is saved in `file.dat` (binary file).
   - Bills are saved in `sample.txt`.

---

## **Code Structure**
- **Classes**:
  - `Room`: Represents room details.
  - `Customer`: Represents customer details.
  - `MySQL`: Handles database operations.

- **Functions**:
  - `Save_Customer`: Saves customer data to a file and the database.
  - `Load_Customer`: Loads customer data from a file or the database.
  - `delete_Customer`: Deletes customer data from a file and the database.
  - `Bill`: Generates a bill for a customer and saves it to a file and the database.
  - `PrintAllRooms`: Displays room details and saves them to the database.

- **Main Function**:
  - Provides a menu-driven interface for managing customers and billing.

---

## **Error Handling**
- Logs invalid operations (e.g., invalid date format, failed database queries) to the `error_log` table in the database.

---

## **Future Enhancements**
- Add a graphical user interface (GUI) for better user experience.
- Implement room availability checks.
- Add more detailed error handling and logging.

---

## **Author**
- **Name**: kolluru vidwath kumar
- **Contact**: kvidwathkumar@gmail.com

---




