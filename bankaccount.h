#ifndef BANKACCOUNT_H
#define BANKACCOUNT_H

#include <QString>
#include <QDate>
#include <QVector>
#include <QFile>
#include <QTextStream>

class Transaction {
public:
    QString type;
    double amount;
    QDate date;
    QString description;

    Transaction(QString t = "", double a = 0, QString d = "")
        : type(t), amount(a), description(d) {
        date = QDate::currentDate();
    }
};

class BankAccount {
protected:
    static int nextAccountId;
    int accountId;
    QString holderName;
    double balance;
    QString accountType;
    bool isActive;
    QVector<Transaction> transactions;

public:
    BankAccount();
    BankAccount(QString name, QString type = "Savings", double initial = 0);

    int getId() const { return accountId; }
    QString getName() const { return holderName; }
    double getBalance() const { return balance; }
    bool getStatus() const { return isActive; }
    QString getType() const { return accountType; }
    QVector<Transaction> getTransactions() const { return transactions; }

    virtual void deposit(double amount);
    virtual void withdraw(double amount);
    virtual void transfer(BankAccount* receiver, double amount);

    void blockAccount();
    void unblockAccount();
    void updateName(QString newName);

    void saveToFile();
    void loadFromFile(QString data);

    virtual QString getInfo() const;

private:
    void addTransaction(QString type, double amount, QString desc = "");
};

#endif
