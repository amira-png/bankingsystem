/*
 * database.cpp
 *
 *  Created on: Nov 6, 2020
 *      Author: Amira
 */

#include <iostream>
#include <string>
#include "database.h"
#include "session.h"
#include <cassert>

using namespace std;

Database::Database() : db(nullptr) {
		if (initDB()) {
			if (!createPersonsTable()) {
				cerr << "Error creating the persons table" << endl;
				exit(-2);
			}
			if (!createAccountsTable()) {
				cerr << "Error creating the accounts table" << endl;
				exit(-2);
			}
		} else {
			cerr << "Error connecting to the bank database" << endl;
			exit(-1);
		}
	}

Database::~Database() {
	sqlite3_close_v2(db);
}

bool Database::initDB() {
	int rc;
	rc = sqlite3_open_v2(DBNAME, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
	if (rc) {
		cerr << "Error opening the future bank database: " << sqlite3_errmsg(db) << endl;
		return false;
	}
	rc = sqlite3_busy_timeout(db, 0);
	if (rc)
		cerr << "Error setting busy handler for the future bank database: "
			<< sqlite3_errmsg(db) << endl;
	return true;
}

bool Database::createAccountsTable() {
	char *zErrMsg = nullptr;
	int rc;

	string sql = "CREATE TABLE IF NOT EXISTS ACCOUNTS("
			"ID			INT		PRIMARY KEY NOT NULL,"
			"LOCKED			BOOLEAN		NOT NULL,"
			"OWNER			INT		NOT NULL UNIQUE,"
			"BALANCE		REAL		NOT NULL,"
			"LABEL			TEXT		NOT NULL,"
			"FOREIGN KEY(OWNER)	REFERENCES	PERSONS(ID));";

	rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &zErrMsg);

	if (rc != SQLITE_OK) {
		cerr << "SQL error: " << zErrMsg << endl;
		sqlite3_free(zErrMsg);
		return false;
	} else {
		return true;
	}
	return false;
}

bool Database::createPersonsTable() {
	char *zErrMsg = nullptr;
	int rc;

	string sql = "CREATE TABLE IF NOT EXISTS PERSONS("
			"ID               INT     NOT NULL,"
			"USERNAME         TEXT    NOT NULL UNIQUE,"
			"FIRSTNAME        TEXT    NOT NULL,"
			"LASTNAME         TEXT    NOT NULL,"
			"NATIONALID       TEXT    NOT NULL UNIQUE,"
			"PASSWORD         TEXT    NOT NULL,"
			"TYPE             INT     NOT NULL,"
			"LOCKED           BOOLEAN NOT NULL,"
			"CAPS             INT     NOT NULL,"
			"PRIMARY KEY (ID));";

	rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &zErrMsg);

	if (rc != SQLITE_OK) {
		cerr << "SQL error: " << zErrMsg << endl;
		sqlite3_free(zErrMsg);
		return false;
	} else {
		return true;
	}
	return false;
}

bool Database::insertAccount(Account *acct) {
	const char *zErrMsg;
	int rc, column=1;
	sqlite3_stmt *stmt;

	string sql = "INSERT OR REPLACE INTO ACCOUNTS VALUES (?,?,?,?,?);";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare insert statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, column++, acct->getId());
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, column++, acct->isLocked());
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, column++, acct->getCustomerId());
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_double(stmt, column++, acct->getBalance());
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_text(stmt, column++, acct->getAccountLabel().c_str(),
			acct->getAccountLabel().length(), SQLITE_TRANSIENT);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_DONE != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		return false;
	}
	sqlite3_finalize(stmt);
	return true;
}

bool Database::deleteAccount(Account *acct) {
	const char *zErrMsg;
	int rc;
	sqlite3_stmt *stmt;

	string sql = "DELETE FROM ACCOUNTS WHERE ID = ?;";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare delete statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, 1, acct->getId());
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in delete " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_DONE != rc) {
		cerr << "Error binding value in delete " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		return false;
	}
	sqlite3_finalize(stmt);
	return true;
}

