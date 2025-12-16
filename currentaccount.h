#ifndef CURRENTACCOUNT_H
#define CURRENTACCOUNT_H

#include "bankaccount.h"

class CurrentAccount : public BankAccount {
    double overdraftLimit;

public:
    CurrentAccount(QString name, double initial = 0);
    void withdraw(double amount) override;
    QString getInfo() const override;
};

#endif
