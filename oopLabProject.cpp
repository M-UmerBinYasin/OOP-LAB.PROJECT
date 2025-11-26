#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <string>
#include <cmath>

using namespace std;

// Console color codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

// Custom exception class
class BankException {
private:
    string message;
public:
    BankException(const string& msg) : message(msg) {}
    const char* what() const { return message.c_str(); }
};

// Forward declarations
class Transaction;
class Account;
class Customer;
class Admin;
class Bank;

// Transaction class
class Transaction {
private:
    static int nextID;
    int transactionID;
    string type;
    double amount;
    string dateTime;
    int senderAccount;
    int receiverAccount;

public:
    Transaction() : transactionID(0), amount(0), senderAccount(0), receiverAccount(0) {}

    Transaction(const string& t, double amt, int sender = 0, int receiver = 0) {
        transactionID = nextID++;
        type = t;
        amount = amt;
        senderAccount = sender;
        receiverAccount = receiver;
        setCurrentDateTime();
    }

    void setCurrentDateTime() {
        time_t now = time(0);
        struct tm timeinfo;
        localtime_s(&timeinfo, &now);
        char buffer[20];
        strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", &timeinfo);
        dateTime = buffer;
    }

    void displayTransaction() const {
        cout << CYAN << BOLD << "| " << setw(4) << transactionID << " | "
            << setw(12) << left << type << " | "
            << setw(10) << right << fixed << setprecision(2) << amount << " | "
            << setw(19) << dateTime << " |" << RESET;

        if (type == "transfer") {
            cout << CYAN << " " << senderAccount << " -> " << receiverAccount << RESET;
        }
        cout << endl;
    }

    void saveToFile(ofstream& file) const {
        file << transactionID << " " << type << " " << amount << " "
            << dateTime << " " << senderAccount << " " << receiverAccount << endl;
    }

    void loadFromFile(ifstream& file) {
        file >> transactionID >> type >> amount >> dateTime >> senderAccount >> receiverAccount;
    }

    int getID() const { return transactionID; }
    string getType() const { return type; }
    double getAmount() const { return amount; }
    int getSender() const { return senderAccount; }
    int getReceiver() const { return receiverAccount; }
};

int Transaction::nextID = 1;

// Person base class
class Person {
protected:
    string name;
    int id;
    string phone;
    string email;

public:
    Person() : id(0) {}

    virtual void displayInfo() const {
        cout << GREEN << BOLD << "| " << setw(4) << id << " | "
            << setw(20) << left << name << " | "
            << setw(15) << phone << " | "
            << setw(25) << email << " |" << RESET << endl;
    }

    virtual void setInfo(const string& n, int i, const string& p, const string& e) {
        if (i <= 0) throw BankException("Invalid ID");
        if (n.empty()) throw BankException("Name cannot be empty");

        name = n;
        id = i;
        phone = p;
        email = e;
    }

    int getID() const { return id; }
    string getName() const { return name; }
};

// Account class
class Account {
private:
    static int nextAccountNumber;
    int accountNumber;
    double balance;
    int customerID;
    string accountType;
    double dailyLimit;
    double dailyWithdrawn;
    Transaction** transactionHistory;
    int transactionCount;
    int transactionCapacity;

    void resizeTransactionHistory() {
        int newCapacity = transactionCapacity * 2;
        Transaction** newHistory = new Transaction * [newCapacity];

        for (int i = 0; i < transactionCount; i++) {
            newHistory[i] = transactionHistory[i];
        }

        delete[] transactionHistory;
        transactionHistory = newHistory;
        transactionCapacity = newCapacity;
    }

public:
    Account() : accountNumber(0), balance(0), customerID(0), dailyLimit(1000),
        dailyWithdrawn(0), transactionCount(0), transactionCapacity(10) {
        accountType = "Savings";
        transactionHistory = new Transaction * [transactionCapacity];
    }

