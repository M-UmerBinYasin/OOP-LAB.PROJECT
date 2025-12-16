#ifndef BANKMAINWINDOW_H
#define BANKMAINWINDOW_H

#include <QMainWindow>
#include "bankaccount.h"
#include "currentaccount.h"

QT_BEGIN_NAMESPACE
namespace Ui { class BankMainWindow; }
QT_END_NAMESPACE

class BankMainWindow : public QMainWindow {
    Q_OBJECT

public:
    BankMainWindow(QWidget *parent = nullptr);
    ~BankMainWindow();

private slots:
    void on_createBtn_clicked();
    void on_loginBtn_clicked();
    void on_depositBtn_clicked();
    void on_withdrawBtn_clicked();
    void on_transferBtn_clicked();
    void on_logoutBtn_clicked();
    void on_adminLoginBtn_clicked();
    void on_blockBtn_clicked();
    void on_unblockBtn_clicked();
    void on_changeNameBtn_clicked();
    void on_searchBtn_clicked();

private:
    Ui::BankMainWindow *ui;
    QVector<BankAccount*> accounts;
    BankAccount* currentCustomer;
    bool adminLoggedIn;

    BankAccount* findAccount(int id);
    void updateCustomerDisplay();
    void updateAdminDisplay();
    void loadAccountsFromFile();
    void saveAccountsToFile();
    double getAmountInput(QString title, QString label);
    int getAccountIdInput(QString title, QString label);
};

#endif
