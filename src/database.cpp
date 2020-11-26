/*
 * database.cpp
 *
 *  Created on: Nov 6, 2020
 *      Author: Amira
 */

#include <iostream>
#include <string>
#include <string.h>
#include "database.h"
#include "session.h"
#include <assert.h>

#define USERID 0
#define USERNAME 1
#define FIRSTNAME 2
#define LASTNAME 3
#define NATIONALID 4
#define PASSWORD 5
#define USERTYPE 6
#define USERLOCK 7
#define USERCAPS 8

using namespace std;

Database::Database() :
		db(nullptr) {
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
	rc = sqlite3_open_v2(DBNAME, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

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
			"ID				INT			PRIMARY KEY NOT NULL,"
			"LOCKED			BOOLEAN		NOT NULL,"
			"OWNER			INT			NOT NULL UNIQUE,"
			"BALANCE		INT			NOT NULL,"
			"FOREIGN KEY(OWNER)	REFERENCES	PERSONS(ID));";

	rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &zErrMsg);

	if (rc != SQLITE_OK) {
		cerr << "SQL error: " << zErrMsg << endl;
		sqlite3_free(zErrMsg);
		return false;
	} else {
		sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);
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

	rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &zErrMsg);

	if (rc != SQLITE_OK) {
		cerr << "SQL error: " << zErrMsg << endl;
		sqlite3_free(zErrMsg);
		return false;
	} else {
		sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);
		return true;
	}
	return false;
}

bool Database::insertAccount(Account *acct) {
	const char *zErrMsg;
	int rc;
	sqlite3_stmt *stmt;

	string sql = "INSERT OR REPLACE INTO ACCOUNTS VALUES (?,?,?,?);";

	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare insert statment " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, 1, acct->getId());
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, 2, acct->isLocked());
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, 3, acct->getCustomerId());
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, 4, acct->getBalance());
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
	rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	assert(rc == SQLITE_OK);

	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare delete statment " << sql.c_str() << " " << rc
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
	int balance = 0;

	string sql = "SELECT * from ACCOUNTS WHERE ID = ?";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statment " << sql.c_str() << " " << rc
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
		accountid = sqlite3_column_int(stmt, 0);
		lockstatus = sqlite3_column_int(stmt, 1);
		custid = sqlite3_column_int(stmt, 2);
		balance = sqlite3_column_int(stmt, 3);
	}

	if (accountid > 0) {
		Account *acct = new Account();
		acct->setId(accountid);
		acct->setBalance(balance);
		acct->setCustomerId(custid);
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
	int lockstatus = 0;
	int custid = 0;
	int balance = 0;

	string sql = "SELECT * FROM ACCOUNTS WHERE OWNER = ?";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statment " << sql.c_str() << " " << rc
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
		lockstatus = sqlite3_column_int(stmt, 1);
		custid = sqlite3_column_int(stmt, 2);
		balance = sqlite3_column_int(stmt, 3);
	}

	Account *acct = new Account();
	acct->setId(accountid);
	acct->setBalance(balance);
	acct->setCustomerId(custid);
	lockstatus ? acct->lock() : acct->unlock();

	sqlite3_finalize(stmt);
	return acct;
}