Account* Database::retrieveAccount(const int account_id) const{
	const char *zErrMsg = nullptr;
	sqlite3_stmt *stmt = nullptr;
	int rc;
	int accountid = 0;
	int lockstatus = 0;
	int custid = 0;
	float balance = 0;
	string label;

	string sql = "SELECT * from ACCOUNTS WHERE ID = ?";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return nullptr;
	}

	rc = sqlite3_bind_int64(stmt, 1, account_id);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in select " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return nullptr;
	}

	int step = sqlite3_step(stmt);
	if (step == SQLITE_ROW) {
		int column = 0;
		accountid = sqlite3_column_int(stmt, column++);
		lockstatus = sqlite3_column_int(stmt, column++);
		custid = sqlite3_column_int(stmt, column++);
		balance = sqlite3_column_int(stmt, column++);
		label = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, column++)));
	}

	if (accountid > 0) {
		auto *acct = new Account();
		acct->setId(accountid);
		acct->setBalance(balance);
		acct->setCustomerId(custid);
		acct->setAccountLabel(label);
		lockstatus ? acct->lock() : acct->unlock();
		sqlite3_finalize(stmt);
		return acct;
	}
	sqlite3_finalize(stmt);
	return nullptr;
}

Account* Database::retrieveAccountByCustomer(const int customer_id) const{
	const char *zErrMsg = nullptr;
	sqlite3_stmt *stmt = nullptr;
	int rc;
	int accountid = 0;

	string sql = "SELECT ID FROM ACCOUNTS WHERE OWNER = ?";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return nullptr;
	}

	rc = sqlite3_bind_int64(stmt, 1, customer_id);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in select " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return nullptr;
	}

	int step = sqlite3_step(stmt);
	if (step == SQLITE_ROW) {
		accountid = sqlite3_column_int(stmt, 0);
	}

	Account *acct = retrieveAccount(accountid);

	sqlite3_finalize(stmt);
	return acct;
}

Customer* Database::retrieveCustomerByAccount(const int accountid) const {
	const char *zErrMsg = nullptr;
	sqlite3_stmt *stmt = nullptr;
	int rc;
	int userid = 0;
	string uname;
	string fname;
	string lname;
	string natid;
	string password;
	int usertype = 0;
	int lockstatus = 0;
	int caps = 0;

	string sql = "SELECT USERNAME from PERSONS WHERE ID = (SELECT OWNER FROM ACCOUNTS WHERE ID = ?);";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return nullptr;
	}

	rc = sqlite3_bind_int64(stmt, 1, accountid);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in select " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return nullptr;
	}

	int step = sqlite3_step(stmt);
	if (step == SQLITE_ROW) {
		userid = sqlite3_column_int(stmt, 0);
		uname = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,0)));
		auto *person = dynamic_cast<Customer*>(retrievePerson(uname));
		Account *acc = retrieveAccount(accountid);
		if (acc) person->setAccount(acc);
		sqlite3_finalize(stmt);
		return person;
	}
	sqlite3_finalize(stmt);
	return nullptr;
}

bool Database::insertPerson(Person *p) {
	const char *zErrMsg;
	int rc, column = 1;
	sqlite3_stmt *stmt;

	string sql = "INSERT OR REPLACE INTO PERSONS VALUES (?,?,?,?,?,?,?,?,?);";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare insert statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, column++, p->getId());
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_text(stmt, column++, p->getUserName().c_str(),
			p->getUserName().length(), SQLITE_TRANSIENT);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_text(stmt, column++, p->getFirstName().c_str(),
			p->getFirstName().length(), SQLITE_TRANSIENT);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_text(stmt, column++, p->getLastName().c_str(),
			p->getLastName().length(), SQLITE_TRANSIENT);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_text(stmt, column++, p->getNationalId().c_str(),
			p->getNationalId().length(), SQLITE_TRANSIENT);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_text(stmt, column++, p->getPassword().c_str(),
			p->getPassword().length(), SQLITE_TRANSIENT);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, column++, setUserType(p));
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " <<  rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, column++, p->isLocked());
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, column++, computeUserCaps(p));
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_DONE != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		return false;
	}
	sqlite3_finalize(stmt);
	return true;
}

