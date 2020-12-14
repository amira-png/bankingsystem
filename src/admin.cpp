/*
 * admin.cpp

 *
 *  Created on: Oct 19, 2020
 *      Author: amira
 */
#include <assert.h>
#include <typeinfo>
#include "session.h"
#include "userinterface.h"
#ifdef __linux__
#include <unistd.h>
#include <libsmartcols/libsmartcols.h>
#include <locale.h>
#endif

/*
 * Session methods
 * */

bool Session::createAdmin(Admin *admin) {
	if (!isAuthorized(Session::ADMIN_CREATE) && m_totalUsers > 0)
		return false;

	if (!admin || (typeid(*admin) != typeid(Admin)))
		return false;

	if (m_db->userExists(admin->getUserName()))
		return false;

	if (!m_db->insertPerson(admin)){
		if (admin) delete admin;
		return false;
	}
	delete admin;
	return true;
}

bool Session::updateAdmin(Admin *admin) {
	if (!isAuthorized(Session::ADMIN_UPDATE))
		return false;

	if (!admin || (typeid(*admin) != typeid(Admin)))
		return false;

	if (!m_db->userExists(admin->getUserName()))
		return false;

	if (!m_db->insertPerson(admin)){
		if (admin) delete admin;
		return false;
	}
	delete admin;
	return true;
}

bool Session::deleteAdmin(Admin *admin) {
	if (!isAuthorized(Session::ADMIN_DELETE))
		return false;

	if (!admin || (typeid(*admin) != typeid(Admin)))
		return false;

	if (!m_db->userExists(admin->getUserName()))
		return false;

	if (!m_db->deletePerson(admin)){
		if (admin) delete admin;
		return false;
	}
	delete admin;
	return true;
}

bool Session::deactivateAdmin(Admin *admin) {
	if (!isAuthorized(Session::ADMIN_DEACTIVATE))
		return false;

	if (!admin || (typeid(*admin) != typeid(Admin)))
		return false;

	if (!m_db->userExists(admin->getUserName()))
		return false;

	admin->lock();

	if (!m_db->insertPerson(admin)){
		if (admin) delete admin;
		return false;
	}
	delete admin;
	return true;
}

bool Session::activateAdmin(Admin *admin) {
	if (!isAuthorized(Session::ADMIN_ACTIVATE))
		return false;

	if (!admin || (typeid(*admin) != typeid(Admin)))
		return false;

	if (!m_db->userExists(admin->getUserName()))
		return false;

	admin->unlock();

	if (!m_db->insertPerson(admin)) {
		if (admin) delete admin;
		return false;
	}
	delete admin;
	return true;
}

bool Session::printAdminInfo(Admin *admin) {
	if (!isAuthorized(Session::ADMIN_PRINT_INFO) && (admin != m_user))
		return false;

	if (!admin || (typeid(*admin) != typeid(Admin)))
		return false;

	if (!m_db->userExists(admin->getUserName()))
		return false;

	string status = admin->isLocked() ? "Yes" : "No";
	string cap_admin_create = admin->canCreateAdmin() ? "yes" : "no";
	string cap_admin_update = admin->canUpdateAdmin() ? "yes" : "no";
	string cap_admin_delete = admin->canDeleteAdmin() ? "yes" : "no";
	string cap_admin_activate = admin->canActivateAdmin() ? "yes" : "no";
	string cap_admin_deactivate = admin->canDeactivateAdmin() ? "yes" : "no";
	string cap_admin_print = admin->canPrintAdminInfo() ? "yes" : "no";
	string cap_admin_listall = admin->canListAllAdmin() ? "yes" : "no";

	struct libscols_table *tb;
	struct libscols_line *line, *username, *fullname, *id, *natid, *stat, *privs, *role;
	setlocale(LC_ALL, "");

	tb = scols_new_table();
	scols_table_new_column(tb, "", 0.1, SCOLS_FL_TREE);

	line = username = scols_table_new_line(tb, NULL);
	scols_line_set_data(line, 0, string(admin->getUserName()).c_str());
	line = fullname = scols_table_new_line(tb, username);
	scols_line_set_data(line, 0, string("Name: "
				+ string(admin->getFirstName()
				+ " " + admin->getLastName())).c_str());
	line = id = scols_table_new_line(tb, username);
	scols_line_set_data(line, 0, string("ID: "
				+ to_string(admin->getId())).c_str());
	line = natid = scols_table_new_line(tb, username);
	scols_line_set_data(line, 0, string("National ID: "
				+ admin->getNationalId()).c_str());
	line = stat = scols_table_new_line(tb, username);
	scols_line_set_data(line, 0, string("Locked: " + status).c_str());
	line = privs =scols_table_new_line(tb, username);
	scols_line_set_data(line, 0, "Privileges");
	line = role = scols_table_new_line(tb, privs);
	scols_line_set_data(line, 0, "Admin");
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("create: " + cap_admin_create).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("update: " + cap_admin_update).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("delete: " + cap_admin_delete).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("activate: " + cap_admin_activate).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("deactivate: " + cap_admin_deactivate).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("print info: " + cap_admin_print).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("list all: " + cap_admin_listall).c_str());

	scols_print_table(tb);
	scols_unref_table(tb);
	cout << endl;

	delete admin;
	return true;
}