bool Database::insertPerson(Person *p) {
	const char *zErrMsg;
	int rc;
	sqlite3_stmt *stmt;

	string sql = "INSERT OR REPLACE INTO PERSONS VALUES (?,?,?,?,?,?,?,?,?);";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare insert statment " << sql.c_str() << " " << rc
				<< " " << sqlite3_errmsg(db) << endl;
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, 1, p->getId());
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_text(stmt, 2, p->getUserName().c_str(),
			p->getUserName().length(), SQLITE_TRANSIENT);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_text(stmt, 3, p->getFirstName().c_str(),
			p->getFirstName().length(), SQLITE_TRANSIENT);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_text(stmt, 4, p->getLastName().c_str(),
			p->getLastName().length(), SQLITE_TRANSIENT);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_text(stmt, 5, p->getNationalId().c_str(),
			p->getNationalId().length(), SQLITE_TRANSIENT);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_text(stmt, 6, p->getPassword().c_str(),
			p->getPassword().length(), SQLITE_TRANSIENT);
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, 7, setUserType(p));
	if (SQLITE_OK != rc) {
		fprintf(stderr, "Error binding value in insert (%i): %s\n", rc,
				sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, 8, p->isLocked());
	if (SQLITE_OK != rc) {
		cerr << "Error binding value in insert " << rc << " "
				<< sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return false;
	}

	rc = sqlite3_bind_int64(stmt, 9, computeUserCaps(p));
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
	rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	assert(rc == SQLITE_OK);

	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare delete statment " << sql.c_str() << " " << rc
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
	string uname = "";
	string fname = "";
	string lname = "";
	string natid = "";
	string password = "";
	int usertype = 0;
	int lockstatus = 0;
	int caps = 0;

	string sql = "SELECT * from PERSONS WHERE USERNAME = ?";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statment " << sql.c_str() << " " << rc
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
		userid = sqlite3_column_int(stmt, USERID);
		uname = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,USERNAME)));
		fname = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,FIRSTNAME)));
		lname = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,LASTNAME)));
		natid = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,NATIONALID)));
		password = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,PASSWORD)));
		usertype = sqlite3_column_int(stmt, USERTYPE);
		lockstatus = sqlite3_column_int(stmt, USERLOCK);
		caps = sqlite3_column_int(stmt, USERCAPS);
	}

	if (usertype == Session::CUSTOMER) {
		Customer *person = new Customer();
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

	else if (usertype == Session::EMPLOYEE) {
		Employee *person = new Employee();
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

	else if (usertype == Session::ADMIN) {
		Admin *person = new Admin();
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
		Employee *emp = dynamic_cast<Employee*>(p);
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
		Admin *adm = dynamic_cast<Admin*>(p);
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

	for (vector<int>::iterator it = allcaps.begin(); it != allcaps.end(); ++it)
		usercaps |= *it;
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
		cerr << "Can't prepare select statment " << sql.c_str() << " " << rc
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
		cerr << "Can't prepare select statment " << sql.c_str() << " " << rc
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

vector<Person*> Database::getAllPersons(const int person_type) {
	const char *zErrMsg = nullptr;
	sqlite3_stmt *stmt = nullptr;
	int rc;
	int userid;
	string uname;
	string fname;
	string lname;
	string natid;
	string password;
	int usertype;
	int lockstatus;
	int caps;
	vector<Person*> list;

	string sql = "SELECT * from PERSONS WHERE TYPE = ?";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statment " << sql.c_str() << " " << rc
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
			userid = sqlite3_column_int(stmt, USERID);
			uname = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,USERNAME)));
			fname = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,FIRSTNAME)));
			lname = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,LASTNAME)));
			natid = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,NATIONALID)));
			password = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,PASSWORD)));
			usertype = sqlite3_column_int(stmt, USERTYPE);
			lockstatus = sqlite3_column_int(stmt, USERLOCK);
			caps = sqlite3_column_int(stmt, USERCAPS);

			if (usertype == person_type) { // Extra check
				Person *person = new Person();
				person->setId(userid);
				person->setUserName(uname);
				person->setFirstName(fname);
				person->setLastName(lname);
				person->setNationalId(natid);
				person->setPassword(password);
				person->setUserType(usertype);
				lockstatus ? person->lock() : person->unlock();
				person->setCaps(caps);
				list.push_back(person);
			}
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