bool Database::deletePerson(Person *p) {
	const char *zErrMsg;
	int rc;
	sqlite3_stmt *stmt;

	string sql = "DELETE FROM PERSONS WHERE USERNAME = ?;";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare delete statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_text(stmt, 1, p->getUserName().c_str(),
			p->getUserName().length(), SQLITE_TRANSIENT);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in delete " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_step(stmt);
	if (SQLITE_DONE != rc) {
		cerr << "Error binding value in delete " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		return false;
	}
	sqlite3_finalize(stmt);
	return true;
}

int Database::setUserType(Person *p) {
	if (typeid(*p) == typeid(Customer))
		return Session::CUSTOMER;
	else if (typeid(*p) == typeid(Employee))
		return Session::EMPLOYEE;
	else if (typeid(*p) == typeid(Admin))
		return Session::ADMIN;
	else return Session::UNKNOWN;
}

Person* Database::retrievePerson(const string &username) const {
	const char *zErrMsg = nullptr;
	sqlite3_stmt *stmt = nullptr;
	int rc;
	int userid = 0;
	string uname;
	string fname;
	string lname;
	string natid;
	string password;
	int usertype = 0;
	int lockstatus = 0;
	int caps = 0;

	string sql = "SELECT * from PERSONS WHERE USERNAME = ?";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return nullptr;
	}

	rc = sqlite3_bind_text(stmt, 1, username.c_str(), username.length(),
			SQLITE_TRANSIENT);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in select " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return nullptr;
	}

	int step = sqlite3_step(stmt);
	if (step == SQLITE_ROW) {
		int column = 0;
		userid = sqlite3_column_int(stmt, column++);
		uname = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,column++)));
		fname = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,column++)));
		lname = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,column++)));
		natid = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,column++)));
		password = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,column++)));
		usertype = sqlite3_column_int(stmt, column++);
		lockstatus = sqlite3_column_int(stmt, column++);
		caps = sqlite3_column_int(stmt, column++);
	}

	switch (usertype) {
	case Session::CUSTOMER: {
		auto *person = new Customer();
		person->setId(userid);
		person->setUserName(uname);
		person->setFirstName(fname);
		person->setLastName(lname);
		person->setNationalId(natid);
		person->setPassword(password);
		person->setUserType(usertype);
		lockstatus ? person->lock() : person->unlock();
		person->setCaps(caps);
		Account *acct = retrieveAccountByCustomer(person->getId());
		if (acct)
			person->setAccount(acct);
		sqlite3_finalize(stmt);
		return person;
	}

	case Session::EMPLOYEE: {
		auto *person = new Employee();
		person->setId(userid);
		person->setUserName(uname);
		person->setFirstName(fname);
		person->setLastName(lname);
		person->setNationalId(natid);
		person->setPassword(password);
		person->setUserType(usertype);
		lockstatus ? person->lock() : person->unlock();
		person->setCaps(caps);
		person->cap_acctCreate(caps & Session::ACCOUNT_CREATE);
		person->cap_acctUpdate(caps & Session::ACCOUNT_UPDATE);
		person->cap_acctDelete(caps & Session::ACCOUNT_DELETE);
		person->cap_acctActivate(caps & Session::ACCOUNT_ACTIVATE);
		person->cap_acctDeactivate(caps & Session::ACCOUNT_DEACTIVATE);
		person->cap_acctPrintInfo(caps & Session::ACCOUNT_PRINT_INFO);
		person->cap_acctListAll(caps & Session::ACCOUNT_LIST_ALL);

		person->cap_custCreate(caps & Session::CUSTOMER_CREATE);
		person->cap_custUpdate(caps & Session::CUSTOMER_UPDATE);
		person->cap_custDelete(caps & Session::CUSTOMER_DELETE);
		person->cap_custActivate(caps & Session::CUSTOMER_ACTIVATE);
		person->cap_custDeactivate(caps & Session::CUSTOMER_DEACTIVATE);
		person->cap_custPrintInfo(caps & Session::CUSTOMER_PRINT_INFO);
		person->cap_custListAll(caps & Session::CUSTOMER_LIST_ALL);
		sqlite3_finalize(stmt);
		return person;
	}

	case Session::ADMIN: {
		auto *person = new Admin();
		person->setId(userid);
		person->setUserName(uname);
		person->setFirstName(fname);
		person->setLastName(lname);
		person->setNationalId(natid);
		person->setPassword(password);
		person->setUserType(usertype);
		lockstatus ? person->lock() : person->unlock();
		person->setCaps(caps);
		person->cap_acctCreate(caps & Session::ACCOUNT_CREATE);
		person->cap_acctUpdate(caps & Session::ACCOUNT_UPDATE);
		person->cap_acctDelete(caps & Session::ACCOUNT_DELETE);
		person->cap_acctActivate(caps & Session::ACCOUNT_ACTIVATE);
		person->cap_acctDeactivate(caps & Session::ACCOUNT_DEACTIVATE);
		person->cap_acctPrintInfo(caps & Session::ACCOUNT_PRINT_INFO);
		person->cap_acctListAll(caps & Session::ACCOUNT_LIST_ALL);

		person->cap_custCreate(caps & Session::CUSTOMER_CREATE);
		person->cap_custUpdate(caps & Session::CUSTOMER_UPDATE);
		person->cap_custDelete(caps & Session::CUSTOMER_DELETE);
		person->cap_custActivate(caps & Session::CUSTOMER_ACTIVATE);
		person->cap_custDeactivate(caps & Session::CUSTOMER_DEACTIVATE);
		person->cap_custPrintInfo(caps & Session::CUSTOMER_PRINT_INFO);
		person->cap_custListAll(caps & Session::CUSTOMER_LIST_ALL);

		person->cap_EmployeeCreate(caps & Session::EMPLOYEE_CREATE);
		person->cap_EmployeeUpdate(caps & Session::EMPLOYEE_UPDATE);
		person->cap_EmployeeDelete(caps & Session::EMPLOYEE_DELETE);
		person->cap_EmployeeActivate(caps & Session::EMPLOYEE_ACTIVATE);
		person->cap_EmployeeDeactivate(caps & Session::EMPLOYEE_DEACTIVATE);
		person->cap_EmployeePrintInfo(caps & Session::EMPLOYEE_PRINT_INFO);
		person->cap_EmployeeListAll(caps & Session::EMPLOYEE_LIST_ALL);

		person->cap_AdminCreate(caps & Session::ADMIN_CREATE);
		person->cap_AdminUpdate(caps & Session::ADMIN_UPDATE);
		person->cap_AdminDelete(caps & Session::ADMIN_DELETE);
		person->cap_AdminActivate(caps & Session::ADMIN_ACTIVATE);
		person->cap_AdminDeactivate(caps & Session::ADMIN_DEACTIVATE);
		person->cap_AdminPrintInfo(caps & Session::ADMIN_PRINT_INFO);
		person->cap_AdminListAll(caps & Session::ADMIN_LIST_ALL);
		sqlite3_finalize(stmt);
		return person;
	}
	default:
		return nullptr;
	}
	sqlite3_finalize(stmt);
	return nullptr;
}

