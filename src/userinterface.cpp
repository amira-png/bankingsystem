/*
 * userinterface.cpp
 *
 *  Created on: Nov 10, 2020
 *      Author: amira
 */

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <termios.h>
#include <unistd.h>
#endif
#include <string>
#include <limits>
#include <csignal>
#include "userinterface.h"

Ui::Ui() :
	m_session(nullptr), m_execute(nullptr) {
	m_operationsMap.clear();
	m_session = new Session();
	showWelcomeScreen();
	if (m_session->firstRun())
		registerSuperAdmin();
}

Ui::~Ui() {
	delete m_session;
	m_operationsMap.clear();
}

inline void Ui::printNewLines(int c) {
	for (int i = 0; i < c; i++)
		cout << endl;
}

void Ui::registerSuperAdmin() {

	string username;
	string firstname;
	string lastname;
	string nationalid;
	string password;
	string password_confirm;

	cout << "Registering the bank super administrator" << endl;
	printNewLines(1);

	Admin *super = new Admin();
	super->setId(m_session->genUserId());
	cout << "User name: ";
	cin >> username;
	cout << "First name: ";
	cin >> firstname;
	cout << "Last name: ";
	cin >> lastname;
	cout << "National ID: ";
	cin >> nationalid;
	printNewLines(1);
	do {
		password = string(getpass("Password: "));
		printNewLines(1);
		password_confirm = string(getpass("Confirm Password: "));
		if (password != password_confirm)
			cerr << "Password mismatch, Please try again" << endl;
	} while (password != password_confirm);

	super->setUserName(username);
	super->setFirstName(firstname);
	super->setLastName(lastname);
	super->setNationalId(nationalid);
	super->setPassword(m_session->encrypt(password));
	super->unlock();
	super->cap_AdminCreate(true);
	super->cap_AdminUpdate(true);
	super->cap_AdminDelete(true);
	super->cap_AdminActivate(true);
	super->cap_AdminDeactivate(true);
	super->cap_AdminListAll(true);
	super->cap_AdminPrintInfo(true);

	if (!m_session->createAdmin(super)) {
		cerr << "Error creating super administrator please contact the system developer"
			<< endl;
		exit(-1);
	} else {
		cout << "Bank super administrator created successfully, please login to setup more users"
			<< endl;
		run();
	}
}

void Ui::showWelcomeScreen() {

	vector<string> b;

	b.push_back("8888888888       888                                  888888b.                     888");
	b.push_back("888              888                                  888  \"88b                    888");
	b.push_back("888              888                                  888  .88P                    888");
	b.push_back("8888888 888  888 888888 888  888 888d888 .d88b.       8888888K.   8888b.  88888b.  888  888");
	b.push_back("888     888  888 888    888  888 888P\"  d8P  Y8b      888  \"Y88b     \"88b 888 \"88b 888 .88P");
	b.push_back("888     888  888 888    888  888 888    88888888      888    888 .d888888 888  888 888888K");
	b.push_back("888     Y88b 888 Y88b.  Y88b 888 888    Y8b.          888   d88P 888  888 888  888 888 \"88b");
	b.push_back("888      \"Y88888  \"Y888  \"Y88888 888     \"Y8888       8888888P\"  \"Y888888 888  888 888  888");

	for (vector<string>::iterator it = b.begin(); it != b.end(); ++it) {
		cout << *it << endl;
		usleep(100000);
	}

	printNewLines(2);
	const string version = "Version: " + (string)(VERSION);
	cout << "\t\t" << version << endl;
	cout << "\t\tCopyright © 2020: Amira Mohamed <amira.gnu@gmail.com>" << endl;
	cout << "\t\tLicense: General Public License V3 <https://www.gnu.org/licenses/gpl-3.0.en.html>";
	printNewLines(2);
}