    Account(int custID, const string& type, double initialDeposit = 0) {
        accountNumber = nextAccountNumber++;
        balance = initialDeposit;
        customerID = custID;
        dailyLimit = 1000;
        dailyWithdrawn = 0;
        accountType = type;
        transactionCount = 0;
        transactionCapacity = 10;
        transactionHistory = new Transaction * [transactionCapacity];

        if (initialDeposit > 0) {
            addTransaction(new Transaction("deposit", initialDeposit));
        }
    }

    ~Account() {
        for (int i = 0; i < transactionCount; i++) {
            delete transactionHistory[i];
        }
        delete[] transactionHistory;
    }

    void deposit(double amount) {
        if (amount <= 0) throw BankException("Deposit amount must be positive");
        if (amount > 100000) throw BankException("Deposit amount too large");

        balance += amount;
        addTransaction(new Transaction("deposit", amount));
        cout << GREEN << "SUCCESS: Deposit successful! New balance: $"
            << fixed << setprecision(2) << balance << RESET << endl;
    }

    void withdraw(double amount) {
        if (amount <= 0) throw BankException("Withdrawal amount must be positive");
        if (amount > balance) throw BankException("Insufficient funds");
        if (!limitCheck(amount)) throw BankException("Exceeds daily withdrawal limit");

        balance -= amount;
        dailyWithdrawn += amount;
        addTransaction(new Transaction("withdraw", amount));
        cout << GREEN << "SUCCESS: Withdrawal successful! New balance: $"
            << fixed << setprecision(2) << balance << RESET << endl;
    }

    bool limitCheck(double amount) const {
        return (amount <= dailyLimit) && (dailyWithdrawn + amount <= dailyLimit);
    }

    double checkBalance() const {
        return balance;
    }

    void displayAccountDetails() const {
        cout << BLUE << BOLD << "| " << setw(10) << accountNumber << " | "
            << setw(12) << left << accountType << " | "
            << setw(12) << right << fixed << setprecision(2) << balance << " | "
            << setw(8) << customerID << " | "
            << setw(10) << (dailyLimit - dailyWithdrawn) << " |" << RESET << endl;
    }

    void showHistory() const {
        cout << YELLOW << BOLD << "+------+------------+------------+---------------------+-----------------+" << endl;
        cout << "| " << setw(4) << "ID" << " | "
            << setw(10) << "Type" << " | "
            << setw(10) << "Amount" << " | "
            << setw(19) << "Date/Time" << " | "
            << setw(15) << "Details" << " |" << endl;
        cout << "+------+------------+------------+---------------------+-----------------+" << RESET << endl;

        for (int i = 0; i < transactionCount; i++) {
            transactionHistory[i]->displayTransaction();
        }
        cout << YELLOW << "+------+------------+------------+---------------------+-----------------+" << RESET << endl;
    }

    void addTransaction(Transaction* transaction) {
        if (transactionCount >= transactionCapacity) {
            resizeTransactionHistory();
        }
        transactionHistory[transactionCount++] = transaction;
    }

    void saveToFile(ofstream& file) const {
        file << accountNumber << " " << balance << " " << customerID << " "
            << accountType << " " << dailyLimit << " " << dailyWithdrawn << " "
            << transactionCount << endl;

        for (int i = 0; i < transactionCount; i++) {
            transactionHistory[i]->saveToFile(file);
        }
    }

    void loadFromFile(ifstream& file) {
        file >> accountNumber >> balance >> customerID >> accountType
            >> dailyLimit >> dailyWithdrawn >> transactionCount;

        transactionCapacity = transactionCount + 10;
        transactionHistory = new Transaction * [transactionCapacity];

        for (int i = 0; i < transactionCount; i++) {
            transactionHistory[i] = new Transaction();
            transactionHistory[i]->loadFromFile(file);
        }
    }

    void resetDailyLimit() {
        dailyWithdrawn = 0;
    }

    int getAccountNumber() const { return accountNumber; }
    int getCustomerID() const { return customerID; }
    double getBalance() const { return balance; }
    double getRemainingDailyLimit() const { return dailyLimit - dailyWithdrawn; }
};