int Database::computeUserCaps(Person *p) {
	int usertype = setUserType(p);
	vector<int> allcaps;
	int usercaps = 0;

	switch (usertype) {
	case Session::CUSTOMER:
		allcaps.push_back(Session::CUSTOMER_PRINT_OWN_INFO);
		allcaps.push_back(Session::CUSTOMER_TRANSFER_TO_ACCOUNT);
		allcaps.push_back(Session::ACCOUNT_PRINT_OWN_INFO);
		break;

	case Session::EMPLOYEE: {
		auto *emp = dynamic_cast<Employee*>(p);
		if (emp->canCreateAccount())
			allcaps.push_back(Session::ACCOUNT_CREATE);
		if (emp->canUpdateAccount())
			allcaps.push_back(Session::ACCOUNT_UPDATE);
		if (emp->canDeleteAccount())
			allcaps.push_back(Session::ACCOUNT_DELETE);
		if (emp->canActivateAccount())
			allcaps.push_back(Session::ACCOUNT_ACTIVATE);
		if (emp->canDeactivateAccount())
			allcaps.push_back(Session::ACCOUNT_DEACTIVATE);
		if (emp->canListAllAccounts())
			allcaps.push_back(Session::ACCOUNT_LIST_ALL);
		if (emp->canPrintAccountInfo())
			allcaps.push_back(Session::ACCOUNT_PRINT_INFO);
		if (emp->canCreateCustomer())
			allcaps.push_back(Session::CUSTOMER_CREATE);
		if (emp->canUpdateCustomer())
			allcaps.push_back(Session::CUSTOMER_UPDATE);
		if (emp->canDeleteCustomer())
			allcaps.push_back(Session::CUSTOMER_DELETE);
		if (emp->canActivateCustomer())
			allcaps.push_back(Session::CUSTOMER_ACTIVATE);
		if (emp->canDeactivateCustomer())
			allcaps.push_back(Session::CUSTOMER_DEACTIVATE);
		if (emp->canListAllCustomers())
			allcaps.push_back(Session::CUSTOMER_LIST_ALL);
		if (emp->canPrintCustomerInfo())
			allcaps.push_back(Session::CUSTOMER_PRINT_INFO);
		break;
	}
	case Session::ADMIN: {
		auto *adm = dynamic_cast<Admin*>(p);
		if (adm->canCreateAdmin())
			allcaps.push_back(Session::ADMIN_CREATE);
		if (adm->canUpdateAdmin())
			allcaps.push_back(Session::ADMIN_UPDATE);
		if (adm->canDeleteAdmin())
			allcaps.push_back(Session::ADMIN_DELETE);
		if (adm->canActivateAdmin())
			allcaps.push_back(Session::ADMIN_ACTIVATE);
		if (adm->canDeactivateAdmin())
			allcaps.push_back(Session::ADMIN_DEACTIVATE);
		if (adm->canPrintAdminInfo())
			allcaps.push_back(Session::ADMIN_PRINT_INFO);
		if (adm->canListAllAdmin())
			allcaps.push_back(Session::ADMIN_LIST_ALL);
		if (adm->canCreateEmployee())
			allcaps.push_back(Session::EMPLOYEE_CREATE);
		if (adm->canUpdateEmployee())
			allcaps.push_back(Session::EMPLOYEE_UPDATE);
		if (adm->canDeleteEmployee())
			allcaps.push_back(Session::EMPLOYEE_DELETE);
		if (adm->canActivateEmployee())
			allcaps.push_back(Session::EMPLOYEE_ACTIVATE);
		if (adm->canDeactivateEmployee())
			allcaps.push_back(Session::EMPLOYEE_DEACTIVATE);
		if (adm->canPrintEmployeeInfo())
			allcaps.push_back(Session::EMPLOYEE_PRINT_INFO);
		if (adm->canListAllEmployee())
			allcaps.push_back(Session::EMPLOYEE_LIST_ALL);
		if (adm->canCreateAccount())
			allcaps.push_back(Session::ACCOUNT_CREATE);
		if (adm->canUpdateAccount())
			allcaps.push_back(Session::ACCOUNT_UPDATE);
		if (adm->canDeleteAccount())
			allcaps.push_back(Session::ACCOUNT_DELETE);
		if (adm->canActivateAccount())
			allcaps.push_back(Session::ACCOUNT_ACTIVATE);
		if (adm->canDeactivateAccount())
			allcaps.push_back(Session::ACCOUNT_DEACTIVATE);
		if (adm->canListAllAccounts())
			allcaps.push_back(Session::ACCOUNT_LIST_ALL);
		if (adm->canPrintAccountInfo())
			allcaps.push_back(Session::ACCOUNT_PRINT_INFO);
		if (adm->canCreateCustomer())
			allcaps.push_back(Session::CUSTOMER_CREATE);
		if (adm->canUpdateCustomer())
			allcaps.push_back(Session::CUSTOMER_UPDATE);
		if (adm->canDeleteCustomer())
			allcaps.push_back(Session::CUSTOMER_DELETE);
		if (adm->canActivateCustomer())
			allcaps.push_back(Session::CUSTOMER_ACTIVATE);
		if (adm->canDeactivateCustomer())
			allcaps.push_back(Session::CUSTOMER_DEACTIVATE);
		if (adm->canListAllCustomers())
			allcaps.push_back(Session::CUSTOMER_LIST_ALL);
		if (adm->canPrintCustomerInfo())
			allcaps.push_back(Session::CUSTOMER_PRINT_INFO);
		break;
	}
	default:
		break;
	}

	for (int & allcap : allcaps)
		usercaps |= allcap;
	return usercaps;
}

