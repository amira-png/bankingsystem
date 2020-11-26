/*
 * employee.cpp
 *
 *  Created on: Nov 3, 2020
 *      Author: amira
 */

#include <assert.h>
#include <typeinfo>
#ifdef __linux__
#include <unistd.h>
#include <libsmartcols/libsmartcols.h>
#include <locale.h>
#endif
#include <limits>
#include "session.h"
#include "userinterface.h"

/*
 * Session methods
 * */

bool Session::createAccount(Account *acct) {
	if (!isAuthorized(Session::ACCOUNT_CREATE))
		return false;

	if (!acct || (typeid(*acct) != typeid(Account)))
		return false;

	if (!m_db->insertAccount(acct)) {
		if (acct) delete acct;
		return false;
	}

	return true;
}

bool Session::updateAccount(Account *acct) {
	if (!isAuthorized(Session::ACCOUNT_UPDATE))
		return false;

	if (!acct || (typeid(*acct) != typeid(Account)))
		return false;

	if (!m_db->insertAccount(acct)) {
		if (acct) delete acct;
		return false;
	}

	return true;
}

bool Session::deleteAccount(Account *acct) {
	if (!isAuthorized(Session::ACCOUNT_DELETE))
		return false;

	if (!acct || (typeid(*acct) != typeid(Account)))
		return false;

	if (!m_db->deleteAccount(acct)) {
		if (acct) delete acct;
		return false;
	}

	return true;
}

bool Session::activateAccount(Account *acct) {
	if (!isAuthorized(Session::ACCOUNT_ACTIVATE))
		return false;

	if (!acct || (typeid(*acct) != typeid(Account)))
		return false;

	acct->unlock();

	if (!m_db->insertAccount(acct)) {
		if (acct) delete acct;
		return false;
	}

	return true;
}

bool Session::deactivateAccount(Account *acct) {
	if (!isAuthorized(Session::ACCOUNT_DEACTIVATE))
		return false;

	if (!acct || (typeid(*acct) != typeid(Account)))
		return false;

	acct->lock();

	if (!m_db->insertAccount(acct)) {
		if (acct) delete acct;
		return false;
	}

	return true;
}

bool Session::printAccountInfo(Account *acct) {

	if (!acct || (typeid(*acct) != typeid(Account)))
		return false;

	Customer *acct_owner;
	if (acct)
		acct_owner = getCustomerByAccount(acct->getId());
	else
		return false;

	if (!isAuthorized(Session::ACCOUNT_PRINT_INFO)
			&& (acct_owner->getId() != m_user->getId()))
		return false;

	string status = acct->isLocked() ? "Yes" : "No";
	struct libscols_table *tb;
	struct libscols_line *line, *accountid, *balance, *stat, *owner;
	setlocale(LC_ALL, "");

	tb = scols_new_table();
	scols_table_new_column(tb, "", 0.1, SCOLS_FL_TREE);
	line = accountid = scols_table_new_line(tb, NULL);
	scols_line_set_data(line, 0, string("ID: "
			+ to_string(acct->getId())).c_str());
	line = balance = scols_table_new_line(tb, accountid);
	scols_line_set_data(line, 0, string("Balance: "
			+ to_string(acct->getBalance())).c_str());

	if (acct_owner) {
		line = owner = scols_table_new_line(tb, accountid);
		scols_line_set_data(line, 0, string("Owner: "
				+ acct_owner->getFirstName()
				+ " " + acct_owner->getLastName()).c_str());
	}

	line = stat = scols_table_new_line(tb, accountid);
	scols_line_set_data(line, 0, string("Locked: " + status).c_str());

	scols_print_table(tb);
	scols_unref_table(tb);
	cout << endl;

	if (acct && acct_owner && acct_owner->getId() != m_user->getId()) delete acct;

	return true;
}

bool Session::ListAllAccounts() {
	if (!isAuthorized(Session::ACCOUNT_LIST_ALL))
		return false;

	vector<Account*> accounts = m_db->getAllAccounts();

	if (!accounts.size())
		return false;

	for (Account *account : accounts)
		printAccountInfo(account);

	accounts.clear();

	return true;
}

bool Session::createCustomer(Customer *customer) {
	if (!isAuthorized(Session::CUSTOMER_CREATE))
		return false;

	if (!customer || (typeid(*customer) != typeid(Customer)))
		return false;

	if (m_db->userExists(customer->getUserName()))
		return false;

	if (!m_db->insertPerson(customer)) {
		if (customer) delete customer;
		return false;
	}

	return true;
}

bool Session::updateCustomer(Customer *customer) {  // REVISIT ME
	if (!isAuthorized(Session::CUSTOMER_UPDATE))
		return false;

	if (!customer || (typeid(*customer) != typeid(Customer)))
		return false;

	if (!m_db->userExists(customer->getUserName()))
		return false;

	if (!m_db->insertPerson(customer)) {
		if (customer) delete customer;
		return false;
	}

	return true;
}

