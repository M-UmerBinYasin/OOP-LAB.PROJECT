#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <windows.h>
using namespace std;

const string ACCOUNTS_FILE = "bank_accounts.dat";
const string TRANSACTIONS_FILE = "transactions.dat";

void clearScreen() {
    system("cls");
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void resetColor() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

void showHeader() {
    setColor(11);
    cout << "============================================\n";
    cout << "     BANK MANAGEMENT SYSTEM\n";
    cout << "============================================\n";
    resetColor();
}

string intToString(int value) {
    stringstream ss;
    ss << value;
    return ss.str();
}

string doubleToString(double value) {
    stringstream ss;
    ss << value;
    return ss.str();
}

void safeGetLine(string& input) {
    getline(cin, input);
}

class BankException {
protected:
    string message;
public:
    BankException(const string& msg) : message(msg) {}
    virtual void display() const {
        setColor(12);
        cout << "Bank Error: " << message << endl;
        resetColor();
    }
    virtual ~BankException() {}
};

class AccountException : public BankException {
public:
    AccountException(const string& msg) : BankException(msg) {}
    void display() const override {
        setColor(12);
        cout << "Account Error: " << message << endl;
        resetColor();
    }
};

class TransactionException : public BankException {
public:
    TransactionException(const string& msg) : BankException(msg) {}
    void display() const override {
        setColor(12);
        cout << "Transaction Error: " << message << endl;
        resetColor();
    }
};

class AdminException : public BankException {
public:
    AdminException(const string& msg) : BankException(msg) {}
    void display() const override {
        setColor(12);
        cout << "Admin Error: " << message << endl;
        resetColor();
    }
};

class Date {
private:
    int day, month, year;
public:
    Date() : day(1), month(1), year(2024) {}
    Date(int d, int m, int y) : day(d), month(m), year(y) {}

    string toString() const {
        return intToString(day) + "/" + intToString(month) + "/" + intToString(year);
    }

    static Date getCurrentDate() {
        time_t now = time(0);
        tm* ltm;
        tm timeStruct;
        ltm = &timeStruct;
        localtime_s(ltm, &now);
        return Date(ltm->tm_mday, 1 + ltm->tm_mon, 1900 + ltm->tm_year);
    }

    bool operator==(const Date& other) const {
        return (day == other.day && month == other.month && year == other.year);
    }
};

class Transaction {
private:
    static int nextId;
    int transactionId;
    string type;
    double amount;
    Date date;
    string description;
    int fromAccount;
    int toAccount;
public:
    Transaction() : transactionId(0), amount(0), fromAccount(0), toAccount(0) {}

    Transaction(const string& t, double amt, const string& desc = "",
        int from = 0, int to = 0)
        : transactionId(++nextId), type(t), amount(amt), description(desc),
        fromAccount(from), toAccount(to) {
        date = Date::getCurrentDate();
    }

    void display() const {
        setColor(14);
        cout << "ID: " << transactionId;
        resetColor();
        cout << " | Type: " << type
            << " | Amount: $";
        setColor(10);
        cout << amount;
        resetColor();
        cout << " | Date: " << date.toString();
        if (!description.empty()) {
            cout << " | Desc: " << description;
        }
        if (fromAccount > 0 && toAccount > 0) {
            cout << " | From: " << fromAccount << " To: " << toAccount;
        }
        cout << endl;
    }

    void saveToFile(ofstream& file) const {
        file << transactionId << " " << type << " " << amount << " "
            << description << " " << fromAccount << " " << toAccount << endl;
    }

    void loadFromFile(ifstream& file) {
        file >> transactionId >> type >> amount >> description >> fromAccount >> toAccount;
    }

    int getId() const { return transactionId; }
    string getType() const { return type; }
    double getAmount() const { return amount; }
    int getFromAccount() const { return fromAccount; }
    int getToAccount() const { return toAccount; }
};

int Transaction::nextId = 1000;

class TransactionHistory {
private:
    Transaction* transactions;
    int count;
    int capacity;

    void resize() {
        capacity *= 2;
        Transaction* newTransactions = new Transaction[capacity];
        for (int i = 0; i < count; i++) {
            newTransactions[i] = transactions[i];
        }
        delete[] transactions;
        transactions = newTransactions;
    }

public:
    TransactionHistory() : count(0), capacity(10) {
        transactions = new Transaction[capacity];
    }

    ~TransactionHistory() {
        delete[] transactions;
    }

    void addTransaction(const Transaction& t) {
        if (count == capacity) {
            resize();
        }
        transactions[count++] = t;

        ofstream file(TRANSACTIONS_FILE, ios::app);
        if (file.is_open()) {
            t.saveToFile(file);
            file.close();
        }
    }

    void displayAll() const {
        if (count == 0) {
            cout << "No transactions found.\n";
            return;
        }
        setColor(11);
        cout << "\n========== TRANSACTION HISTORY ==========\n";
        resetColor();
        for (int i = 0; i < count; i++) {
            transactions[i].display();
        }
        setColor(11);
        cout << "========================================\n";
        resetColor();
    }

    void loadFromFile() {
        ifstream file(TRANSACTIONS_FILE);
        if (file.is_open()) {
            while (!file.eof()) {
                Transaction t;
                t.loadFromFile(file);
                if (t.getId() > 0) {
                    addTransaction(t);
                }
            }
            file.close();
        }
    }

    int getCount() const { return count; }
};

class BankAccount {
protected:
    static int nextAccountId;
    int accountId;
    string accountHolder;
    double balance;
    string accountType;
    bool isActive;
    double dailyLimit;
    double dailyWithdrawn;
    Date lastResetDate;
    TransactionHistory history;

public:
    void addToHistory(const Transaction& t) {
        history.addTransaction(t);
    }

    BankAccount() : accountId(0), balance(0), isActive(true),
        dailyLimit(5000), dailyWithdrawn(0) {
    }

    BankAccount(const string& name, const string& type = "Savings", double initial = 0)
        : accountId(++nextAccountId), accountHolder(name), balance(initial),
        accountType(type), isActive(true), dailyLimit(5000),
        dailyWithdrawn(0) {
        lastResetDate = Date::getCurrentDate();
        if (initial > 0) {
            history.addTransaction(Transaction("Deposit", initial, "Initial deposit"));
        }
        saveToFile();
    }

    virtual ~BankAccount() {}

    int getAccountId() const { return accountId; }
    string getAccountHolder() const { return accountHolder; }
    double getBalance() const { return balance; }
    bool getIsActive() const { return isActive; }

    virtual void deposit(double amount) {
        if (!isActive) {
            throw AccountException("Account is blocked!");
        }
        if (amount <= 0) {
            throw TransactionException("Deposit amount must be positive!");
        }

        balance += amount;
        resetDailyLimit();
        history.addTransaction(Transaction("Deposit", amount));
        setColor(10);
        cout << "Successfully deposited $" << amount << endl;
        resetColor();
        saveToFile();
    }

    virtual void withdraw(double amount) {
        if (!isActive) {
            throw AccountException("Account is blocked!");
        }
        if (amount <= 0) {
            throw TransactionException("Withdrawal amount must be positive!");
        }
        if (amount > balance) {
            throw TransactionException("Insufficient balance!");
        }

        resetDailyLimit();
        if (dailyWithdrawn + amount > dailyLimit) {
            throw TransactionException("Daily withdrawal limit exceeded!");
        }

        balance -= amount;
        dailyWithdrawn += amount;
        history.addTransaction(Transaction("Withdrawal", amount));
        setColor(10);
        cout << "Successfully withdrew $" << amount << endl;
        resetColor();
        saveToFile();
    }

    virtual void transfer(BankAccount* receiver, double amount) {
        if (!isActive) {
            throw AccountException("Your account is blocked!");
        }
        if (!receiver->getIsActive()) {
            throw AccountException("Receiver account is blocked!");
        }
        if (amount <= 0) {
            throw TransactionException("Transfer amount must be positive!");
        }
        if (amount > balance) {
            throw TransactionException("Insufficient balance for transfer!");
        }

        resetDailyLimit();
        if (dailyWithdrawn + amount > dailyLimit) {
            throw TransactionException("Daily transfer limit exceeded!");
        }

        balance -= amount;
        receiver->receiveTransfer(amount);
        dailyWithdrawn += amount;

        Transaction senderTrans("Transfer Out", amount, "Online transfer", accountId, receiver->getAccountId());
        Transaction receiverTrans("Transfer In", amount, "Online transfer", accountId, receiver->getAccountId());

        history.addTransaction(senderTrans);
        receiver->history.addTransaction(receiverTrans);

        setColor(10);
        cout << "Successfully transferred $" << amount
            << " to Account " << receiver->getAccountId() << endl;
        resetColor();

        saveToFile();
        receiver->saveToFile();
    }

    void receiveTransfer(double amount) {
        balance += amount;
        history.addTransaction(Transaction("Transfer In", amount, "Received transfer"));
    }

    void resetDailyLimit() {
        Date today = Date::getCurrentDate();
        if (!(today == lastResetDate)) {
            dailyWithdrawn = 0;
            lastResetDate = today;
        }
    }

    virtual void display() const {
        setColor(11);
        cout << "\n========== ACCOUNT DETAILS ==========\n";
        resetColor();
        cout << "Account ID: " << accountId << endl;
        cout << "Account Holder: " << accountHolder << endl;
        cout << "Account Type: " << accountType << endl;
        cout << "Balance: $";
        setColor(10);
        cout << balance << endl;
        resetColor();
        cout << "Status: ";
        setColor(isActive ? 10 : 12);
        cout << (isActive ? "Active" : "Blocked") << endl;
        resetColor();
        cout << "Daily Limit: $" << dailyLimit << endl;
        cout << "Today's Withdrawn: $" << dailyWithdrawn << endl;
        setColor(11);
        cout << "====================================\n";
        resetColor();
    }

    void showTransactionHistory() const {
        history.displayAll();
    }

    void blockAccount() {
        isActive = false;
        saveToFile();
    }

    void unblockAccount() {
        isActive = true;
        saveToFile();
    }

    void setDailyLimit(double limit) {
        if (limit < 0) {
            throw AccountException("Limit cannot be negative!");
        }
        dailyLimit = limit;
        saveToFile();
    }

    void updateAccountHolder(const string& newName) {
        if (newName.empty()) {
            throw AccountException("Name cannot be empty!");
        }
        accountHolder = newName;
        saveToFile();
    }

    void saveToFile() const {
        ofstream file(ACCOUNTS_FILE, ios::app);
        if (file.is_open()) {
            file << accountId << " " << accountHolder << " " << balance << " "
                << accountType << " " << isActive << " " << dailyLimit << endl;
            file.close();
        }
    }

    void loadFromFile(ifstream& file) {
        file >> accountId >> accountHolder >> balance >> accountType >> isActive >> dailyLimit;
        if (accountId > nextAccountId) {
            nextAccountId = accountId;
        }
    }
};

int BankAccount::nextAccountId = 10000;

class CurrentAccount : public BankAccount {
private:
    double overdraftLimit;
public:
    CurrentAccount(const string& name, double initial = 0)
        : BankAccount(name, "Current", initial), overdraftLimit(1000) {
    }

    void withdraw(double amount) override {
        if (!isActive) {
            throw AccountException("Account is blocked!");
        }
        if (amount <= 0) {
            throw TransactionException("Withdrawal amount must be positive!");
        }

        resetDailyLimit();
        if (dailyWithdrawn + amount > dailyLimit) {
            throw TransactionException("Daily withdrawal limit exceeded!");
        }

        if (amount > balance + overdraftLimit) {
            throw TransactionException("Exceeds overdraft limit!");
        }

        balance -= amount;
        dailyWithdrawn += amount;
        history.addTransaction(Transaction("Withdrawal", amount));
        setColor(10);
        cout << "Successfully withdrew $" << amount << " (Overdraft available)\n";
        resetColor();
        saveToFile();
    }

    void transfer(BankAccount* receiver, double amount) override {
        if (!isActive) {
            throw AccountException("Your account is blocked!");
        }
        if (!receiver->getIsActive()) {
            throw AccountException("Receiver account is blocked!");
        }
        if (amount <= 0) {
            throw TransactionException("Transfer amount must be positive!");
        }

        resetDailyLimit();
        if (dailyWithdrawn + amount > dailyLimit) {
            throw TransactionException("Daily transfer limit exceeded!");
        }

        if (amount > balance + overdraftLimit) {
            throw TransactionException("Exceeds overdraft limit!");
        }

        balance -= amount;
        receiver->receiveTransfer(amount);
        dailyWithdrawn += amount;

        Transaction senderTrans("Transfer Out", amount, "Online transfer", getAccountId(), receiver->getAccountId());
        Transaction receiverTrans("Transfer In", amount, "Online transfer", getAccountId(), receiver->getAccountId());

        history.addTransaction(senderTrans);
        receiver->addToHistory(receiverTrans);

        setColor(10);
        cout << "Successfully transferred $" << amount
            << " to Account " << receiver->getAccountId() << endl;
        resetColor();

        saveToFile();
        receiver->saveToFile();
    }

    void display() const override {
        BankAccount::display();
        cout << "Overdraft Limit: $";
        setColor(14);
        cout << overdraftLimit << endl;
        resetColor();
    }

    void setOverdraftLimit(double limit) {
        overdraftLimit = limit;
        saveToFile();
    }
};

class BankManagementSystem {
private:
    BankAccount** accounts;
    int accountCount;
    int capacity;
    string adminPassword;

    void resize() {
        capacity *= 2;
        BankAccount** newAccounts = new BankAccount * [capacity];
        for (int i = 0; i < accountCount; i++) {
            newAccounts[i] = accounts[i];
        }
        delete[] accounts;
        accounts = newAccounts;
    }

    BankAccount* findAccount(int accountId) {
        for (int i = 0; i < accountCount; i++) {
            if (accounts[i]->getAccountId() == accountId) {
                return accounts[i];
            }
        }
        return NULL;
    }

    void clearInputBuffer() {
        cin.clear();
        cin.ignore(1000, '\n');
    }

    bool askContinue(const string& message = "Do you want to continue? (y/n): ") {
        char choice;
        setColor(14);
        cout << message;
        resetColor();
        cin >> choice;
        clearInputBuffer();
        return (choice == 'y' || choice == 'Y');
    }

public:
    BankManagementSystem() : accountCount(0), capacity(20), adminPassword("admin123") {
        accounts = new BankAccount * [capacity];
        loadFromFile();
    }

    ~BankManagementSystem() {
        saveToFile();
        for (int i = 0; i < accountCount; i++) {
            delete accounts[i];
        }
        delete[] accounts;
    }

    void createAccount() {
        do {
            try {
                clearScreen();
                showHeader();

                string name;
                double initial;
                int choice;

                setColor(11);
                cout << "\n========== CREATE NEW ACCOUNT ==========\n";
                resetColor();
                cout << "Enter account holder name: ";
                clearInputBuffer();
                safeGetLine(name);

                if (name.empty()) {
                    throw AccountException("Name cannot be empty!");
                }

                cout << "Enter account type (1. Savings, 2. Current): ";
                if (!(cin >> choice)) {
                    clearInputBuffer();
                    throw AccountException("Invalid input! Please enter 1 or 2.");
                }

                if (choice != 1 && choice != 2) {
                    throw AccountException("Invalid account type! Please enter 1 or 2.");
                }

                cout << "Enter initial deposit: $";
                if (!(cin >> initial)) {
                    clearInputBuffer();
                    throw TransactionException("Invalid amount! Please enter numbers only.");
                }

                if (initial < 0) {
                    throw TransactionException("Initial deposit cannot be negative!");
                }

                BankAccount* newAccount;
                if (choice == 2) {
                    newAccount = new CurrentAccount(name, initial);
                }
                else {
                    newAccount = new BankAccount(name, "Savings", initial);
                }

                if (accountCount == capacity) {
                    resize();
                }

                accounts[accountCount++] = newAccount;
                setColor(10);
                cout << "\n Account created successfully!\n";
                cout << " Your Account ID: " << newAccount->getAccountId() << endl;
                resetColor();
                newAccount->display();

            }
            catch (BankException& e) {
                e.display();
                cout << "Please try again.\n";
            }
            catch (...) {
                cout << "Unexpected error occurred!\n";
            }
        } while (askContinue("Create another account? (y/n): "));
    }

    void customerMenu() {
        try {
            clearScreen();
            showHeader();

            int accountId;
            setColor(11);
            cout << "\n========== CUSTOMER LOGIN ==========\n";
            resetColor();
            cout << "Enter your Account ID: ";
            if (!(cin >> accountId)) {
                clearInputBuffer();
                throw AccountException("Invalid Account ID!");
            }

            BankAccount* account = findAccount(accountId);
            if (!account) {
                throw AccountException("Account not found!");
            }

            if (!account->getIsActive()) {
                throw AccountException("Account is blocked! Contact admin.");
            }

            int choice;
            do {
                clearScreen();
                showHeader();

                setColor(11);
                cout << "\n========== CUSTOMER MENU ==========\n";
                resetColor();
                cout << "Account: " << account->getAccountHolder()
                    << " (ID: " << account->getAccountId() << ")\n";
                cout << "1. Deposit Money\n";
                cout << "2. Withdraw Money\n";
                cout << "3. Online Transfer\n";
                cout << "4. Check Balance\n";
                cout << "5. View Account Details\n";
                cout << "6. View Transaction History\n";
                cout << "7. Return to Main Menu\n";
                setColor(14);
                cout << "Enter choice: ";
                resetColor();

                if (!(cin >> choice)) {
                    clearInputBuffer();
                    setColor(12);
                    cout << "Invalid choice! Please enter a number.\n";
                    resetColor();
                    system("pause");
                    continue;
                }

                switch (choice) {
                case 1: {
                    double amount;
                    cout << "Enter amount to deposit: $";
                    if (!(cin >> amount)) {
                        clearInputBuffer();
                        setColor(12);
                        cout << "Invalid amount!\n";
                        resetColor();
                        break;
                    }
                    try {
                        account->deposit(amount);
                    }
                    catch (BankException& e) {
                        e.display();
                    }
                    break;
                }
                case 2: {
                    double amount;
                    cout << "Enter amount to withdraw: $";
                    if (!(cin >> amount)) {
                        clearInputBuffer();
                        setColor(12);
                        cout << "Invalid amount!\n";
                        resetColor();
                        break;
                    }
                    try {
                        account->withdraw(amount);
                    }
                    catch (BankException& e) {
                        e.display();
                    }
                    break;
                }
                case 3: {
                    int receiverId;
                    double amount;
                    cout << "Enter receiver Account ID: ";
                    if (!(cin >> receiverId)) {
                        clearInputBuffer();
                        setColor(12);
                        cout << "Invalid Account ID!\n";
                        resetColor();
                        break;
                    }

                    if (receiverId == account->getAccountId()) {
                        setColor(12);
                        cout << "Cannot transfer to your own account!\n";
                        resetColor();
                        break;
                    }

                    BankAccount* receiver = findAccount(receiverId);
                    if (!receiver) {
                        setColor(12);
                        cout << "Receiver account not found!\n";
                        resetColor();
                        break;
                    }

                    cout << "Receiver: " << receiver->getAccountHolder() << endl;
                    cout << "Enter amount to transfer: $";
                    if (!(cin >> amount)) {
                        clearInputBuffer();
                        setColor(12);
                        cout << "Invalid amount!\n";
                        resetColor();
                        break;
                    }

                    try {
                        account->transfer(receiver, amount);
                    }
                    catch (BankException& e) {
                        e.display();
                    }
                    break;
                }
                case 4:
                    cout << "\nCurrent Balance: $";
                    setColor(10);
                    cout << account->getBalance() << endl;
                    resetColor();
                    break;
                case 5:
                    account->display();
                    break;
                case 6:
                    account->showTransactionHistory();
                    break;
                case 7:
                    cout << "Returning to main menu...\n";
                    break;
                default:
                    setColor(12);
                    cout << "Invalid choice!\n";
                    resetColor();
                }

                if (choice != 7) {
                    system("pause");
                    if (!askContinue()) {
                        choice = 7;
                    }
                }
            } while (choice != 7);

        }
        catch (BankException& e) {
            e.display();
            system("pause");
        }
        catch (...) {
            cout << "Unexpected error occurred!\n";
            system("pause");
        }
    }

    void adminMenu() {
        try {
            clearScreen();
            showHeader();

            string password;
            setColor(11);
            cout << "\n========== ADMIN LOGIN ==========\n";
            resetColor();
            cout << "Enter admin password: ";
            cin >> password;

            if (password != adminPassword) {
                throw AdminException("Invalid password!");
            }

            int choice;
            do {
                clearScreen();
                showHeader();

                setColor(11);
                cout << "\n========== ADMIN MENU ==========\n";
                resetColor();
                cout << "1. View All Accounts\n";
                cout << "2. Block Account\n";
                cout << "3. Unblock Account\n";
                cout << "4. Delete Account\n";
                cout << "5. Change Customer Info\n";
                cout << "6. Reset Daily Limit\n";
                cout << "7. View Transaction History\n";
                cout << "8. Change Admin Password\n";
                cout << "9. Return to Main Menu\n";
                setColor(14);
                cout << "Enter choice: ";
                resetColor();

                if (!(cin >> choice)) {
                    clearInputBuffer();
                    setColor(12);
                    cout << "Invalid choice!\n";
                    resetColor();
                    system("pause");
                    continue;
                }

                switch (choice) {
                case 1:
                    displayAllAccounts();
                    break;
                case 2:
                    blockUnblockAccount(true);
                    break;
                case 3:
                    blockUnblockAccount(false);
                    break;
                case 4:
                    deleteAccount();
                    break;
                case 5:
                    changeCustomerInfo();
                    break;
                case 6:
                    resetDailyLimit();
                    break;
                case 7:
                    viewAccountTransactions();
                    break;
                case 8:
                    changeAdminPassword();
                    break;
                case 9:
                    cout << "Returning to main menu...\n";
                    break;
                default:
                    setColor(12);
                    cout << "Invalid choice!\n";
                    resetColor();
                }

                if (choice != 9) {
                    system("pause");
                    if (!askContinue("Perform another admin action? (y/n): ")) {
                        choice = 9;
                    }
                }
            } while (choice != 9);

        }
        catch (BankException& e) {
            e.display();
            system("pause");
        }
        catch (...) {
            cout << "Unexpected error occurred!\n";
            system("pause");
        }
    }

    void displayAllAccounts() const {
        if (accountCount == 0) {
            setColor(12);
            cout << "No accounts found.\n";
            resetColor();
            return;
        }

        setColor(11);
        cout << "\n========== ALL ACCOUNTS ==========\n";
        resetColor();
        for (int i = 0; i < accountCount; i++) {
            accounts[i]->display();
        }
    }

    void blockUnblockAccount(bool block) {
        int accountId;
        cout << "Enter Account ID: ";
        if (!(cin >> accountId)) {
            clearInputBuffer();
            setColor(12);
            cout << "Invalid Account ID!\n";
            resetColor();
            return;
        }

        BankAccount* account = findAccount(accountId);
        if (!account) {
            setColor(12);
            cout << "Account not found!\n";
            resetColor();
            return;
        }

        if (block) {
            account->blockAccount();
            setColor(10);
            cout << "Account blocked successfully!\n";
            resetColor();
        }
        else {
            account->unblockAccount();
            setColor(10);
            cout << "Account unblocked successfully!\n";
            resetColor();
        }
    }

    void deleteAccount() {
        int accountId;
        cout << "Enter Account ID to delete: ";
        if (!(cin >> accountId)) {
            clearInputBuffer();
            setColor(12);
            cout << "Invalid Account ID!\n";
            resetColor();
            return;
        }

        for (int i = 0; i < accountCount; i++) {
            if (accounts[i]->getAccountId() == accountId) {
                cout << "Deleting account: " << accounts[i]->getAccountHolder() << endl;
                if (!askContinue("Are you sure? (y/n): ")) {
                    cout << "Deletion cancelled.\n";
                    return;
                }

                delete accounts[i];
                for (int j = i; j < accountCount - 1; j++) {
                    accounts[j] = accounts[j + 1];
                }
                accountCount--;
                setColor(10);
                cout << "Account deleted successfully!\n";
                resetColor();
                saveToFile();
                return;
            }
        }
        setColor(12);
        cout << "Account not found!\n";
        resetColor();
    }

    void changeCustomerInfo() {
        int accountId;
        cout << "Enter Account ID: ";
        if (!(cin >> accountId)) {
            clearInputBuffer();
            setColor(12);
            cout << "Invalid Account ID!\n";
            resetColor();
            return;
        }

        BankAccount* account = findAccount(accountId);
        if (!account) {
            setColor(12);
            cout << "Account not found!\n";
            resetColor();
            return;
        }

        string newName;
        cout << "Current name: " << account->getAccountHolder() << endl;
        cout << "Enter new account holder name: ";
        cin.ignore();
        safeGetLine(newName);

        if (!askContinue("Update name? (y/n): ")) {
            cout << "Update cancelled.\n";
            return;
        }

        try {
            account->updateAccountHolder(newName);
            setColor(10);
            cout << "Customer information updated successfully!\n";
            resetColor();
        }
        catch (BankException& e) {
            e.display();
        }
    }

    void resetDailyLimit() {
        int accountId;
        double newLimit;

        cout << "Enter Account ID: ";
        if (!(cin >> accountId)) {
            clearInputBuffer();
            setColor(12);
            cout << "Invalid Account ID!\n";
            resetColor();
            return;
        }

        BankAccount* account = findAccount(accountId);
        if (!account) {
            setColor(12);
            cout << "Account not found!\n";
            resetColor();
            return;
        }

        cout << "Current daily limit: $" << account->getBalance() << endl;
        cout << "Enter new daily limit: $";
        if (!(cin >> newLimit)) {
            clearInputBuffer();
            setColor(12);
            cout << "Invalid limit!\n";
            resetColor();
            return;
        }

        if (!askContinue("Update daily limit? (y/n): ")) {
            cout << "Update cancelled.\n";
            return;
        }

        try {
            account->setDailyLimit(newLimit);
            setColor(10);
            cout << "Daily limit updated successfully!\n";
            resetColor();
        }
        catch (BankException& e) {
            e.display();
        }
    }

    void viewAccountTransactions() {
        int accountId;
        cout << "Enter Account ID: ";
        if (!(cin >> accountId)) {
            clearInputBuffer();
            setColor(12);
            cout << "Invalid Account ID!\n";
            resetColor();
            return;
        }

        BankAccount* account = findAccount(accountId);
        if (!account) {
            setColor(12);
            cout << "Account not found!\n";
            resetColor();
            return;
        }

        account->showTransactionHistory();
    }

    void changeAdminPassword() {
        string oldPass, newPass;
        cout << "Enter old password: ";
        cin >> oldPass;

        if (oldPass != adminPassword) {
            setColor(12);
            cout << "Invalid old password!\n";
            resetColor();
            return;
        }

        cout << "Enter new password: ";
        cin >> newPass;

        if (!askContinue("Change password? (y/n): ")) {
            cout << "Password change cancelled.\n";
            return;
        }

        if (newPass.empty()) {
            setColor(12);
            cout << "Password cannot be empty!\n";
            resetColor();
            return;
        }

        adminPassword = newPass;
        setColor(10);
        cout << "Password changed successfully!\n";
        resetColor();
    }

    void saveToFile() {
        ofstream file(ACCOUNTS_FILE);
        if (file.is_open()) {
            for (int i = 0; i < accountCount; i++) {
                accounts[i]->saveToFile();
            }
            file.close();
        }
    }

    void loadFromFile() {
        ifstream file(ACCOUNTS_FILE);
        if (file.is_open()) {
            while (!file.eof()) {
                BankAccount* account = new BankAccount();
                account->loadFromFile(file);
                if (account->getAccountId() > 0) {
                    if (accountCount == capacity) {
                        resize();
                    }
                    accounts[accountCount++] = account;
                }
                else {
                    delete account;
                }
            }
            file.close();
        }
    }

    void run() {
        int choice;
        do {
            clearScreen();
            showHeader();

            setColor(14);
            cout << "\n1. Create New Account\n";
            cout << "2. Customer Login\n";
            cout << "3. Admin Login\n";
            cout << "4. Exit\n";
            cout << "Enter choice: ";
            resetColor();

            if (!(cin >> choice)) {
                clearInputBuffer();
                setColor(12);
                cout << "Invalid choice! Please enter a number.\n";
                resetColor();
                system("pause");
                continue;
            }

            switch (choice) {
            case 1:
                createAccount();
                break;
            case 2:
                customerMenu();
                break;
            case 3:
                adminMenu();
                break;
            case 4:
                clearScreen();
                showHeader();
                setColor(10);
                cout << "\nThank you for using our Banking System!\n";
                resetColor();
                saveToFile();
                break;
            default:
                setColor(12);
                cout << "Invalid choice! Try again.\n";
                resetColor();
                system("pause");
            }
        } while (choice != 4);
    }
};

int main() {
    clearScreen();
    showHeader();

    setColor(8);
    cout << "     Data Files: " << ACCOUNTS_FILE << ", " << TRANSACTIONS_FILE << "\n";
    resetColor();

    BankManagementSystem bank;

    try {
        bank.run();
    }
    catch (...) {
        setColor(12);
        cout << "A critical error occurred. System shutting down...\n";
        resetColor();
        return 1;
    }

    return 0;
}