#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>  // for atoi
using namespace std;

struct Account {
int accNo;
string name;
double balance;
};

// Helper: parse account line "accNo|name|balance"
bool parseAccountLine(const string &line, Account &acc) {
stringstream ss(line);
string token;
if (!getline(ss, token, '|')) return false;
acc.accNo = atoi(token.c_str());
if (!getline(ss, acc.name, '|')) return false;
if (!getline(ss, token, '|')) return false;
acc.balance = atof(token.c_str());
return true;
}

// Helper: build account line
string accountLine(const Account &acc) {
stringstream ss;
ss << acc.accNo << "|" << acc.name << "|" << acc.balance;
return ss.str();
}

// Get next account number (find max in file + 1)
int nextAccountNumber() {
ifstream fin("accounts.txt");
string line;
int maxNo = 1000; // start acc from 1001
while (getline(fin, line)) {
Account a;
if (parseAccountLine(line, a)) {
if (a.accNo > maxNo) maxNo = a.accNo;
}
}
fin.close();
return maxNo + 1;
}

// Append transaction to transactions.txt
// format: from|to|type|amount
void recordTransaction(int fromAcc, int toAcc, const string &type, double amount) {
ofstream fout("transactions.txt", ios::app);
fout << fromAcc << "|" << toAcc << "|" << type << "|" << amount << "\n";
fout.close();
}

// Create new account
void createAccount() {
Account acc;
acc.accNo = nextAccountNumber();
cout << "\n--- Create New Account ---\n";
cout << "Enter account holder name: ";
// read line (allow spaces)
cin.ignore();
getline(cin, acc.name);
cout << "Enter initial deposit amount: ";
cin >> acc.balance;
ofstream fout("accounts.txt", ios::app);
fout << accountLine(acc) << "\n";
fout.close();
cout << "Account created successfully!\n";
cout << "Account Number: " << acc.accNo << "\n";
// record initial deposit as transaction (fromAcc = 0 means bank)
recordTransaction(0, acc.accNo, "Deposit", acc.balance);
}

// Find account by accNo, return true if found and fills acc
bool findAccount(int accNo, Account &acc) {
ifstream fin("accounts.txt");
string line;
while (getline(fin, line)) {
Account a;
if (parseAccountLine(line, a) && a.accNo == accNo) {
acc = a;
fin.close();
return true;
}
}
fin.close();
return false;
}

// Update an account (replace line in accounts.txt)
bool updateAccount(const Account &updated) {
ifstream fin("accounts.txt");
ofstream fout("temp_accounts.txt");
string line;
bool updatedFlag = false;
while (getline(fin, line)) {
Account a;
if (parseAccountLine(line, a)) {
if (a.accNo == updated.accNo) {
fout << accountLine(updated) << "\n";
updatedFlag = true;
} else {
fout << line << "\n";
}
}
}
fin.close();
fout.close();
remove("accounts.txt");
rename("temp_accounts.txt", "accounts.txt");
return updatedFlag;
}

void displayAccountDetails() {
cout << "\n--- View Account Details ---\n";
cout << "Enter account number: ";
int accNo; cin >> accNo;
Account a;
if (findAccount(accNo, a)) {
cout << "\nAccount Number: " << a.accNo << "\n";
cout << "Account Holder: " << a.name << "\n";
cout << "Balance: " << a.balance << "\n";
} else {
cout << "Account not found.\n";
}
}

void deposit() {
cout << "\n--- Deposit ---\n";
cout << "Enter account number: ";
int accNo; cin >> accNo;
Account a;
if (!findAccount(accNo, a)) {
cout << "Account not found.\n";
return;
}
cout << "Enter amount to deposit: ";
double amt; cin >> amt;
if (amt <= 0) {
cout << "Invalid amount.\n";
return;
}
a.balance += amt;
if (updateAccount(a)) {
cout << "Deposit successful. New balance: " << a.balance << "\n";
recordTransaction(0, accNo, "Deposit", amt);
} else {
cout << "Error updating account.\n";
}
}