void Ui::ui_change_password() {
	string username, password, confirm_password;

	cout << "Enter username: ";
	cin >> username;

	Person *p = m_session->getPerson(username);
	if (!p) {
		cerr << "No such user" << endl;
		return;
	}

	do {
		password = string(getpass("Enter new password: "));
		confirm_password = string(getpass("Confirm password: "));
		if (password != confirm_password)
			cerr << "Password mismatch, try again" << endl;
	} while (password != confirm_password);


	if (!m_session->changePassword(p, password)) {
		cerr << "Unauthorized to change the password" << endl;
		return;
	}
	else {
		cout << "Password changed successfully" << endl;
	}
}

void Ui::ui_change_own_password() {
	string password, confirm_password;
	string old_password;

	old_password = string(getpass("Current password: "));
	if (!m_session->verifyPassword(old_password)) {
		cerr << "Wrong password" << endl;
		return;
	}

	do {
		password = string(getpass("Enter new password: "));
		confirm_password = string(getpass("Confirm password: "));
		if (password != confirm_password)
			cerr << "Password mismatch, try again" << endl;
	} while (password != confirm_password);

	if (!m_session->changePassword(password)) {
		cerr << "Couldn't change password" << endl;
		return;
	}
	else {
		cout << "Password changed successfully" << endl;
	}
}

void Ui::getCallBack(const string &desc) {

	if ("Main Menu" == desc)
		m_execute = std::bind(&Ui::run, this);

	if ("Create Administrator" == desc)
		m_execute = std::bind(&Ui::ui_create_admin, this);
	if ("Update Administrator" == desc)
		m_execute = std::bind(&Ui::ui_update_admin, this);
	if ("Delete Administrator" == desc)
		m_execute = std::bind(&Ui::ui_delete_admin, this);
	if ("Activate Administrator" == desc)
		m_execute = std::bind(&Ui::ui_activate_admin, this);
	if ("Deactivate Administrator" == desc)
		m_execute = std::bind(&Ui::ui_deactivate_admin, this);
	if ("Print Administrator Information" == desc)
		m_execute = std::bind(&Ui::ui_print_admin, this);
	if ("Print my Administrator Information" == desc)
			m_execute = std::bind(&Ui::ui_print_own_admin, this);
	if ("List All Administrators" == desc)
		m_execute = std::bind(&Ui::ui_listall_admin, this);

	if ("Create Employee" == desc)
		m_execute = std::bind(&Ui::ui_create_employee, this);
	if ("Update Employee" == desc)
		m_execute = std::bind(&Ui::ui_update_employee, this);
	if ("Delete Employee" == desc)
		m_execute = std::bind(&Ui::ui_delete_employee, this);
	if ("Activate Employee" == desc)
		m_execute = std::bind(&Ui::ui_activate_employee, this);
	if ("Deactivate Employee" == desc)
		m_execute = std::bind(&Ui::ui_deactivate_employee, this);
	if ("Print Employee Information" == desc)
		m_execute = std::bind(&Ui::ui_print_employee, this);
	if ("Print my Employee Information" == desc)
		m_execute = std::bind(&Ui::ui_print_own_employee, this);
	if ("List All Employees" == desc)
		m_execute = std::bind(&Ui::ui_listall_employee, this);

	if ("Create Account" == desc)
		m_execute = std::bind(&Ui::ui_create_account, this);
	if ("Update Account" == desc)
		m_execute = std::bind(&Ui::ui_update_account, this);
	if ("Delete Account" == desc)
		m_execute = std::bind(&Ui::ui_delete_account, this);
	if ("Activate Account" == desc)
		m_execute = std::bind(&Ui::ui_activate_account, this);
	if ("Deactivate Account" == desc)
		m_execute = std::bind(&Ui::ui_deactivate_account, this);
	if ("Print Account Information" == desc)
		m_execute = std::bind(&Ui::ui_print_account, this);
	if ("List All Accounts" == desc)
		m_execute = std::bind(&Ui::ui_listall_account, this);
	if ("Transfer Money between accounts" == desc)
		m_execute = std::bind(&Ui::ui_transfer, this);
	if ("Deposit Money to Customer" == desc)
		m_execute = std::bind(&Ui::ui_deposit, this);

	if ("Create a new Customer" == desc)
		m_execute = std::bind(&Ui::ui_create_customer, this);
	if ("Update Existing Customer" == desc)
		m_execute = std::bind(&Ui::ui_update_customer, this);
	if ("Delete Customer" == desc)
		m_execute = std::bind(&Ui::ui_delete_customer, this);
	if ("Activate Customer" == desc)
		m_execute = std::bind(&Ui::ui_activate_customer, this);
	if ("Deactivate Customer" == desc)
		m_execute = std::bind(&Ui::ui_deactivate_customer, this);
	if ("Print Customer Information" == desc)
		m_execute = std::bind(&Ui::ui_print_customer, this);
	if ("List All Customers" == desc)
		m_execute = std::bind(&Ui::ui_listall_customer, this);
	if ("Print my customer Information" == desc)
		m_execute = std::bind(&Ui::ui_print_own_customer, this);
	if ("Print my account Information" == desc)
		m_execute = std::bind(&Ui::ui_print_own_account, this);
	if ("Transfer money to another Account" == desc)
		m_execute = std::bind(&Ui::ui_transfer_own, this);
	if ("Deposit" == desc)
		m_execute = std::bind(&Ui::ui_deposit_own, this);
	if ("Withdraw" == desc)
		m_execute = std::bind(&Ui::ui_withdraw, this);

	if ("Change Password" == desc)
		m_execute = std::bind(&Ui::ui_change_password, this);
	if ("Change own Password" == desc)
		m_execute = std::bind(&Ui::ui_change_own_password, this);
	if ("Logout" == desc)
		m_execute = std::bind(&Ui::logout, this);
	if ("Exit" == desc) {
		cout << "Bye Bye" << endl;
		exit(0);
	}
}

