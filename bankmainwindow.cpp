#include "bankmainwindow.h"
#include "ui_bankmainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QFile>
#include <QTextStream>

BankMainWindow::BankMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::BankMainWindow),
    currentCustomer(nullptr), adminLoggedIn(false) {
    ui->setupUi(this);
    setWindowTitle("Bank Management System");
    loadAccountsFromFile();
    ui->custOperations->setEnabled(false);
    ui->adminOperations->setEnabled(false);
}

BankMainWindow::~BankMainWindow() {
    saveAccountsToFile();
    for(auto acc : accounts) delete acc;
    delete ui;
}

void BankMainWindow::on_createBtn_clicked() {
    QString name = ui->nameInput->text().trimmed();
    if(name.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a name!");
        return;
    }
    double deposit = ui->depositSpin->value();
    int typeIndex = ui->typeCombo->currentIndex();

    BankAccount* newAccount;
    if(typeIndex == 0) newAccount = new BankAccount(name, "Savings", deposit);
    else newAccount = new CurrentAccount(name, deposit);

    accounts.append(newAccount);
    newAccount->saveToFile();

    ui->nameInput->clear();
    ui->depositSpin->setValue(0);
    QMessageBox::information(this, "Success",
                             QString("Account created!\nID: %1\nType: %2\nBalance: $%3")
                                 .arg(newAccount->getId()).arg(newAccount->getType()).arg(deposit, 0, 'f', 2));
}

void BankMainWindow::on_loginBtn_clicked() {
    bool ok;
    int accountId = ui->custIdInput->text().toInt(&ok);
    if(!ok) {
        QMessageBox::warning(this, "Error", "Enter valid account ID!");
        return;
    }
    currentCustomer = findAccount(accountId);
    if(!currentCustomer) {
        QMessageBox::warning(this, "Error", "Account not found!");
        return;
    }
    if(!currentCustomer->getStatus()) {
        QMessageBox::warning(this, "Error", "Account blocked!");
        return;
    }
    ui->custOperations->setEnabled(true);
    ui->custIdInput->clear();
    updateCustomerDisplay();
    QMessageBox::information(this, "Success", "Welcome " + currentCustomer->getName());
}

void BankMainWindow::on_depositBtn_clicked() {
    if(!currentCustomer) return;
    double amount = getAmountInput("Deposit", "Enter amount:");
    if(amount <= 0) return;
    currentCustomer->deposit(amount);
    currentCustomer->saveToFile();
    updateCustomerDisplay();
    QMessageBox::information(this, "Success", QString("Deposited $%1").arg(amount, 0, 'f', 2));
}

void BankMainWindow::on_withdrawBtn_clicked() {
    if(!currentCustomer) return;
    double amount = getAmountInput("Withdraw", "Enter amount:");
    if(amount <= 0) return;
    currentCustomer->withdraw(amount);
    currentCustomer->saveToFile();
    updateCustomerDisplay();
    QMessageBox::information(this, "Success", QString("Withdrew $%1").arg(amount, 0, 'f', 2));
}

void BankMainWindow::on_transferBtn_clicked() {
    if(!currentCustomer) return;
    int receiverId = getAccountIdInput("Transfer", "Receiver Account ID:");
    if(receiverId <= 0) return;
    if(receiverId == currentCustomer->getId()) {
        QMessageBox::warning(this, "Error", "Cannot transfer to yourself!");
        return;
    }
    BankAccount* receiver = findAccount(receiverId);
    if(!receiver) {
        QMessageBox::warning(this, "Error", "Receiver not found!");
        return;
    }
    double amount = getAmountInput("Transfer", "Enter amount:");
    if(amount <= 0) return;
    currentCustomer->transfer(receiver, amount);
    currentCustomer->saveToFile();
    receiver->saveToFile();
    updateCustomerDisplay();
    QMessageBox::information(this, "Success", QString("Transferred $%1").arg(amount, 0, 'f', 2));
}

void BankMainWindow::on_logoutBtn_clicked() {
    currentCustomer = nullptr;
    ui->custOperations->setEnabled(false);
    ui->transactionsList->clear();
    ui->custInfo->clear();
    QMessageBox::information(this, "Info", "Logged out!");
}