int Account::nextAccountNumber = 1001;

// Customer class
class Customer : public Person {
private:
    int accountNumber;
    string address;

public:
    Customer() : accountNumber(0) {}

    void setInfo(const string& n, int i, const string& p, const string& e,
        const string& addr, int accNum) {
        Person::setInfo(n, i, p, e);
        address = addr;
        accountNumber = accNum;
    }

    void displayInfo() const override {
        cout << GREEN << BOLD << "+------+----------------------+-----------------+-------------------------+------------------+" << endl;
        cout << "| " << setw(4) << "ID" << " | "
            << setw(20) << "Name" << " | "
            << setw(15) << "Phone" << " | "
            << setw(23) << "Email" << " | "
            << setw(18) << "Account" << " |" << endl;
        cout << "+------+----------------------+-----------------+-------------------------+------------------+" << RESET << endl;
        Person::displayInfo();
        cout << GREEN << BOLD << "+------+----------------------+-----------------+-------------------------+------------------+" << RESET << endl;
        cout << "Address: " << address << endl;
    }

    void showAccount() const {
        cout << BLUE << "Account Number: " << accountNumber << RESET << endl;
    }

    void updateInfo(const string& newPhone, const string& newEmail, const string& newAddress) {
        phone = newPhone;
        email = newEmail;
        address = newAddress;
        cout << GREEN << "SUCCESS: Customer information updated successfully!" << RESET << endl;
    }

    int getAccountNumber() const { return accountNumber; }

    void saveToFile(ofstream& file) const {
        file << id << " " << name << " " << phone << " " << email << " "
            << address << " " << accountNumber << endl;
    }

    void loadFromFile(ifstream& file) {
        file >> id >> name >> phone >> email >> address >> accountNumber;
    }
};

// Admin class
class Admin : public Person {
private:
    string username;
    string password;

public:
    Admin() : username("admin"), password("admin123") {
        name = "System Administrator";
        id = 1;
    }

    bool login(const string& user, const string& pass) {
        return (username == user && password == pass);
    }

    void viewAllAccounts(Account** accounts, int accountCount) const {
        cout << MAGENTA << BOLD << "+------------+------------+------------+----------+------------+" << endl;
        cout << "| " << setw(10) << "Account" << " | "
            << setw(10) << "Type" << " | "
            << setw(10) << "Balance" << " | "
            << setw(8) << "Cust ID" << " | "
            << setw(10) << "Remaining" << " |" << endl;
        cout << "| " << setw(10) << "Number" << " | "
            << setw(10) << "" << " | "
            << setw(10) << "" << " | "
            << setw(8) << "" << " | "
            << setw(10) << "Limit" << " |" << endl;
        cout << "+------------+------------+------------+----------+------------+" << RESET << endl;

        for (int i = 0; i < accountCount; i++) {
            accounts[i]->displayAccountDetails();
        }
        cout << MAGENTA << "+------------+------------+------------+----------+------------+" << RESET << endl;
    }

    void viewUserTransactions(Account** accounts, int accountCount, int customerID) const {
        bool found = false;
        for (int i = 0; i < accountCount; i++) {
            if (accounts[i]->getCustomerID() == customerID) {
                if (!found) {
                    cout << MAGENTA << "=== TRANSACTIONS FOR CUSTOMER ID: " << customerID << " ===" << RESET << endl;
                    found = true;
                }
                accounts[i]->showHistory();
            }
        }
        if (!found) {
            throw BankException("No accounts found for customer ID: " + to_string(customerID));
        }
    }

    void blockAccount(Account** accounts, int accountCount, int accNumber) const {
        cout << RED << "ALERT: Account #" << accNumber << " has been blocked!" << RESET << endl;
    }

