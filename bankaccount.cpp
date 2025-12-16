#include "bankaccount.h"

int BankAccount::nextAccountId = 10000;

BankAccount::BankAccount() : accountId(0), balance(0), isActive(true) {}

BankAccount::BankAccount(QString name, QString type, double initial)
    : accountId(++nextAccountId), holderName(name), balance(initial),
    accountType(type), isActive(true) {
    if(initial > 0) {
        addTransaction("Deposit", initial, "Initial deposit");
    }
}

void BankAccount::addTransaction(QString type, double amount, QString desc) {
    transactions.append(Transaction(type, amount, desc));
}

void BankAccount::deposit(double amount) {
    if(!isActive) return;
    if(amount <= 0) return;
    balance += amount;
    addTransaction("Deposit", amount);
}

void BankAccount::withdraw(double amount) {
    if(!isActive) return;
    if(amount <= 0) return;
    if(amount > balance) return;
    balance -= amount;
    addTransaction("Withdrawal", amount);
}

void BankAccount::transfer(BankAccount* receiver, double amount) {
    if(!isActive || !receiver->getStatus()) return;
    if(amount <= 0) return;
    if(amount > balance) return;
    balance -= amount;
    receiver->balance += amount;
    addTransaction("Transfer Out", amount, "To: " + QString::number(receiver->getId()));
    receiver->addTransaction("Transfer In", amount, "From: " + QString::number(accountId));
}

void BankAccount::blockAccount() { isActive = false; }
void BankAccount::unblockAccount() { isActive = true; }

void BankAccount::updateName(QString newName) {
    holderName = newName;
}

QString BankAccount::getInfo() const {
    return QString("ID: %1 | Name: %2 | Balance: $%3 | Type: %4 | Status: %5")
    .arg(accountId).arg(holderName).arg(balance, 0, 'f', 2)
        .arg(accountType).arg(isActive ? "Active" : "Blocked");
}

void BankAccount::saveToFile() {
    QFile file("accounts.dat");
    if(file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream out(&file);
        out << accountId << "," << holderName << "," << balance << ","
            << accountType << "," << isActive << "\n";
        file.close();
    }
}

void BankAccount::loadFromFile(QString data) {
    QStringList parts = data.split(",");
    if(parts.size() >= 5) {
        accountId = parts[0].toInt();
        holderName = parts[1];
        balance = parts[2].toDouble();
        accountType = parts[3];
        isActive = parts[4].toInt();
        if(accountId > nextAccountId) nextAccountId = accountId;
    }
}