bool Session::deleteCustomer(Customer *customer) {
	if (!isAuthorized(Session::CUSTOMER_DELETE))
		return false;

	if (!customer || (typeid(*customer) != typeid(Customer)))
		return false;

	if (!m_db->userExists(customer->getUserName()))
		return false;

	if (!m_db->deletePerson(customer)) {
		if (customer) delete customer;
		return false;
	}
	return true;
}

bool Session::activateCustomer(Customer *customer) {
	if (!isAuthorized(Session::CUSTOMER_ACTIVATE))
		return false;

	if (!customer || (typeid(*customer) != typeid(Customer)))
		return false;

	if (!m_db->userExists(customer->getUserName()))
		return false;

	customer->unlock();

	if (!m_db->insertPerson(customer)) {
		if (customer) delete customer;
		return false;
	}

	return true;
}

bool Session::deactivateCustomer(Customer *customer) {
	if (!isAuthorized(Session::CUSTOMER_DEACTIVATE))
		return false;

	if (!customer || (typeid(*customer) != typeid(Customer)))
		return false;

	if (!m_db->userExists(customer->getUserName()))
		return false;

	customer->lock();

	if (!m_db->insertPerson(customer)){
		if (customer) delete customer;
		return false;
	}

	return true;
}

bool Session::printCustomerInfo(Customer *customer) {
	if (!isAuthorized(Session::CUSTOMER_PRINT_INFO) && (customer != m_user))
		return false;

	if (!customer || (typeid(*customer) != typeid(Customer)))
		return false;

	if (!m_db->userExists(customer->getUserName()))
		return false;

	string status = customer->isLocked() ? "Yes" : "No";
	struct libscols_table *tb;
	struct libscols_line *line, *username, *fullname, *id, *natid, *stat, *account;
	setlocale(LC_ALL, "");

	tb = scols_new_table();
	scols_table_new_column(tb, "", 0.1, SCOLS_FL_TREE);

	line = username = scols_table_new_line(tb, NULL);
	scols_line_set_data(line, 0, string(customer->getUserName()).c_str());
    line = fullname = scols_table_new_line(tb, username);
    scols_line_set_data(line, 0, string("Name: "
    		+ string(customer->getFirstName()
    		+ " " + customer->getLastName())).c_str());
    line = id = scols_table_new_line(tb, username);
    scols_line_set_data(line, 0, string("ID: "
    		+ to_string(customer->getId())).c_str());
    line = natid = scols_table_new_line(tb, username);
    scols_line_set_data(line, 0, string("National ID: "
    		+ customer->getNationalId()).c_str());
    line = stat = scols_table_new_line(tb, username);
    scols_line_set_data(line, 0, string("Locked: " + status).c_str());

    Account *acc = customer->getAccount();
    if (acc && acc->getId()!=0) {
        line = account = scols_table_new_line(tb, username);
        scols_line_set_data(line, 0, string("Account number: "
        		+ to_string(acc->getId())).c_str());
    }

    scols_print_table(tb);
    scols_unref_table(tb);
    cout << endl;

    if (customer && customer->getId() != m_user->getId()) delete customer;

	return true;
}

bool Session::ListAllCustomers() {
	if (!isAuthorized(Session::CUSTOMER_LIST_ALL))
		return false;

	vector<Customer*> customers = m_db->getAllCustomers();
	if (!customers.size())
		return false;

	for (Customer *customer : customers)
		printCustomerInfo(customer);

	customers.clear();

	return true;
}

bool Session::printEmployeeInfo() {
	Employee *emp = dynamic_cast<Employee*>(m_user);
	if (!bIsLoggedIn || !emp) return false;
	return printEmployeeInfo(emp);
}

bool Session::transfer(Account *from, Account *to, const int sum) {

	if (!from || !to || sum > from->getBalance())
		return false;

	int oldFromBalance = from->getBalance();
	int oldToBalance = to->getBalance();

	int newFromBalance = oldFromBalance - sum;
	int newToBalance = oldToBalance + sum;

	from->setBalance(newFromBalance);
	to->setBalance(newToBalance);

	if (!m_db->insertAccount(from)){
		if (from) delete from;
		return false;
	}

	if (!m_db->insertAccount(to)){
		if (to) delete to;
		return false;
	}

	return true;
}

bool Session::deposit(Account *acct, const int sum) {

	if (typeid(*acct) != typeid(Account))
		return false;

	if (!acct)
		return false;

	int newBalance = acct->getBalance() + sum;
	acct->setBalance(newBalance);

	if (!m_db->insertAccount(acct)){
		if (acct) delete acct;
		return false;
	}

	return true;
}


/*
 * User interface methods
 * */