void Ui::listWhatToDo() {
	for (map<int, string>::iterator it = m_operationsMap.begin();
			it != m_operationsMap.end(); ++it)
		cout << it->first << " " << it->second << endl;
}

void Ui::logout() {
	printNewLines(1);
	cout << "Logging out" << endl;
	printNewLines(1);

	delete m_session;
	m_operationsMap.clear();
	m_capabilitiesLabels.clear();
	m_session = new Session();
	run();
}

int Ui::run() {

	string username, password;

	if (!m_session->isLoggedIn()) {
		cout << "Username: ";
		cin >> username;
		password = string(getpass("Password: "));

		bool loggedIn = m_session->login(username, password);
		if (!loggedIn) {
			cerr << "Login Failed" << endl;
			run();
		}
	}

	vector<string> v = m_session->getSessionCapabilities();
	m_capabilitiesLabels.push_back("Main Menu");
	copy(v.begin(), v.end(), back_inserter(m_capabilitiesLabels));
	m_capabilitiesLabels.push_back("Logout");
	m_capabilitiesLabels.push_back("Exit");
	v.clear();
	int counter = 1;
	for (string &capability : m_capabilitiesLabels) {
		m_operationsMap.insert(pair<int, string>(counter, capability));
		counter++;
	}
	m_capabilitiesLabels.clear();

	printNewLines(1);
	string headmsg = "Allowed operations for " + m_session->getSessionUser();
	int headsize = headmsg.length();
	for (int i = 0; i < headsize; i++) cout << "=";
	printNewLines(1);
	cout << headmsg << endl;
	for (int i = 0; i < headsize; i++) cout << "=";
	printNewLines(2);
	listWhatToDo();
	printNewLines(2);

	map<int, string>::iterator it;
	for (;;) {
		int operation = 0;
		string prompt = "(" + m_session->getSessionUser() + ") "
				+ "Select " + m_session->getSessionUserType() + " Operation: ";
		cout << prompt;
		cin >> operation;
		while (1) {
			if (cin.fail()) {
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				cout << prompt;
				cin >> operation;
			} else {
				it = m_operationsMap.find(operation);
				if (it != m_operationsMap.end()) {
					break;
				} else {
					cout <<prompt;
					cin >> operation;
				}
			}
		}

		getCallBack(m_operationsMap.find(operation)->second);
		m_execute();
	}
	return 0;
}
