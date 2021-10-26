/*
 * session.h
 *
 *  Created on: Nov 4, 2020
 *      Author: Amira
 */

#ifndef SRC_SESSION_H_
#define SRC_SESSION_H_

#include <iostream>
#include <string>
#include <vector>
#include <typeinfo>
#include <sstream>
#ifdef __linux__
#include "openssl/sha.h"
#endif
#include "customer.h"
#include "employee.h"
#include "admin.h"
#include "person.h"
#include "database.h"

using namespace std;

class Session {

private:
	Person *m_user;
	Database *m_db;
	bool bIsLoggedIn;
	int m_userType;
	int m_totalUsers;
	vector<string> m_sessionCapabilities;
	void setUserType();
	void setSessionCapabilities();
	bool isAuthorized(int priv);
	void setSessionUser(Person *p) {
		m_user = p;
		setSessionCapabilities();
		this->bIsLoggedIn = true;
	}

public:
	Session();
	~Session();

	enum {
		UNDEFINED = 0,
		ACCOUNT_CREATE = (1 << 0),
		ACCOUNT_DELETE = (1 << 1),
		ACCOUNT_UPDATE = (1 << 2),
		ACCOUNT_ACTIVATE = (1 << 3),
		ACCOUNT_DEACTIVATE = (1 << 4),
		ACCOUNT_LIST_ALL = (1 << 5),
		ACCOUNT_PRINT_INFO = (1 << 6),
		ACCOUNT_PRINT_OWN_INFO = (1 << 7),
		CUSTOMER_CREATE = (1 << 8),
		CUSTOMER_UPDATE = (1 << 9),
		CUSTOMER_DELETE = (1 << 10),
		CUSTOMER_ACTIVATE = (1 << 11),
		CUSTOMER_DEACTIVATE = (1 << 12),
		CUSTOMER_PRINT_INFO = (1 << 13),
		CUSTOMER_PRINT_OWN_INFO = (1 << 14),
		CUSTOMER_LIST_ALL = (1 << 15),
		CUSTOMER_TRANSFER_TO_ACCOUNT = (1 << 16),
		EMPLOYEE_CREATE = (1 << 17),
		EMPLOYEE_UPDATE = (1 << 18),
		EMPLOYEE_DELETE = (1 << 19),
		EMPLOYEE_PRINT_INFO = (1 << 20),
		EMPLOYEE_LIST_ALL = (1 << 21),
		EMPLOYEE_ACTIVATE = (1 << 22),
		EMPLOYEE_DEACTIVATE = (1 << 23),
		ADMIN_CREATE = (1 << 24),
		ADMIN_UPDATE = (1 << 25),
		ADMIN_DELETE = (1 << 26),
		ADMIN_PRINT_INFO = (1 << 27),
		ADMIN_LIST_ALL = (1 << 28),
		ADMIN_ACTIVATE = (1 << 29),
		ADMIN_DEACTIVATE = (1 << 30)
	};

	enum {
		UNKNOWN, ADMIN, EMPLOYEE, CUSTOMER
	};

	bool firstRun() const;
	bool isLoggedIn() const {
		return bIsLoggedIn;
	}
	bool login(const string &username, const string &password);
	void logout();
	bool changePassword(Person *p, const string &newpassword);
	bool changePassword(const string &newpassword);
	bool verifyPassword(const string &password);
	static string encrypt(const string &word) ;

	vector<string> getSessionCapabilities() {
		return m_sessionCapabilities;
	}

	Person* getPerson(const string &username);
	Customer* getCustomer(const string &username);
	Customer* getCustomerByAccount(int accountid);
	Employee* getEmployee(const string &username);
	Admin* getAdmin(const string &username);
	Account* getAccount(int accountid);
	Account* getAccountByCustomer(int customerid);
	string getSessionUser();
	string getSessionUserType();
	int genUserId();
	int genAccountId();

	// Customer methods
	bool withdraw(float sum);
	bool transfer(int to, float sum);
	bool printCustomerInfo();
	bool printAccountInfo();
	bool deposit(float sum);

	// Employee methods
	bool createAccount(Account *acct);
	bool deleteAccount(Account *acct);
	bool updateAccount(Account *acct);
	bool deactivateAccount(Account *acct);
	bool activateAccount(Account *acct);
	bool createCustomer(Customer *customer);
	bool updateCustomer(Customer *customer);
	bool deleteCustomer(Customer *customer);
	bool activateCustomer(Customer *customer);
	bool deactivateCustomer(Customer *customer);
	bool transfer(Account *from, Account *to, float sum);
	bool deposit(Account *acct, float sum);
	bool ListAllCustomers();
	bool printCustomerInfo(Customer *cust);
	bool printAccountInfo(Account *acct);
	bool printEmployeeInfo();

	// Admin methods
	bool createAdmin(Admin *admin);
	bool updateAdmin(Admin *admin);
	bool deleteAdmin(Admin *admin);
	bool activateAdmin(Admin *admin);
	bool deactivateAdmin(Admin *admin);
	bool printAdminInfo(Admin *admin);
	bool printAdminInfo();
	bool createEmployee(Employee *emp);
	bool updateEmployee(Employee *emp);
	bool deleteEmployee(Employee *emp);
	bool activateEmployee(Employee *emp);
	bool deactivateEmployee(Employee *emp);
	bool printEmployeeInfo(Employee *emp);
	bool ListAllAdmins();
	bool ListAllEmployees();
	bool ListAllAccounts();
};

#endif /* SRC_SESSION_H_ */