void Ui::ui_print_own_employee() {
	if (!m_session->printEmployeeInfo())
		cerr << "Error printing my employee info" << endl;
}

void Ui::ui_create_customer() {

	string username;
	string firstname;
	string lastname;
	string nationalid;
	string password;
	string password_confirm;

	cout << "Registering a new customer" << endl;
	cout << endl;

	Customer *tmp = new Customer();
	tmp->setId(m_session->genUserId());

	cout << "User name: ";
	cin >> username;
	cout << "First name: ";
	cin >> firstname;
	cout << "Last name: ";
	cin >> lastname;

	cout << "National ID: ";
	cin >> nationalid;
	cout << endl;

	do {
		password = string(getpass("Password: "));
		cout << endl;
		password_confirm = string(getpass("Confirm Password: "));
		if (password != password_confirm)
			cerr << "Password mismatch, Please try again" << endl;
	} while (password != password_confirm);

	tmp->setUserName(username);
	tmp->setFirstName(firstname);
	tmp->setLastName(lastname);
	tmp->setNationalId(nationalid);
	tmp->setPassword(m_session->encrypt(password));

	tmp->lock();

	tmp->setAccount(nullptr);

	if (!m_session->createCustomer(tmp)) {
		cerr << "Error creating the customer please contact the an  administrator"
			<< endl;
		return;
	} else {
		cout << "Customer was created successfully, please login to continue working"
			<< endl;
	}
}

void Ui::ui_update_customer() {

	string username;
	string firstname;
	string lastname;
	string nationalid;
	Customer *tmp;

	cout << "Updating a Customer" << endl;
	cout << endl;

	do {
		cout << "Customer user name: ";
		cin >> username;
		tmp = m_session->getCustomer(username);
		if (!tmp)
			cerr << "Customer account doesn't exit" << endl;
	} while (!tmp);

	cout << "First name: ";
	cin >> firstname;
	cout << "Last name: ";
	cin >> lastname;
	cout << "National ID: ";
	cin >> nationalid;
	cout << endl;

	tmp->setFirstName(firstname);
	tmp->setLastName(lastname);
	tmp->setNationalId(nationalid);

	tmp->isLocked() ? tmp->lock() : tmp->unlock();

	if (!m_session->updateCustomer(tmp)) {
		cerr << "Error updating " << username
				<< " please contact an admin" << endl;
		return;
	} else {
		cout << "Customer was updated successfully, please login to continue working"
			<< endl;
	}
}

void Ui::ui_delete_customer() {

	string username = "";
	Customer *tmp;
	cout << "Enter Customer's user name to delete: ";
	cin >> username;
	tmp = m_session->getCustomer(username);
	if (tmp) {
		if (!m_session->deleteCustomer(tmp)) {
			cerr << "Failed to delete customer acount: " << username << endl;
			return;
		}
		else
			cout << "Deleted customer account: " << username << endl;
	} else
		cout << "Failed to query the deleting desired Customer: " << username
				<< endl;
}

void Ui::ui_activate_customer() {

	string username;
	Customer *tmp;

	do {
		cout << "Customer username: ";
		cin >> username;
		tmp = m_session->getCustomer(username);
		if (!tmp)
			cerr << "Customer account doesn't exit" << endl;
	} while (!tmp);

	if (!m_session->activateCustomer(tmp)) {
		cerr << "Error activating " << username
				<< " please contact an Administrator" << endl;
		return;
	} else
		cout << "Customer was activated successfully, please login to continue working"
			<< endl;
}

void Ui::ui_deactivate_customer() {

	string username;
	Customer *tmp;

	do {
		cout << "Customer username: ";
		cin >> username;
		tmp = m_session->getCustomer(username);
		if (!tmp)
			cerr << "Customer account doesn't exit" << endl;
	} while (!tmp);

	if (!m_session->deactivateCustomer(tmp)) {
		cerr << "Error activating " << username
				<< " please contact an Administrator" << endl;
		return;
	} else
		cout << "Customer was deactivated successfully" << endl;
}

void Ui::ui_print_customer() {

	string username;
	cout << "Customer username: ";
	cin >> username;
	Customer *cust = m_session->getCustomer(username);
	if (!cust) {
		cerr << "No such customer" << endl;
		return;
	}

	if (!m_session->printCustomerInfo(cust)) {
		cerr << "Failed to print customer info" << endl;
		return;
	}
}

void Ui::ui_listall_customer() {
	m_session->ListAllCustomers();
}

void Ui::ui_create_account() {

	Account *acct = new Account();
	acct->setId(m_session->genAccountId());
	string customer_name = "";
	cout << "Enter customer name: ";
	cin >> customer_name;
	Customer *cust = m_session->getCustomer(customer_name);
	if (!cust) {
		cerr << "No such customer, Can't create account" << endl;
		return;
	}

	acct->setCustomerId(cust->getId());
	acct->setBalance(0);

	if (!m_session->createAccount(acct)) {
		cerr << "Error creating an account" << endl;
		return;
	} else {
		cout << "Account " << acct->getId() << " created successfully" << endl;
	}
}