bool Database::userExists(const string &username) const {
	const char *zErrMsg = nullptr;
	int rc;
	sqlite3_stmt *stmt = nullptr;

	string sql = "SELECT * from PERSONS WHERE USERNAME = ?";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statment " << sql.c_str() << " " << rc
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
		cerr << "Can't prepare select statment " << sql.c_str() << " " << rc
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
	int lockstatus;
	int custid;
	int balance;
	vector<Account*> list;

	string sql = "SELECT * from ACCOUNTS;";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statment " << sql.c_str() << " " << rc
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
			lockstatus = sqlite3_column_int(stmt, 1);
			custid = sqlite3_column_int(stmt, 2);
			balance = sqlite3_column_int(stmt, 3);

			Account *acct = new Account();
			acct->setId(accountid);
			acct->setBalance(balance);
			acct->setCustomerId(custid);
			lockstatus ? acct->lock() : acct->unlock();

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

int Database::getUsersCount() {
	const char *zErrMsg = nullptr;
	sqlite3_stmt *stmt = nullptr;
	int rc;
	int total = 0;

	string sql = "SELECT COUNT(*) from PERSONS;";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statment " << sql.c_str() << " " << rc
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

Customer* Database::retrieveCustomerByAccount(const int accountid) const {
	const char *zErrMsg = nullptr;
	sqlite3_stmt *stmt = nullptr;
	int rc;
	int userid = 0;
	string uname = "";
	string fname = "";
	string lname = "";
	string natid = "";
	string password = "";
	int usertype = 0;
	int lockstatus = 0;
	int caps = 0;

	string sql = "SELECT * from PERSONS WHERE ID = (SELECT OWNER FROM ACCOUNTS WHERE ID = ?);";
	rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, &zErrMsg);
	if (SQLITE_OK != rc) {
		cerr << "Can't prepare select statment " << sql.c_str() << " " << rc
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
		userid = sqlite3_column_int(stmt, USERID);
		uname = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,
		USERNAME)));
		fname = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,
		FIRSTNAME)));
		lname = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,
		LASTNAME)));
		natid = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,
		NATIONALID)));
		password = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,
		PASSWORD)));
		usertype = sqlite3_column_int(stmt, USERTYPE);
		lockstatus = sqlite3_column_int(stmt, USERLOCK);
		caps = sqlite3_column_int(stmt, USERCAPS);
	}

	if (usertype == Session::CUSTOMER) {
		Customer *person = new Customer();
		person->setId(userid);
		person->setUserName(uname);
		person->setFirstName(fname);
		person->setLastName(lname);
		person->setNationalId(natid);
		person->setPassword(password);
		person->setUserType(usertype);
		lockstatus ? person->lock() : person->unlock();
		person->setCaps(caps);
		Account *acc = retrieveAccount(accountid);
		if (acc)
			person->setAccount(acc);
		sqlite3_finalize(stmt);
		return person;
	}
	sqlite3_finalize(stmt);
	return nullptr;
}

vector<Admin*> Database::getAllAdmins() {
	vector<Admin*> admins;
	vector<Person*> persons = getAllPersons(Session::ADMIN);
	if (!persons.size())
		return vector<Admin*>();

	for (Person *p : persons) {
		Admin *adm = new Admin();
		int caps = p->getCaps();
		adm->setId(p->getId());
		adm->setUserName(p->getUserName());
		adm->setFirstName(p->getFirstName());
		adm->setLastName(p->getLastName());
		adm->setNationalId(p->getNationalId());
		adm->setPassword(p->getPassword());
		adm->setUserType(p->getUserType());
		adm->setCaps(p->getCaps());
		p->isLocked() ? adm->lock():adm->unlock();
		(caps & Session::ADMIN_CREATE) ? adm->cap_AdminCreate(true):adm->cap_AdminCreate(false);
		(caps & Session::ADMIN_UPDATE) ? adm->cap_AdminUpdate(true):adm->cap_AdminUpdate(false);
		(caps & Session::ADMIN_DELETE) ? adm->cap_AdminDelete(true):adm->cap_AdminDelete(false);
		(caps & Session::ADMIN_ACTIVATE) ? adm->cap_AdminActivate(true):adm->cap_AdminActivate(false);
		(caps & Session::ADMIN_DEACTIVATE) ? adm->cap_AdminDeactivate(true):adm->cap_AdminDeactivate(false);
		(caps & Session::ADMIN_PRINT_INFO) ? adm->cap_AdminPrintInfo(true):adm->cap_AdminPrintInfo(false);
		(caps & Session::ADMIN_LIST_ALL) ? adm->cap_AdminListAll(true):adm->cap_AdminListAll(false);
		admins.push_back(adm);
	}

	for (Person *p : persons) delete p;
	persons.clear();

	if (admins.size()) return admins;
	return vector<Admin*>();
}