int Database::generateAccountNumber() {
	const char *zErrMsg = nullptr;
	int rc;
	sqlite3_stmt *stmt = nullptr;
	int maxid = 0;

	string sql = "SELECT MAX(ID) from ACCOUNTS;";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		cerr << "Couldn't generate a valid account number, please contact your administrator!!!"
				<< endl;
		exit(-1);
	}

	int step = sqlite3_step(stmt);
	if (step == SQLITE_ROW) {
		maxid = sqlite3_column_int(stmt, 0);
		if (maxid <= 0) {
			sqlite3_finalize(stmt);
			return 1;
		}
	}
	sqlite3_finalize(stmt);
	return maxid + 1;
}

int Database::generatePersonNumber() {
	const char *zErrMsg = nullptr;
	int rc;
	sqlite3_stmt *stmt = nullptr;
	int maxid = 0;

	string sql = "SELECT MAX(ID) from PERSONS;";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_close(db);
		cerr << "Couldn't generate a valid person number, please contact your administrator!!!"
				<< endl;
		exit(-1);
	}

	int step = sqlite3_step(stmt);
	if (step == SQLITE_ROW) {
		maxid = sqlite3_column_int(stmt, 0);
		if (maxid <= 0) {
			sqlite3_finalize(stmt);
			return 1;
		}
	}
	sqlite3_finalize(stmt);
	return maxid + 1;
}