    void deleteAccount(Account**& accounts, int& accountCount, int accNumber) {
        int foundIndex = -1;
        for (int i = 0; i < accountCount; i++) {
            if (accounts[i]->getAccountNumber() == accNumber) {
                foundIndex = i;
                break;
            }
        }

        if (foundIndex != -1) {
            delete accounts[foundIndex];
            for (int i = foundIndex; i < accountCount - 1; i++) {
                accounts[i] = accounts[i + 1];
            }
            accountCount--;
            cout << RED << "ALERT: Account #" << accNumber << " has been deleted!" << RESET << endl;
        }
        else {
            throw BankException("Account not found: " + to_string(accNumber));
        }
    }
};

// Bank class - central controller
class Bank {
private:
    Customer** customers;
    Account** accounts;
    Admin admin;
    int customerCount;
    int accountCount;
    int customerCapacity;
    int accountCapacity;

    void resizeCustomers() {
        int newCapacity = customerCapacity * 2;
        Customer** newCustomers = new Customer * [newCapacity];

        for (int i = 0; i < customerCount; i++) {
            newCustomers[i] = customers[i];
        }

        delete[] customers;
        customers = newCustomers;
        customerCapacity = newCapacity;
    }

    void resizeAccounts() {
        int newCapacity = accountCapacity * 2;
        Account** newAccounts = new Account * [newCapacity];

        for (int i = 0; i < accountCount; i++) {
            newAccounts[i] = accounts[i];
        }

        delete[] accounts;
        accounts = newAccounts;
        accountCapacity = newCapacity;
    }

    void resetDailyLimits() {
        for (int i = 0; i < accountCount; i++) {
            accounts[i]->resetDailyLimit();
        }
    }

public:
    Bank() : customerCount(0), accountCount(0), customerCapacity(10), accountCapacity(10) {
        customers = new Customer * [customerCapacity];
        accounts = new Account * [accountCapacity];
    }

    ~Bank() {
        for (int i = 0; i < customerCount; i++) {
            delete customers[i];
        }
        for (int i = 0; i < accountCount; i++) {
            delete accounts[i];
        }
        delete[] customers;
        delete[] accounts;
    }

    void createAccount() {
        try {
            string name, phone, email, address, type;
            int id;
            double initialDeposit;

            cout << "Enter customer name: ";
            cin.ignore();
            getline(cin, name);

            cout << "Enter customer ID: ";
            cin >> id;

            // Check if customer ID already exists
            for (int i = 0; i < customerCount; i++) {
                if (customers[i]->getID() == id) {
                    throw BankException("Customer ID already exists");
                }
            }

            cout << "Enter phone: ";
            cin >> phone;

            cout << "Enter email: ";
            cin >> email;

            cout << "Enter address: ";
            cin.ignore();
            getline(cin, address);

            cout << "Enter account type (Savings/Current): ";
            cin >> type;

            cout << "Enter initial deposit: $";
            cin >> initialDeposit;

            if (initialDeposit < 0) {
                throw BankException("Initial deposit cannot be negative");
            }

            if (initialDeposit > 1000000) {
                throw BankException("Initial deposit too large");
            }

            // Create account first to get account number
            Account* newAccount = new Account(id, type, initialDeposit);

            // Create customer
            if (customerCount >= customerCapacity) {
                resizeCustomers();
            }
            customers[customerCount] = new Customer();
            customers[customerCount]->setInfo(name, id, phone, email, address, newAccount->getAccountNumber());
            customerCount++;

            // Add account to bank
            if (accountCount >= accountCapacity) {
                resizeAccounts();
            }
            accounts[accountCount] = newAccount;
            accountCount++;

            cout << GREEN << "SUCCESS: Account created successfully!" << endl;
            cout << "Account Number: " << newAccount->getAccountNumber() << RESET << endl;

        }
        catch (const BankException& e) {
            cout << RED << "ERROR: " << e.what() << RESET << endl;
        }
    }

