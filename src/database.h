/*
 * database.h
 *
 *  Created on: Nov 6, 2020
 *      Author: Amira
 */

#ifndef SRC_DATABASE_H_
#define SRC_DATABASE_H_

#include <vector>
#include <map>
#include <sqlite3.h>
#include "account.h"
#include "person.h"
#include "customer.h"
#include "employee.h"
#include "admin.h"

using namespace std;

#define DBNAME "future_bank.db"

class Database {

private:
	sqlite3 *db;
	bool initDB();
	static int setUserType(Person *p);
	static int computeUserCaps(Person *p);
	bool createAccountsTable();
	bool createPersonsTable();
	vector<Person*> getAllPersons(int person_type);

public:
	Database();
	~Database();

	bool insertAccount(Account *acct);
	bool deleteAccount(Account *acct);
	Account* retrieveAccount(int account_id) const;
	Account* retrieveAccountByCustomer(int customer_id) const;
	Customer* retrieveCustomerByAccount(int accountid) const;

	bool insertPerson(Person *p);
	bool deletePerson(Person *p);
	Person* retrievePerson(const string &username) const;
	bool userExists(const string &username) const;
	bool accountExists(int account_id) const;

	int generateAccountNumber();
	int generatePersonNumber();
	int getUsersCount();

	vector<Admin*> getAllAdmins();
	vector<Employee*> getAllEmployees();
	vector<Customer*> getAllCustomers();
	vector<Account*> getAllAccounts();
};

#endif /* SRC_DATABASE_H_ */