vector<Employee*> Database::getAllEmployees() {
	vector<Employee*> employees;
	vector<Person*> persons = getAllPersons(Session::EMPLOYEE);
	if (!persons.size())
			return vector<Employee*>();

	for (Person *p : persons) {
		Employee *emp = new Employee();
		int caps = p->getCaps();
		emp->setId(p->getId());
		emp->setUserName(p->getUserName());
		emp->setFirstName(p->getFirstName());
		emp->setLastName(p->getLastName());
		emp->setNationalId(p->getNationalId());
		emp->setPassword(p->getPassword());
		emp->setUserType(p->getUserType());
		emp->setCaps(p->getCaps());
		p->isLocked() ? emp->lock():emp->unlock();
		(caps & Session::CUSTOMER_CREATE) ? emp->cap_custCreate(true):emp->cap_custCreate(false);
		(caps & Session::CUSTOMER_UPDATE) ? emp->cap_custUpdate(true):emp->cap_custUpdate(false);
		(caps & Session::CUSTOMER_DELETE) ? emp->cap_custDelete(true):emp->cap_custDelete(false);
		(caps & Session::CUSTOMER_ACTIVATE) ? emp->cap_custActivate(true):emp->cap_custActivate(false);
		(caps & Session::CUSTOMER_DEACTIVATE) ? emp->cap_custDeactivate(true):emp->cap_custDeactivate(false);
		(caps & Session::CUSTOMER_PRINT_INFO) ? emp->cap_custPrintInfo(true):emp->cap_custPrintInfo(false);
		(caps & Session::CUSTOMER_LIST_ALL) ? emp->cap_custListAll(true):emp->cap_custListAll(false);
		(caps & Session::ACCOUNT_CREATE) ? emp->cap_acctCreate(true):emp->cap_acctCreate(false);
		(caps & Session::ACCOUNT_UPDATE) ? emp->cap_acctUpdate(true):emp->cap_acctUpdate(false);
		(caps & Session::ACCOUNT_DELETE) ? emp->cap_acctDelete(true):emp->cap_acctDelete(false);
		(caps & Session::ACCOUNT_ACTIVATE) ? emp->cap_acctActivate(true):emp->cap_acctActivate(false);
		(caps & Session::ACCOUNT_DEACTIVATE) ? emp->cap_acctDeactivate(true):emp->cap_acctDeactivate(false);
		(caps & Session::ACCOUNT_PRINT_INFO) ? emp->cap_acctPrintInfo(true):emp->cap_acctPrintInfo(false);
		(caps & Session::ACCOUNT_LIST_ALL) ? emp->cap_acctListAll(true):emp->cap_acctListAll(false);
		employees.push_back(emp);
	}
	for (Person *p : persons) delete p;
	persons.clear();

	if (employees.size()) return employees;
	return vector<Employee*>();
}

vector<Customer*> Database::getAllCustomers() {
	vector<Customer*> customers;
	vector<Person*> persons = getAllPersons(Session::CUSTOMER);
	if (!persons.size())
		return vector<Customer*>();

	for (Person *p : persons) {
		Customer *customer = new Customer();
		customer->setId(p->getId());
		customer->setUserName(p->getUserName());
		customer->setFirstName(p->getFirstName());
		customer->setLastName(p->getLastName());
		customer->setNationalId(p->getNationalId());
		customer->setPassword(p->getPassword());
		customer->setUserType(p->getUserType());
		customer->setCaps(p->getCaps());
		p->isLocked() ? customer->lock():customer->unlock();
		Account *acc = this->retrieveAccountByCustomer(customer->getId());
		if (acc) customer->setAccount(acc);
		customers.push_back(customer);
	}

	for (Person *p : persons) delete p;
	persons.clear();

	if (customers.size()) return customers;
	return vector<Customer*>();
}