bool Session::printAdminInfo() {
	Admin *admin = dynamic_cast<Admin*>(m_user);
	if (!bIsLoggedIn || !admin)
		return false;
	return printAdminInfo(admin);
}

bool Session::ListAllAdmins() {
	if (!isAuthorized(Session::ADMIN_LIST_ALL))
		return false;

	vector<Admin*> admins = m_db->getAllAdmins();

	if (!admins.size()) {
		return false;
	}

	for (Admin *admin : admins)
		printAdminInfo(admin);

	admins.clear();
	return true;
}

bool Session::createEmployee(Employee *emp) {
	if (!isAuthorized(Session::EMPLOYEE_CREATE))
		return false;

	if (!emp || typeid(*emp) != typeid(Employee))
		return false;

	if (m_db->userExists(emp->getUserName()))
		return false;

	if (!m_db->insertPerson(emp)) {
		if (emp) delete emp;
		return false;
	}
	delete emp;
	return true;
}

bool Session::updateEmployee(Employee *emp) {
	if (!isAuthorized(Session::EMPLOYEE_UPDATE))
		return false;

	if (!emp || typeid(*emp) != typeid(Employee))
		return false;

	if (!m_db->userExists(emp->getUserName()))
		return false;

	if (!m_db->insertPerson(emp)) {
		if (emp) delete emp;
		return false;
	}
	delete emp;
	return true;
}

bool Session::deleteEmployee(Employee *emp) {
	if (!isAuthorized(Session::EMPLOYEE_DELETE))
		return false;

	if (!emp || typeid(*emp) != typeid(Employee))
		return false;

	if (!m_db->userExists(emp->getUserName()))
		return false;

	if (!m_db->deletePerson(emp)){
		if (emp) delete emp;
		return false;
	}
	delete emp;
	return true;
}

bool Session::activateEmployee(Employee *emp) {
	if (!isAuthorized(Session::EMPLOYEE_ACTIVATE))
		return false;

	if (!emp || typeid(*emp) != typeid(Employee))
		return false;

	if (!m_db->userExists(emp->getUserName()))
		return false;

	emp->unlock();

	if (!m_db->insertPerson(emp)){
		if (emp) delete emp;
		return false;
	}
	delete emp;
	return true;
}

bool Session::deactivateEmployee(Employee *emp) {
	if (!isAuthorized(Session::EMPLOYEE_DEACTIVATE))
		return false;

	if (!emp || typeid(*emp) != typeid(Employee))
		return false;

	if (!m_db->userExists(emp->getUserName()))
		return false;

	emp->lock();

	if (!m_db->insertPerson(emp)){
		if (emp) delete emp;
		return false;
	}
	delete emp;
	return true;
}

