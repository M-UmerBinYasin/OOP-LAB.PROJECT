#include "currentaccount.h"

CurrentAccount::CurrentAccount(QString name, double initial)
    : BankAccount(name, "Current", initial), overdraftLimit(1000) {}

void CurrentAccount::withdraw(double amount) {
    if(!getStatus()) return;
    if(amount <= 0) return;
    if(amount > getBalance() + overdraftLimit) return;
    BankAccount::withdraw(amount);
}

QString CurrentAccount::getInfo() const {
    return BankAccount::getInfo() + QString(" | Overdraft: $%1").arg(overdraftLimit, 0, 'f', 2);
}
