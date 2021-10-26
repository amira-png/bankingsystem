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

	auto *super = new Admin();
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
	super->setPassword(Session::encrypt(password));
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

	b.emplace_back("8888888888       888                                  888888b.                     888");
	b.emplace_back("888              888                                  888  \"88b                    888");
	b.emplace_back("888              888                                  888  .88P                    888");
	b.emplace_back("8888888 888  888 888888 888  888 888d888 .d88b.       8888888K.   8888b.  88888b.  888  888");
	b.emplace_back("888     888  888 888    888  888 888P\"  d8P  Y8b      888  \"Y88b     \"88b 888 \"88b 888 .88P");
	b.emplace_back("888     888  888 888    888  888 888    88888888      888    888 .d888888 888  888 888888K");
	b.emplace_back("888     Y88b 888 Y88b.  Y88b 888 888    Y8b.          888   d88P 888  888 888  888 888 \"88b");
	b.emplace_back("888      \"Y88888  \"Y888  \"Y88888 888     \"Y8888       8888888P\"  \"Y888888 888  888 888  888");

	for (auto & it : b) {
		cout << it << endl;
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
		m_execute = [this] { run(); };

	if ("Create Administrator" == desc)
		m_execute = [this] { ui_create_admin(); };
	if ("Update Administrator" == desc)
		m_execute = [this] { ui_update_admin(); };
	if ("Delete Administrator" == desc)
		m_execute = [this] { ui_delete_admin(); };
	if ("Activate Administrator" == desc)
		m_execute = [this] { ui_activate_admin(); };
	if ("Deactivate Administrator" == desc)
		m_execute = [this] { ui_deactivate_admin(); };
	if ("Print Administrator Information" == desc)
		m_execute = [this] { ui_print_admin(); };
	if ("Print my Administrator Information" == desc)
			m_execute = [this] { ui_print_own_admin(); };
	if ("List All Administrators" == desc)
		m_execute = [this] { ui_listall_admin(); };

	if ("Create Employee" == desc)
		m_execute = [this] { ui_create_employee(); };
	if ("Update Employee" == desc)
		m_execute = [this] { ui_update_employee(); };
	if ("Delete Employee" == desc)
		m_execute = [this] { ui_delete_employee(); };
	if ("Activate Employee" == desc)
		m_execute = [this] { ui_activate_employee(); };
	if ("Deactivate Employee" == desc)
		m_execute = [this] { ui_deactivate_employee(); };
	if ("Print Employee Information" == desc)
		m_execute = [this] { ui_print_employee(); };
	if ("Print my Employee Information" == desc)
		m_execute = [this] { ui_print_own_employee(); };
	if ("List All Employees" == desc)
		m_execute = [this] { ui_listall_employee(); };

	if ("Create Account" == desc)
		m_execute = [this] { ui_create_account(); };
	if ("Update Account" == desc)
		m_execute = [this] { ui_update_account(); };
	if ("Delete Account" == desc)
		m_execute = [this] { ui_delete_account(); };
	if ("Activate Account" == desc)
		m_execute = [this] { ui_activate_account(); };
	if ("Deactivate Account" == desc)
		m_execute = [this] { ui_deactivate_account(); };
	if ("Print Account Information" == desc)
		m_execute = [this] { ui_print_account(); };
	if ("List All Accounts" == desc)
		m_execute = [this] { ui_listall_account(); };
	if ("Transfer Money between accounts" == desc)
		m_execute = [this] { ui_transfer(); };
	if ("Deposit Money to Customer" == desc)
		m_execute = [this] { ui_deposit(); };

	if ("Create a new Customer" == desc)
		m_execute = [this] { ui_create_customer(); };
	if ("Update Existing Customer" == desc)
		m_execute = [this] { ui_update_customer(); };
	if ("Delete Customer" == desc)
		m_execute = [this] { ui_delete_customer(); };
	if ("Activate Customer" == desc)
		m_execute = [this] { ui_activate_customer(); };
	if ("Deactivate Customer" == desc)
		m_execute = [this] { ui_deactivate_customer(); };
	if ("Print Customer Information" == desc)
		m_execute = [this] { ui_print_customer(); };
	if ("List All Customers" == desc)
		m_execute = [this] { ui_listall_customer(); };
	if ("Print my customer Information" == desc)
		m_execute = [this] { ui_print_own_customer(); };
	if ("Print my account Information" == desc)
		m_execute = [this] { ui_print_own_account(); };
	if ("Transfer money to another Account" == desc)
		m_execute = [this] { ui_transfer_own(); };
	if ("Deposit" == desc)
		m_execute = [this] { ui_deposit_own(); };
	if ("Withdraw" == desc)
		m_execute = [this] { ui_withdraw(); };

	if ("Change Password" == desc)
		m_execute = [this] { ui_change_password(); };
	if ("Change own Password" == desc)
		m_execute = [this] { ui_change_own_password(); };
	if ("Logout" == desc)
		m_execute = [this] { logout(); };
	if ("Exit" == desc) {
		cout << "Bye Bye" << endl;
		exit(0);
	}
}

void Ui::listWhatToDo() {
    for (auto & it : m_operationsMap) { cout << it.first << " " << it.second << endl; }
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

[[noreturn]] int Ui::run() {

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
	m_capabilitiesLabels.emplace_back("Main Menu");
	copy(v.begin(), v.end(), back_inserter(m_capabilitiesLabels));
	m_capabilitiesLabels.emplace_back("Logout");
	m_capabilitiesLabels.emplace_back("Exit");
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
		while (true) {
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
}