bool Session::printEmployeeInfo(Employee *emp) {
	if (!isAuthorized(Session::EMPLOYEE_PRINT_INFO) && (emp != m_user))
		return false;

	if (!emp || (typeid(*emp) != typeid(Employee)))
		return false;

	if (!m_db->userExists(emp->getUserName()))
		return false;

	string status = emp->isLocked() ? "Yes" : "No";
	string cap_cust_create = emp->canCreateCustomer() ? "yes" : "no";
	string cap_cust_update = emp->canUpdateCustomer() ? "yes" : "no";
	string cap_cust_delete = emp->canDeleteCustomer() ? "yes" : "no";
	string cap_cust_activate = emp->canActivateCustomer() ? "yes" : "no";
	string cap_cust_deactivate = emp->canDeactivateCustomer() ? "yes" : "no";
	string cap_cust_print = emp->canPrintCustomerInfo() ? "yes" : "no";
	string cap_cust_listall = emp->canListAllCustomers() ? "yes" : "no";

	struct libscols_table *tb;
	struct libscols_line *line, *username, *fullname, *id, *natid, *stat, *privs, *role;
	setlocale(LC_ALL, "");

	tb = scols_new_table();
	scols_table_new_column(tb, "", 0.1, SCOLS_FL_TREE);

	line = username = scols_table_new_line(tb, NULL);
	scols_line_set_data(line, 0, string(emp->getUserName()).c_str());
	line = fullname = scols_table_new_line(tb, username);
	scols_line_set_data(line, 0, string("Name: "
				+ string(emp->getFirstName() + " " + emp->getLastName())).c_str());
	line = id = scols_table_new_line(tb, username);
	scols_line_set_data(line, 0, string("ID: "
				+ to_string(emp->getId())).c_str());
	line = natid = scols_table_new_line(tb, username);
	scols_line_set_data(line, 0, string("National ID: "
				+ emp->getNationalId()).c_str());
	line = stat = scols_table_new_line(tb, username);
	scols_line_set_data(line, 0, string("Locked: " + status).c_str());
	line = privs =scols_table_new_line(tb, username);
	scols_line_set_data(line, 0, "Privileges");
	line = role = scols_table_new_line(tb, privs);
	scols_line_set_data(line, 0, "Customer");
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("create: " + cap_cust_create).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("update: " + cap_cust_update).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("delete: " + cap_cust_delete).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("activate: " + cap_cust_activate).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("deactivate: " + cap_cust_deactivate).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("print info: " + cap_cust_print).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("list all: " + cap_cust_listall).c_str());

	string cap_acct_create = emp->canCreateAccount() ? "yes" : "no";
	string cap_acct_update = emp->canUpdateAccount() ? "yes" : "no";
	string cap_acct_delete = emp->canDeleteAccount() ? "yes" : "no";
	string cap_acct_activate = emp->canActivateAccount() ? "yes" : "no";
	string cap_acct_deactivate = emp->canDeactivateAccount() ? "yes" : "no";
	string cap_acct_print = emp->canPrintAccountInfo() ? "yes" : "no";
	string cap_acct_listall = emp->canListAllAccounts() ? "yes" : "no";

	line = role = scols_table_new_line(tb, privs);
	scols_line_set_data(line, 0, "Account");
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("create: " + cap_acct_create).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("update: " + cap_acct_update).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("delete: " + cap_acct_delete).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("activate: " + cap_acct_activate).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("deactivate: " + cap_acct_deactivate).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("print info: " + cap_acct_print).c_str());
	line = scols_table_new_line(tb, role);
	scols_line_set_data(line, 0, string("list all: " + cap_acct_listall).c_str());

	scols_print_table(tb);
	scols_unref_table(tb);
	cout << endl;

	delete emp;
	return true;
}

bool Session::ListAllEmployees() {
	if (!isAuthorized(Session::EMPLOYEE_LIST_ALL))
		return false;

	vector<Employee*> employees = m_db->getAllEmployees();
	if (!employees.size())
		return false;

	for (Employee *emp : employees)
		printEmployeeInfo(emp);

	employees.clear();
	return true;
}


/*
 * User interface methods
 * */