void withdraw() {
cout << "\n--- Withdraw ---\n";
cout << "Enter account number: ";
int accNo; cin >> accNo;
Account a;
if (!findAccount(accNo, a)) {
cout << "Account not found.\n";
return;
}
cout << "Enter amount to withdraw: ";
double amt; cin >> amt;
if (amt <= 0) {
cout << "Invalid amount.\n";
return;
}
if (amt > a.balance) {
cout << "Insufficient funds. Current balance: " << a.balance << "\n";
return;
}
a.balance -= amt;
if (updateAccount(a)) {
cout << "Withdrawal successful. New balance: " << a.balance << "\n";
recordTransaction(accNo, 0, "Withdraw", amt);
} else {
cout << "Error updating account.\n";
}
}

void transfer() {
    cout << "\n--- Transfer Funds ---\n";
    cout << "Enter source account number: ";
    int fromAcc; cin >> fromAcc;
    Account src;
    if (!findAccount(fromAcc, src)) {
        cout << "Source account not found.\n";
        return;
    }
    cout << "Enter destination account number: ";
    int toAcc; cin >> toAcc;
    Account dst;
    if (!findAccount(toAcc, dst)) {
        cout << "Destination account not found.\n";
        return;
    }
    cout << "Enter amount to transfer: ";
    double amt; cin >> amt;
    if (amt <= 0) {
        cout << "Invalid amount.\n";
        return;
    }
    if (amt > src.balance) {
        cout << "Insufficient funds in source. Balance: " << src.balance << "\n";
        return;
    }
    src.balance -= amt;
    dst.balance += amt;
    bool ok1 = updateAccount(src);
    bool ok2 = updateAccount(dst);
    if (ok1 && ok2) {
        cout << "Transfer successful.\n";
        cout << "Source new balance: " << src.balance << "\n";
        cout << "Destination new balance: " << dst.balance << "\n";
        recordTransaction(fromAcc, toAcc, "Transfer", amt);
    } else {
        cout << "Error updating accounts.\n";
    }
}

void showTransactionHistory() {
    cout << "\n--- Transaction History ---\n";
    cout << "Enter account number: ";
    int accNo; cin >> accNo;
    ifstream fin("transactions.txt");
    string line;
    bool found = false;
    cout << "\n(from -> to)  Type    Amount\n";
    cout << "----------------------------\n";
    while (getline(fin, line)) {
        stringstream ss(line);
        string token;
        int from, to;
        string type;
        double amt;
        if (!getline(ss, token, '|')) continue;
        from = atoi(token.c_str());
        if (!getline(ss, token, '|')) continue;
        to = atoi(token.c_str());
        if (!getline(ss, type, '|')) continue;
        if (!getline(ss, token, '|')) continue;
        amt = atof(token.c_str());
        if (from == accNo || to == accNo) {
cout << "(" << from << " -> " << to << ")  " << type << "   " << amt << "\n";
found = true;
        }
    }
    fin.close();
    if (!found) cout << "No transactions found for this account.\n";
}

void showAllAccounts() {
    cout << "\n--- All Accounts ---\n";
    ifstream fin("accounts.txt");
    string line;
    cout << "AccNo\tName\tBalance\n";
    cout << "---------------------------\n";
    while (getline(fin, line)) {
Account a;
if (parseAccountLine(line, a)) {
cout << a.accNo << "\t" << a.name << "\t" << a.balance << "\n";
}
    }
    fin.close();
}

int main() {
int choice;
while (true) {
cout << "\n====== SIMPLE BANKING SYSTEM ======\n";
cout << "1. Create Account\n";
cout << "2. View Account Details\n";
cout << "3. Deposit\n";
cout << "4. Withdraw\n";
cout << "5. Transfer\n";
cout << "6. Transaction History\n";
cout << "7. Show All Accounts\n";
cout << "8. Exit\n";
cout << "Enter your choice: ";
cin >> choice;
switch (choice) {
case 1: createAccount(); break;
case 2: displayAccountDetails(); break;
case 3: deposit(); break;
case 4: withdraw(); break;
case 5: transfer(); break;
case 6: showTransactionHistory(); break;
case 7: showAllAccounts(); break;
case 8: cout << "Thank you for using the banking system.\n"; return 0;
default: cout << "Invalid choice. Try again.\n";
}
}
return 0;
}