int Database::getUsersCount() {
	const char *zErrMsg = nullptr;
	sqlite3_stmt *stmt = nullptr;
	int rc;
	int total = 0;

	string sql = "SELECT COUNT(*) from PERSONS;";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		exit(-1);
	}

	int step = sqlite3_step(stmt);
	if (step == SQLITE_ROW) {
		total = sqlite3_column_int(stmt, 0);

	} else {
		cerr << "Error retrieving persons count from the database" << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		exit(-1);
	}
	sqlite3_finalize(stmt);
	return total;
}

bool Database::userExists(const string &username) const {
	const char *zErrMsg = nullptr;
	int rc;
	sqlite3_stmt *stmt = nullptr;

	string sql = "SELECT * from PERSONS WHERE USERNAME = ?";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		exit(-1);
	}

	rc = sqlite3_bind_text(stmt, 1, username.c_str(), username.length(),
	SQLITE_TRANSIENT);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in select " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		exit(-1);
	}

	int step = sqlite3_step(stmt);
	if (step == SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return true;
	}
	else {
		sqlite3_finalize(stmt);
		return false;
	}
}

bool Database::accountExists(const int account_id) const {
	const char *zErrMsg = nullptr;
	int rc;
	sqlite3_stmt *stmt = nullptr;

	string sql = "SELECT * from ACCOUNTS WHERE ID = ?";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		exit(-1);
	}

	rc = sqlite3_bind_int64(stmt, 1, account_id);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in select " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		exit(-1);
	}

	int step = sqlite3_step(stmt);
	if (step == SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return true;
	}
	else {
		sqlite3_finalize(stmt);
		return false;
	}
}

vector<Account*> Database::getAllAccounts() {
	const char *zErrMsg = nullptr;
	sqlite3_stmt *stmt = nullptr;
	int rc;
	int accountid;
	vector<Account*> list;

	string sql = "SELECT ID from ACCOUNTS;";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		exit(-1);
	}

	for (;;) {
		int step = sqlite3_step(stmt);
		if (step == SQLITE_DONE)
			break;
		if (step == SQLITE_ROW) {
			accountid = sqlite3_column_int(stmt, 0);
			Account *acct = retrieveAccount(accountid);
			list.push_back(acct);

		} else {
			cerr << "Error retrieving accounts info from the database" << endl;
			sqlite3_finalize(stmt);
			sqlite3_close(db);
			exit(-1);
		}
	}
	sqlite3_finalize(stmt);
	return list;
}