void BankMainWindow::on_adminLoginBtn_clicked() {
    QString password = ui->adminPassInput->text();
    if(password == "admin123") {
        adminLoggedIn = true;
        ui->adminOperations->setEnabled(true);
        ui->adminPassInput->clear();
        updateAdminDisplay();
        QMessageBox::information(this, "Success", "Admin login successful!");
    } else {
        QMessageBox::warning(this, "Error", "Wrong password!");
    }
}

void BankMainWindow::on_blockBtn_clicked() {
    if(!adminLoggedIn) return;
    int accountId = getAccountIdInput("Block", "Account ID:");
    BankAccount* account = findAccount(accountId);
    if(account) {
        account->blockAccount();
        account->saveToFile();
        updateAdminDisplay();
        QMessageBox::information(this, "Success", "Account blocked!");
    }
}

void BankMainWindow::on_unblockBtn_clicked() {
    if(!adminLoggedIn) return;
    int accountId = getAccountIdInput("Unblock", "Account ID:");
    BankAccount* account = findAccount(accountId);
    if(account) {
        account->unblockAccount();
        account->saveToFile();
        updateAdminDisplay();
        QMessageBox::information(this, "Success", "Account unblocked!");
    }
}

void BankMainWindow::on_changeNameBtn_clicked() {
    if(!adminLoggedIn) return;
    int accountId = getAccountIdInput("Change Name", "Account ID:");
    BankAccount* account = findAccount(accountId);
    if(account) {
        bool ok;
        QString newName = QInputDialog::getText(this, "Change Name",
                                                "New name:", QLineEdit::Normal, account->getName(), &ok);
        if(ok && !newName.isEmpty()) {
            account->updateName(newName);
            account->saveToFile();
            updateAdminDisplay();
            QMessageBox::information(this, "Success", "Name updated!");
        }
    }
}

void BankMainWindow::on_searchBtn_clicked() {
    QString search = ui->searchInput->text().toLower();
    ui->accountsList->clear();
    for(auto acc : accounts) {
        if(acc->getName().toLower().contains(search) ||
            QString::number(acc->getId()).contains(search)) {
            ui->accountsList->addItem(acc->getInfo());
        }
    }
}

BankAccount* BankMainWindow::findAccount(int id) {
    for(auto acc : accounts) if(acc->getId() == id) return acc;
    return nullptr;
}

void BankMainWindow::updateCustomerDisplay() {
    if(!currentCustomer) return;
    ui->custInfo->setText(currentCustomer->getInfo());
    ui->transactionsList->clear();
    for(const auto& t : currentCustomer->getTransactions()) {
        QString item = QString("%1 | $%2 | %3 | %4")
        .arg(t.date.toString("dd/MM/yyyy"))
            .arg(t.amount, 0, 'f', 2)
            .arg(t.type).arg(t.description);
        ui->transactionsList->addItem(item);
    }
}

void BankMainWindow::updateAdminDisplay() {
    ui->accountsList->clear();
    for(auto acc : accounts) ui->accountsList->addItem(acc->getInfo());
}

void BankMainWindow::loadAccountsFromFile() {
    QFile file("accounts.dat");
    if(!file.exists()) return;
    if(file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while(!in.atEnd()) {
            QString line = in.readLine();
            if(line.isEmpty()) continue;
            QStringList parts = line.split(",");
            if(parts.size() >= 5) {
                BankAccount* account;
                if(parts[3] == "Current") account = new CurrentAccount(parts[1], parts[2].toDouble());
                else account = new BankAccount(parts[1], "Savings", parts[2].toDouble());
                account->loadFromFile(line);
                accounts.append(account);
            }
        }
        file.close();
    }
}

void BankMainWindow::saveAccountsToFile() {
    QFile file("accounts.dat");
    if(file.open(QIODevice::WriteOnly)) {
        QTextStream out(&file);
        for(auto acc : accounts) acc->saveToFile();
        file.close();
    }
}

double BankMainWindow::getAmountInput(QString title, QString label) {
    bool ok;
    double amount = QInputDialog::getDouble(this, title, label, 0, 0, 1000000, 2, &ok);
    return ok ? amount : 0;
}

int BankMainWindow::getAccountIdInput(QString title, QString label) {
    bool ok;
    int id = QInputDialog::getInt(this, title, label, 0, 0, 999999, 1, &ok);
    return ok ? id : 0;
}