    void deleteAccount(int accNumber) {
        try {
            admin.deleteAccount(accounts, accountCount, accNumber);

            // Also remove customer associated with this account
            for (int i = 0; i < customerCount; i++) {
                if (customers[i]->getAccountNumber() == accNumber) {
                    delete customers[i];
                    for (int j = i; j < customerCount - 1; j++) {
                        customers[j] = customers[j + 1];
                    }
                    customerCount--;
                    break;
                }
            }
        }
        catch (const BankException& e) {
            cout << RED << "ERROR: " << e.what() << RESET << endl;
        }
    }

    Account* searchAccount(int accNumber) {
        for (int i = 0; i < accountCount; i++) {
            if (accounts[i]->getAccountNumber() == accNumber) {
                return accounts[i];
            }
        }
        return nullptr;
    }

    Customer* searchCustomer(int custID) {
        for (int i = 0; i < customerCount; i++) {
            if (customers[i]->getID() == custID) {
                return customers[i];
            }
        }
        return nullptr;
    }

    void transferFunds(int senderAcc, int receiverAcc, double amount) {
        try {
            if (amount <= 0) throw BankException("Transfer amount must be positive");
            if (amount > 50000) throw BankException("Transfer amount exceeds maximum limit");

            Account* sender = searchAccount(senderAcc);
            Account* receiver = searchAccount(receiverAcc);

            if (!sender) throw BankException("Sender account not found");
            if (!receiver) throw BankException("Receiver account not found");
            if (sender->getBalance() < amount) throw BankException("Insufficient funds");
            if (sender == receiver) throw BankException("Cannot transfer to same account");

            // Perform transfer
            sender->withdraw(amount);
            receiver->deposit(amount);

            // Record transfer transaction for both accounts
            Transaction* transferTx = new Transaction("transfer", amount, senderAcc, receiverAcc);
            sender->addTransaction(transferTx);
            receiver->addTransaction(new Transaction("transfer", amount, senderAcc, receiverAcc));

            cout << GREEN << "SUCCESS: Transfer successful! $" << fixed << setprecision(2) << amount
                << " transferred from " << senderAcc << " to " << receiverAcc << RESET << endl;

        }
        catch (const BankException& e) {
            cout << RED << "ERROR: " << e.what() << RESET << endl;
        }
    }

    void displayAllCustomers() const {
        if (customerCount == 0) {
            cout << YELLOW << "No customers found." << RESET << endl;
            return;
        }

        cout << MAGENTA << BOLD << "=== ALL CUSTOMERS (" << customerCount << ") ===" << RESET << endl;
        for (int i = 0; i < customerCount; i++) {
            customers[i]->displayInfo();
            cout << endl;
        }
    }

    void displayAllAccounts() const {
        if (accountCount == 0) {
            cout << YELLOW << "No accounts found." << RESET << endl;
            return;
        }
        admin.viewAllAccounts(accounts, accountCount);
    }

    void customerMenu(int customerID) {
        Customer* customer = searchCustomer(customerID);
        Account* account = nullptr;

        if (customer) {
            account = searchAccount(customer->getAccountNumber());
        }

        if (!customer || !account) {
            cout << RED << "ERROR: Customer or account not found!" << RESET << endl;
            return;
        }

        cout << GREEN << "Welcome, " << customer->getName() << "!" << RESET << endl;

        int choice;
        do {
            cout << CYAN << BOLD << "\n=== CUSTOMER DASHBOARD ===" << RESET << endl;
            cout << "1. Display My Info" << endl;
            cout << "2. Check Balance" << endl;
            cout << "3. Deposit Money" << endl;
            cout << "4. Withdraw Money" << endl;
            cout << "5. View Transaction History" << endl;
            cout << "6. Update Personal Info" << endl;
            cout << "7. View Account Details" << endl;
            cout << "8. Logout" << endl;
            cout << "Enter your choice: ";
            cin >> choice;

            try {
                switch (choice) {
                case 1:
                    customer->displayInfo();
                    break;
                case 2:
                    cout << GREEN << "Current Balance: $" << fixed << setprecision(2)
                        << account->checkBalance() << RESET << endl;
                    break;
                case 3: {
                    double amount;
                    cout << "Enter deposit amount: $";
                    cin >> amount;
                    account->deposit(amount);
                    break;
                }
                case 4: {
                    double amount;
                    cout << "Enter withdrawal amount: $";
                    cin >> amount;
                    account->withdraw(amount);
                    break;
                }
                case 5:
                    account->showHistory();
                    break;
                case 6: {
                    string phone, email, address;
                    cout << "Enter new phone: ";
                    cin >> phone;
                    cout << "Enter new email: ";
                    cin >> email;
                    cout << "Enter new address: ";
                    cin.ignore();
                    getline(cin, address);
                    customer->updateInfo(phone, email, address);
                    break;
                }
                case 7:
                    cout << BLUE << BOLD << "=== ACCOUNT DETAILS ===" << RESET << endl;
                    account->displayAccountDetails();
                    cout << "Remaining Daily Limit: $" << account->getRemainingDailyLimit() << endl;
                    break;
                case 8:
                    cout << "Logging out..." << endl;
                    break;
                default:
                    cout << RED << "ERROR: Invalid choice!" << RESET << endl;
                }
            }
            catch (const BankException& e) {
                cout << RED << "ERROR: " << e.what() << RESET << endl;
            }
        } while (choice != 8);
    }