vector<Person*> Database::getAllPersons(const int person_type) {
	const char *zErrMsg = nullptr;
	sqlite3_stmt *stmt = nullptr;
	int rc;
	string uname;
	vector<Person*> list;

	string sql = "SELECT USERNAME from PERSONS WHERE TYPE = ?";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statement " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		exit(-1);
	}

	rc = sqlite3_bind_int64(stmt, 1, person_type);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in select " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		exit(-1);
	}

	for (;;) {
		int step = sqlite3_step(stmt);
		if (step == SQLITE_DONE)
			break;
		else if (step == SQLITE_ROW) {
			uname = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,0)));
			Person *person = retrievePerson(uname);
			list.push_back(person);

		} else {
			cerr << "Error retrieving users info from the database" << endl;
			sqlite3_finalize(stmt);
			sqlite3_close(db);
			exit(-1);
		}
	}
	sqlite3_finalize(stmt);
	return list;
}

vector<Admin*> Database::getAllAdmins() {
	vector<Admin*> admins;
	vector<Person*> persons = getAllPersons(Session::ADMIN);
	if (persons.empty()) return {};
	for (Person *p : persons) {
		auto *adm = dynamic_cast<Admin*>(p);
		int caps = p->getCaps();
		if (caps & Session::ADMIN_CREATE) adm->cap_AdminCreate(true);
		if (caps & Session::ADMIN_UPDATE) adm->cap_AdminUpdate(true);
		if (caps & Session::ADMIN_DELETE) adm->cap_AdminDelete(true);
		if (caps & Session::ADMIN_ACTIVATE) adm->cap_AdminActivate(true);
		if (caps & Session::ADMIN_DEACTIVATE) adm->cap_AdminDeactivate(true);
		if (caps & Session::ADMIN_PRINT_INFO) adm->cap_AdminPrintInfo(true);
		if (caps & Session::ADMIN_LIST_ALL) adm->cap_AdminListAll(true);
		admins.push_back(adm);
	}
	persons.clear();
	if (!admins.empty()) return admins;
	return {};
}

vector<Employee*> Database::getAllEmployees() {
	vector<Employee*> employees;
	vector<Person*> persons = getAllPersons(Session::EMPLOYEE);
	if (persons.empty()) return {};
	for (Person *p : persons) {
		auto *emp = dynamic_cast<Employee*>(p);
		int caps = p->getCaps();
		if (caps & Session::CUSTOMER_CREATE)  emp->cap_custCreate(true);
		if (caps & Session::CUSTOMER_UPDATE) emp->cap_custUpdate(true);
		if (caps & Session::CUSTOMER_DELETE) emp->cap_custDelete(true);
		if (caps & Session::CUSTOMER_ACTIVATE) emp->cap_custActivate(true);
		if (caps & Session::CUSTOMER_DEACTIVATE) emp->cap_custDeactivate(true);
		if (caps & Session::CUSTOMER_PRINT_INFO) emp->cap_custPrintInfo(true);
		if (caps & Session::CUSTOMER_LIST_ALL) emp->cap_custListAll(true);
		if (caps & Session::ACCOUNT_CREATE) emp->cap_acctCreate(true);
		if (caps & Session::ACCOUNT_UPDATE) emp->cap_acctUpdate(true);
		if (caps & Session::ACCOUNT_DELETE) emp->cap_acctDelete(true);
		if (caps & Session::ACCOUNT_ACTIVATE) emp->cap_acctActivate(true);
		if (caps & Session::ACCOUNT_DEACTIVATE) emp->cap_acctDeactivate(true);
		if (caps & Session::ACCOUNT_PRINT_INFO) emp->cap_acctPrintInfo(true);
		if (caps & Session::ACCOUNT_LIST_ALL) emp->cap_acctListAll(true);
		employees.push_back(emp);
	}
	persons.clear();
	if (!employees.empty()) return employees;
	return {};
}

vector<Customer*> Database::getAllCustomers() {
	vector<Customer*> customers;
	vector<Person*> persons = getAllPersons(Session::CUSTOMER);
	if (persons.empty()) return {};
	for (Person *p : persons) {
		auto *customer = dynamic_cast<Customer*>(p);
		Account *acc = this->retrieveAccountByCustomer(customer->getId());
		if (acc) customer->setAccount(acc);
		customers.push_back(customer);
	}
	persons.clear();
	if (!customers.empty()) return customers;
	return {};
}