void Ui::ui_update_account() {

	int account_number;
	cout << "Enter account number: ";
	cin >> account_number;

	while (1) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter account number: ";
			cin >> account_number;
		} else
			break;
	}

	Account *acct = m_session->getAccount(account_number);

	if (!acct) {
		cerr << "No such account" << endl;
		return;
	}

	int newbalance = 0;
	cout << "Enter new balance: ";
	cin >> newbalance;

	while (1) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter new balance: ";
			cin >> newbalance;
		} else
			break;
	}

	if (newbalance > 0) {
		acct->setBalance(newbalance);
	}

	if (!m_session->createAccount(acct)) {
		cerr << "Error creating an account" << endl;
	} else {
		cout << "Account created successfully" << endl;
		delete acct;
	}
}

void Ui::ui_delete_account() {
	int account_number;
	cout << "Enter Account Number: ";
	cin >> account_number;
	while (1) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter account number: ";
			cin >> account_number;
		} else
			break;
	}

	Account *tmp = m_session->getAccount(account_number);
	if (!tmp) {
		cerr << "No such account"<< endl;
		return;
	}

	if (!m_session->deleteAccount(tmp)) {
		cerr << "Error deleting account" << endl;
	} else
		cout << "Account deleted Successfully" << endl;
}

void Ui::ui_activate_account() {
	int account_number;
	cout << "Enter Account Number: ";
	cin >> account_number;
	while (1) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter account number: ";
			cin >> account_number;
		} else
			break;
	}
	Account *acct = m_session->getAccount(account_number);
	if (!acct) {
		cerr << "No such account" << endl;
		return;
	}

	if (!m_session->activateAccount(acct)) {
		cerr << "Failed to activate account" << endl;
	} else
		cout << "Account activated succesfully" << endl;
}

void Ui::ui_deactivate_account() {

	int account_number;
	cout << "Enter Account Number: ";
	cin >> account_number;
	while (1) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter account number: ";
			cin >> account_number;
		} else
			break;
	}
	Account *acct = m_session->getAccount(account_number);
	if (!acct) {
		cerr << "No such account" << endl;
		return;
	}

	if (!m_session->deactivateAccount(acct)) {
		cerr << "Failed to deactivate account" << endl;
		return;
	} else
		cout << "Account deactivated succesfully" << endl;
}

void Ui::ui_print_account() {

	int account_number;
	cout << "Enter account number:";
	cin >> account_number;
	while (1) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter account number: ";
			cin >> account_number;
		} else
			break;
	}

	Account *tmp = m_session->getAccount(account_number);
	if (!tmp) {
		cerr << "No such account" << endl;
		return;
	}

	if (!m_session->printAccountInfo(tmp)) {
		cerr << "Failed to print account info" << endl;
	}

}

void Ui::ui_listall_account() {
	m_session->ListAllAccounts();
}

void Ui::ui_transfer() {
	int source_account_number;
	int destination_account_number;
	int sum;

	cout << "Enter source account number: ";
	cin >> source_account_number;
	while (1) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter source account number: ";
			cin >> source_account_number;
		} else
			break;
	}

	Account *sacct = m_session->getAccount(source_account_number);
	if (!sacct) {
		cerr << "Invalid source account number";
		return;
	}

	cout << "Enter destination account number: ";
	cin >> destination_account_number;
	while (1) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter destination account number: ";
			cin >> destination_account_number;
		} else
			break;
	}

	Account *dacct = m_session->getAccount(destination_account_number);

	if (!dacct) {
		cerr << "Invalid destination account number";
		return;
	}

	cout << "Enter sum: ";
	cin >> sum;
	while (1) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter sum: ";
			cin >> sum;
		} else
			break;
	}

	if (!m_session->transfer(sacct, dacct, sum)) {
		cerr << "Amount or bank accounts are invalid" << endl;
	}
	else {
		cout << "Successfull transfer" << endl;
	}

}

void Ui::ui_deposit() {
	int account_number;
	int sum;

	cout << "Enter account number: ";
	cin >> account_number;
	while (1) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter account number: ";
			cin >> account_number;
		} else
			break;
	}

	Account *acct = m_session->getAccount(account_number);
	if (!acct) {
		cerr << "Invalid account" << endl;
		return;
	}

	cout << "Enter sum: ";
	cin >> sum;
	while (1) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter sum: ";
			cin >> sum;
		} else
			break;
	}


	if (!m_session->deposit(acct, sum)) {
		cerr << "Amount or bank account are invalid" << endl;
	}
	else {
		cout << "Successfull deposit" << endl;
	}
}