    void adminMenu() {
        string username, password;
        cout << "Enter admin username: ";
        cin >> username;
        cout << "Enter admin password: ";
        cin >> password;

        if (!admin.login(username, password)) {
            cout << RED << "ERROR: Invalid admin credentials!" << RESET << endl;
            return;
        }

        cout << GREEN << "Welcome, System Administrator!" << RESET << endl;

        int choice;
        do {
            cout << MAGENTA << BOLD << "\n=== ADMIN DASHBOARD ===" << RESET << endl;
            cout << "1. View All Accounts" << endl;
            cout << "2. View Customer Transactions" << endl;
            cout << "3. Block Account" << endl;
            cout << "4. Delete Account" << endl;
            cout << "5. View All Customers" << endl;
            cout << "6. Reset Daily Limits" << endl;
            cout << "7. System Statistics" << endl;
            cout << "8. Logout" << endl;
            cout << "Enter your choice: ";
            cin >> choice;

            try {
                switch (choice) {
                case 1:
                    displayAllAccounts();
                    break;
                case 2: {
                    int custID;
                    cout << "Enter customer ID: ";
                    cin >> custID;
                    admin.viewUserTransactions(accounts, accountCount, custID);
                    break;
                }
                case 3: {
                    int accNum;
                    cout << "Enter account number to block: ";
                    cin >> accNum;
                    admin.blockAccount(accounts, accountCount, accNum);
                    break;
                }
                case 4: {
                    int accNum;
                    cout << "Enter account number to delete: ";
                    cin >> accNum;
                    deleteAccount(accNum);
                    break;
                }
                case 5:
                    displayAllCustomers();
                    break;
                case 6:
                    resetDailyLimits();
                    cout << GREEN << "SUCCESS: Daily limits reset for all accounts!" << RESET << endl;
                    break;
                case 7:
                    showSystemStatistics();
                    break;
                case 8:
                    cout << "Logging out..." << endl;
                    break;
                default:
                    cout << RED << "ERROR: Invalid choice!" << RESET << endl;
                }
            }
            catch (const BankException& e) {
                cout << RED << "ERROR: " << e.what() << RESET << endl;
            }
        } while (choice != 8);
    }

    void showSystemStatistics() const {
        cout << YELLOW << BOLD << "\n=== SYSTEM STATISTICS ===" << RESET << endl;
        cout << "Total Customers: " << customerCount << endl;
        cout << "Total Accounts: " << accountCount << endl;

        double totalBalance = 0;
        for (int i = 0; i < accountCount; i++) {
            totalBalance += accounts[i]->getBalance();
        }
        cout << "Total Bank Balance: $" << fixed << setprecision(2) << totalBalance << endl;

        if (accountCount > 0) {
            double avgBalance = totalBalance / accountCount;
            cout << "Average Account Balance: $" << fixed << setprecision(2) << avgBalance << endl;
        }
    }