void Ui::ui_create_admin() {
	string username;
	string firstname;
	string lastname;
	string nationalid;
	string password;
	string password_confirm;

	cout << "Registering an administrator" << endl;
	cout << endl;

	Admin *tmp = new Admin();
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

	string answer = "";

	do {
		cout << "Can create other Administrators? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_AdminCreate(true);
	else tmp->cap_AdminCreate(false);
	answer.clear();

	do {
		cout << "Can Update other Administrators? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_AdminUpdate(true);
	else tmp->cap_AdminUpdate(false);
	answer.clear();

	do {
		cout << "Can Delete other Administrators? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_AdminDelete(true);
	else tmp->cap_AdminDelete(false);
	answer.clear();

	do {
		cout << "Can Activate other Administrators? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_AdminActivate(true);
	else tmp->cap_AdminActivate(false);
	answer.clear();

	do {
		cout << "Can Deactivate other Administrators? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_AdminDeactivate(true);
	else tmp->cap_AdminDeactivate(false);
	answer.clear();

	do {
		cout << "Can list all other Administrators? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_AdminListAll(true);
	else tmp->cap_AdminListAll(false);
	answer.clear();

	do {
		cout << "Can print Administrators info? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_AdminPrintInfo(true);
	else tmp->cap_AdminPrintInfo(false);
	answer.clear();

	if (!m_session->createAdmin(tmp)) {
		cerr << "Error creating the administrator" << endl;
		return;
	} else
		cout << "Administrator was created successfully" << endl;
}

void Ui::ui_update_admin() {
	string username;
	string firstname;
	string lastname;
	string nationalid;
	string answer = "";
	Admin *tmp;

	cout << "Updating an administrator" << endl;
	cout << endl;

	do {
		cout << "Admin user name: ";
		cin >> username;
		tmp = m_session->getAdmin(username);
		if (!tmp)
			cerr << "Admin account doesn't exit" << endl;
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

	do {
		cout << "Can create other Administrators? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_AdminCreate(true);
	else tmp->cap_AdminCreate(false);
	answer.clear();

	do {
		cout << "Can Update other Administrators? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_AdminUpdate(true);
	else tmp->cap_AdminUpdate(false);
	answer.clear();

	do {
		cout << "Can Delete other Administrators? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_AdminDelete(true);
	else tmp->cap_AdminDelete(false);
	answer.clear();

	do {
		cout << "Can Activate other Administrators? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_AdminActivate(true);
	else tmp->cap_AdminActivate(false);
	answer.clear();

	do {
		cout << "Can Deactivate other Administrators? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_AdminDeactivate(true);
	else tmp->cap_AdminDeactivate(false);
	answer.clear();

	do {
		cout << "Can list all other Administrators? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_AdminListAll(true);
	else tmp->cap_AdminListAll(false);
	answer.clear();

	do {
		cout << "Can print Administrators info? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_AdminPrintInfo(true);
	else tmp->cap_AdminPrintInfo(false);
	answer.clear();

	if (!m_session->updateAdmin(tmp)) {
		cerr << "Error updating " << username << endl;
		return;
	} else
		cout << "Administrator was updated successfully" << endl;
}

void Ui::ui_delete_admin() {
	string username = "";
	Admin *tmp;
	cout << "Enter administrator's user name to delete: ";
	cin >> username;
	tmp = m_session->getAdmin(username);
	if (tmp) {
		if (!m_session->deleteAdmin(tmp)) {
			cerr << "Failed to delete admin acount: " << username << endl;
			return;
		} else
			cout << "Deleted admin account: " << username << endl;
	} else
		cout << "Failed to query the deleting desired Admin: " << username
			<< endl;
}

void Ui::ui_activate_admin() {
	string username;
	Admin *tmp;

	do {
		cout << "Admin username: ";
		cin >> username;
		tmp = m_session->getAdmin(username);
		if (!tmp)
			cerr << "Admin account doesn't exit" << endl;

	} while (!tmp);

	if (!m_session->activateAdmin(tmp)) {
		cerr << "Error activating " << username << endl;
		return;
	} else
		cout << "Administrator was activated successfully" << endl;
}

void Ui::ui_deactivate_admin() {
	string username;
	Admin *tmp;

	do {
		cout << "Admin username: ";
		cin >> username;
		tmp = m_session->getAdmin(username);
		if (!tmp)
			cerr << "Admin account doesn't exit" << endl;
	} while (!tmp);

	if (!m_session->deactivateAdmin(tmp)) {
		cerr << "Error deactivating " << username << endl;
		return;
	} else
		cout << "Administrator was deactivated successfully" << endl;
}

void Ui::ui_print_admin() {
	string username;
	cout << "Admin username: ";
	cin >> username;
	Admin *admin = m_session->getAdmin(username);
	if (!admin) {
		cerr << "No such Admin" << endl;
		return;
	}

	if (!m_session->printAdminInfo(admin)) {
		cerr << "Error printing admin info" << endl;
		return;
	}
}

void Ui::ui_print_own_admin(){
	if (!m_session->printAdminInfo())
		cerr << "Error printing my admin info" << endl;
}

void Ui::ui_listall_admin() {
	m_session->ListAllAdmins();
}

void Ui::ui_create_employee() {
	string username;
	string firstname;
	string lastname;
	string nationalid;
	string password;
	string password_confirm;

	cout << "Registering an Employee" << endl;
	cout << endl;

	Employee *tmp = new Employee();
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

	string answer = "";

	do {
		cout << "Can create customer? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_custCreate(true);
	else tmp->cap_custCreate(false);
	answer.clear();

	do {
		cout << "Can Update customers? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_custUpdate(true);
	else tmp->cap_custUpdate(false);
	answer.clear();

	do {
		cout << "Can Delete customers? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_custDelete(true);
	else tmp->cap_custDelete(false);
	answer.clear();

	do {
		cout << "Can Activate customer? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_custActivate(true);
	else tmp->cap_custActivate(false);
	answer.clear();

	do {
		cout << "Can Deactivate customer? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_custDeactivate(true);
	else tmp->cap_custDeactivate(false);
	answer.clear();

	do {
		cout << "Can list all customers? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_custListAll(true);
	else tmp->cap_custListAll(false);
	answer.clear();

	do {
		cout << "Can print customer info? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_custPrintInfo(true);
	else tmp->cap_custPrintInfo(false);
	answer.clear();

	do {
		cout << "Can create account? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_acctCreate(true);
	else tmp->cap_acctCreate(false);
	answer.clear();

	do {
		cout << "Can update account? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_acctUpdate(true);
	else tmp->cap_acctUpdate(false);
	answer.clear();

	do {
		cout << "Can delete account? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_acctDelete(true);
	else tmp->cap_acctDelete(false);
	answer.clear();

	do {
		cout << "Can activate account? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_acctActivate(true);
	else tmp->cap_acctActivate(false);
	answer.clear();

	do {
		cout << "Can Deactivate account? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_acctDeactivate(true);
	else tmp->cap_acctDeactivate(false);
	answer.clear();

	do {
		cout << "Can list all accounts? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_acctListAll(true);
	else tmp->cap_acctListAll(false);
	answer.clear();

	do {
		cout << "Can print account info? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_acctPrintInfo(true);
	else tmp->cap_acctPrintInfo(false);
	answer.clear();

	if (!m_session->createEmployee(tmp)) {
		cerr << "Error creating the Employee" << endl;
		return;
	} else
		cout << "Employee was created successfully" << endl;
}

void Ui::ui_update_employee() {

	string username;
	string firstname;
	string lastname;
	string nationalid;
	string answer = "";
	Employee *tmp;

	cout << "Updating an Employee" << endl;
	cout << endl;

	do {
		cout << "Employee user name: ";
		cin >> username;
		tmp = m_session->getEmployee(username);
		if (!tmp)
			cerr << "Employee account doesn't exit" << endl;
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

	do {
		cout << "Can create customer? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_custCreate(true);
	else tmp->cap_custCreate(false);
	answer.clear();

	do {
		cout << "Can Update customers? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_custUpdate(true);
	else tmp->cap_custUpdate(false);
	answer.clear();

	do {
		cout << "Can Delete customers? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_custDelete(true);
	else tmp->cap_custDelete(false);
	answer.clear();

	do {
		cout << "Can Activate customer? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_custActivate(true);
	else tmp->cap_custActivate(false);
	answer.clear();

	do {
		cout << "Can Deactivate customer? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_custDeactivate(true);
	else tmp->cap_custDeactivate(false);
	answer.clear();

	do {
		cout << "Can list all customers? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_custListAll(true);
	else tmp->cap_custListAll(false);
	answer.clear();

	do {
		cout << "Can print customer info? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_custPrintInfo(true);
	else tmp->cap_custPrintInfo(false);
	answer.clear();

	do {
		cout << "Can create account? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_acctCreate(true);
	else tmp->cap_acctCreate(false);
	answer.clear();

	do {
		cout << "Can update account? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_acctUpdate(true);
	else tmp->cap_acctUpdate(false);
	answer.clear();

	do {
		cout << "Can delete account? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_acctDelete(true);
	else tmp->cap_acctDelete(false);
	answer.clear();

	do {
		cout << "Can activate account? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_acctActivate(true);
	else tmp->cap_acctActivate(false);
	answer.clear();

	do {
		cout << "Can Deactivate account? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_acctDeactivate(true);
	else tmp->cap_acctDeactivate(false);
	answer.clear();

	do {
		cout << "Can list all accounts? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_acctListAll(true);
	else tmp->cap_acctListAll(false);
	answer.clear();

	do {
		cout << "Can print account info? (y/N): ";
		cin >> answer;
	} while (!cin.fail()
			&& (answer != "y" && answer != "n" 
			&& answer != "Y" && answer != "N"));

	if (answer == "y" || answer == "Y")
		tmp->cap_acctPrintInfo(true);
	else tmp->cap_acctPrintInfo(false);
	answer.clear();

	if (!m_session->updateEmployee(tmp)) {
		cerr << "Error updating " << username << endl;
		return;
	} else
		cout << "Employee was updated successfully" << endl;
}

void Ui::ui_delete_employee() {
	string username = "";
	Employee *tmp;
	cout << "Enter Emplyee's user name to delete: ";
	cin >> username;
	tmp = m_session->getEmployee(username);
	if (tmp) {
		if (!m_session->deleteEmployee(tmp)) {
			cerr << "Failed to delete employee acount: " << username << endl;
			return;
		}
		else
			cout << "Deleted employee account: " << username << endl;
	} else
		cout << "Failed to query the deleting desired Employee: " << username
			<< endl;
}

void Ui::ui_activate_employee() {
	string username;
	Employee *tmp;

	do {
		cout << "Employee username: ";
		cin >> username;
		tmp = m_session->getEmployee(username);
		if (!tmp)
			cerr << "Employee account doesn't exit" << endl;
	} while (!tmp);

	if (!m_session->activateEmployee(tmp)) {
		cerr << "Error activating " << username << endl;
	} else
		cout << "Employee was activated successfully" << endl;
}

void Ui::ui_deactivate_employee() {
	string username;
	Employee *tmp;

	do {
		cout << "Employee username: ";
		cin >> username;
		tmp = m_session->getEmployee(username);
		if (!tmp)
			cerr << "Employee account doesn't exit" << endl;
	} while (!tmp);

	if (!m_session->deactivateEmployee(tmp)) {
		cerr << "Error activating " << username << endl;
	} else
		cout << "Employee was activated successfully" << endl;
}

void Ui::ui_print_employee() {
	string username;
	cout << "Employee username: ";
	cin >> username;
	Employee *emp = m_session->getEmployee(username);
	if (!emp) {
		cerr << "No such Employee" << endl;
		return;
	}

	if (!m_session->printEmployeeInfo(emp))
		cerr << "Error printing employee info" << endl;
}

void Ui::ui_listall_employee() {
	m_session->ListAllEmployees();
}