    void saveData() {
        ofstream customerFile("customers.txt");
        ofstream accountFile("accounts.txt");

        if (customerFile.is_open()) {
            customerFile << customerCount << endl;
            for (int i = 0; i < customerCount; i++) {
                customers[i]->saveToFile(customerFile);
            }
            customerFile.close();
        }

        if (accountFile.is_open()) {
            accountFile << accountCount << endl;
            for (int i = 0; i < accountCount; i++) {
                accounts[i]->saveToFile(accountFile);
            }
            accountFile.close();
        }

        cout << GREEN << "SUCCESS: Data saved successfully!" << RESET << endl;
    }

    void loadData() {
        ifstream customerFile("customers.txt");
        ifstream accountFile("accounts.txt");

        if (customerFile.is_open()) {
            customerFile >> customerCount;
            customerCapacity = customerCount + 10;
            customers = new Customer * [customerCapacity];

            for (int i = 0; i < customerCount; i++) {
                customers[i] = new Customer();
                customers[i]->loadFromFile(customerFile);
            }
            customerFile.close();
        }

        if (accountFile.is_open()) {
            accountFile >> accountCount;
            accountCapacity = accountCount + 10;
            accounts = new Account * [accountCapacity];

            for (int i = 0; i < accountCount; i++) {
                accounts[i] = new Account();
                accounts[i]->loadFromFile(accountFile);
            }
            accountFile.close();
        }
    }
};

// Utility functions
void displayWelcome() {
    cout << YELLOW << BOLD << "================================================================" << endl;
    cout << "               BANK MANAGEMENT SYSTEM" << endl;
    cout << "                 Professional Edition" << endl;
    cout << "================================================================" << RESET << endl;
}

void displayGoodbye() {
    cout << YELLOW << BOLD << "\n================================================================" << endl;
    cout << "        Thank you for using our Banking System!" << endl;
    cout << "================================================================" << RESET << endl;
}

// Main function
int main() {
    Bank bank;
    int mainChoice;

    displayWelcome();

    // Try to load existing data
    try {
        bank.loadData();
        cout << GREEN << "SUCCESS: Previous data loaded successfully!" << RESET << endl;
    }
    catch (...) {
        cout << YELLOW << "INFO: No previous data found. Starting fresh system." << RESET << endl;
    }

    do {
        cout << CYAN << BOLD << "\n=== MAIN MENU ===" << RESET << endl;
        cout << "1. Create New Account" << endl;
        cout << "2. Customer Login" << endl;
        cout << "3. Admin Login" << endl;
        cout << "4. Transfer Funds" << endl;
        cout << "5. Save Data" << endl;
        cout << "6. Exit System" << endl;
        cout << "Enter your choice: ";
        cin >> mainChoice;

        try {
            switch (mainChoice) {
            case 1:
                bank.createAccount();
                break;
            case 2: {
                int customerID;
                cout << "Enter your customer ID: ";
                cin >> customerID;
                bank.customerMenu(customerID);
                break;
            }
            case 3:
                bank.adminMenu();
                break;
            case 4: {
                int sender, receiver;
                double amount;
                cout << "Enter sender account number: ";
                cin >> sender;
                cout << "Enter receiver account number: ";
                cin >> receiver;
                cout << "Enter amount: $";
                cin >> amount;
                bank.transferFunds(sender, receiver, amount);
                break;
            }
            case 5:
                bank.saveData();
                break;
            case 6:
                bank.saveData();
                displayGoodbye();
                break;
            default:
                cout << RED << "ERROR: Invalid choice! Please try again." << RESET << endl;
            }
        }
        catch (const BankException& e) {
            cout << RED << "ERROR: " << e.what() << RESET << endl;
        }
        catch (...) {
            cout << RED << "ERROR: Unexpected error occurred!" << RESET << endl;
        }
    } while (mainChoice != 6);

    return 0;
}