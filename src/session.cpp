/*
 * session.cpp
 *
 *  Created on: Nov 4, 2020
 *      Author: Amira
 */

#include "session.h"

Session::Session() :
		m_user(nullptr), m_db(nullptr), bIsLoggedIn(false),
		m_userType(UNKNOWN), m_totalUsers(0) {
	m_db = new Database();
	m_sessionCapabilities.clear();
	m_totalUsers = m_db->getUsersCount();
	if (m_totalUsers == 0)
		m_sessionCapabilities.emplace_back("First Run");
}

Session::~Session() {
	delete m_db;
	delete m_user;
}

string Session::encrypt(const string &word) {
#ifdef __linux__
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, word.c_str(), word.length());
	SHA256_Final(hash, &sha256);
	string result = string();
	stringstream sstream;
	for (unsigned char i : hash) {
		sstream << hex << (int) i;
		result = sstream.str();
	}
#elif _WIN32
    // TODO: Windows hashing code should go here, important to modify string called "result"
#endif
	return result;
}

bool Session::isAuthorized(int priv) {
	if (!bIsLoggedIn) return false;
	return (m_user->getCaps() & priv);
}

void Session::setUserType() {
	if (typeid(*m_user) == typeid(Customer))
		m_userType = CUSTOMER;
	else if (typeid(*m_user) == typeid(Employee))
		m_userType = EMPLOYEE;
	else if (typeid(*m_user) == typeid(Admin))
		m_userType = ADMIN;
	else m_userType = UNKNOWN;
}

bool Session::login(const string &username, const string &password) {
	m_sessionCapabilities.clear();
	string unverified_enc_pass = encrypt(password);
	Person *p = m_db->retrievePerson(username);
	if (!p) return false;

	if ((p->getPassword() == unverified_enc_pass) && !p->isLocked()) {
		switch (p->getUserType()) {
		case Session::CUSTOMER: {
			Account *acct = m_db->retrieveAccountByCustomer(p->getId());
			if (acct)
				dynamic_cast<Customer*>(p)->setAccount(acct);
			break;
		}
		case Session::EMPLOYEE:
		case Session::ADMIN:
			break;
		default:
			return false;
		}

		setSessionUser(p);
		this->bIsLoggedIn = true;
		return true;
	}
	return false;
}

void Session::logout() {
	this->bIsLoggedIn = false;
	delete m_user;
}

bool Session::changePassword(Person *p, const string &newpassword) {
	if (!p) return false;
	p->setPassword(encrypt(newpassword));

	switch (p->getUserType()) {
	case Session::CUSTOMER: {
		auto *cust = dynamic_cast<Customer*>(p);
		if (!this->updateCustomer(cust))
			return false;
		break;
	}
	case Session::EMPLOYEE: {
		auto *emp = dynamic_cast<Employee*>(p);
		if (!this->updateEmployee(emp))
			return false;
		break;
	}
	case Session::ADMIN: {
		auto *adm = dynamic_cast<Admin*>(p);
		if (!this->updateAdmin(adm))
			return false;
		break;
	}
	default:
		return false;
	}
	return true;
}

bool Session::changePassword(const string &newpassword) {
	m_user->setPassword(encrypt(newpassword));
	return (m_db->insertPerson(m_user));
}

bool Session::verifyPassword(const string &password) {
	return (m_user->getPassword() == this->encrypt(password));
}

string Session::getSessionUser() {
	return this->m_user->getUserName();
}

string Session::getSessionUserType() {
	if (!m_user)
		return "";
	switch(m_user->getUserType()) {
	case Session::CUSTOMER:
		return "Customer";
	case Session::EMPLOYEE:
		return "Employee";
	case Session::ADMIN:
		return "Admin";
	default:
		return "";
	}
}

void Session::setSessionCapabilities() {
	setUserType();
	switch (m_userType) {
	case CUSTOMER: {
		auto *tmpcust = dynamic_cast<Customer*>(m_user);
		if (tmpcust->getAccount() && tmpcust->getAccount()->getId() != 0) {
			m_sessionCapabilities.emplace_back("Transfer money to another Account");
			m_sessionCapabilities.emplace_back("Deposit");
			m_sessionCapabilities.emplace_back("Withdraw");
			m_sessionCapabilities.emplace_back("Print my account Information");
		}
		m_sessionCapabilities.emplace_back("Print my customer Information");
		m_sessionCapabilities.emplace_back("Change own Password");
		break;
	}
	case EMPLOYEE: {

		auto *tmpemp = dynamic_cast<Employee*>(m_user);
		if (tmpemp->canCreateAccount())
			m_sessionCapabilities.emplace_back("Create Account");
		if (tmpemp->canUpdateAccount())
			m_sessionCapabilities.emplace_back("Update Account");
		if (tmpemp->canDeleteAccount())
			m_sessionCapabilities.emplace_back("Delete Account");
		if (tmpemp->canActivateAccount())
			m_sessionCapabilities.emplace_back("Activate Account");
		if (tmpemp->canDeactivateAccount())
			m_sessionCapabilities.emplace_back("Deactivate Account");
		if (tmpemp->canPrintAccountInfo()) {
			m_sessionCapabilities.emplace_back("Print Account Information");
			if (tmpemp->canListAllAccounts())
				m_sessionCapabilities.emplace_back("List All Accounts");
		}
		if (tmpemp->canCreateCustomer())
			m_sessionCapabilities.emplace_back("Create a new Customer");
		if (tmpemp->canUpdateCustomer())
			m_sessionCapabilities.emplace_back("Update Existing Customer");
		if (tmpemp->canDeleteCustomer())
			m_sessionCapabilities.emplace_back("Delete Customer");
		if (tmpemp->canActivateCustomer())
			m_sessionCapabilities.emplace_back("Activate Customer");
		if (tmpemp->canDeactivateCustomer())
			m_sessionCapabilities.emplace_back("Deactivate Customer");
		if (tmpemp->canPrintCustomerInfo()) {
			m_sessionCapabilities.emplace_back("Print Customer Information");
			if (tmpemp->canListAllCustomers())
				m_sessionCapabilities.emplace_back("List All Customers");
		}
		m_sessionCapabilities.emplace_back("Print my Employee Information");
		m_sessionCapabilities.emplace_back("Transfer Money between accounts");
		m_sessionCapabilities.emplace_back("Deposit Money to Customer");
		m_sessionCapabilities.emplace_back("Change Password");
		m_sessionCapabilities.emplace_back("Change own Password");
		break;
	}
	case ADMIN: {
		auto *tmpadmin = dynamic_cast<Admin*>(m_user);
		if (tmpadmin->canCreateAdmin())
			m_sessionCapabilities.emplace_back("Create Administrator");
		if (tmpadmin->canUpdateAdmin())
			m_sessionCapabilities.emplace_back("Update Administrator");
		if (tmpadmin->canDeleteAdmin())
			m_sessionCapabilities.emplace_back("Delete Administrator");
		if (tmpadmin->canActivateAdmin())
			m_sessionCapabilities.emplace_back("Activate Administrator");
		if (tmpadmin->canDeactivateAdmin())
			m_sessionCapabilities.emplace_back("Deactivate Administrator");
		if (tmpadmin->canPrintAdminInfo()) {
			m_sessionCapabilities.emplace_back("Print Administrator Information");
			if (tmpadmin->canListAllAdmin())
				m_sessionCapabilities.emplace_back("List All Administrators");
		}
		if (tmpadmin->canCreateEmployee())
			m_sessionCapabilities.emplace_back("Create Employee");
		if (tmpadmin->canUpdateEmployee())
			m_sessionCapabilities.emplace_back("Update Employee");
		if (tmpadmin->canDeleteEmployee())
			m_sessionCapabilities.emplace_back("Delete Employee");
		if (tmpadmin->canActivateEmployee())
			m_sessionCapabilities.emplace_back("Activate Employee");
		if (tmpadmin->canDeactivateEmployee())
			m_sessionCapabilities.emplace_back("Deactivate Employee");
		if (tmpadmin->canPrintEmployeeInfo())
			m_sessionCapabilities.emplace_back("Print Employee Information");
		if (tmpadmin->canListAllEmployee())
			m_sessionCapabilities.emplace_back("List All Employees");
		if (tmpadmin->canCreateAccount())
			m_sessionCapabilities.emplace_back("Create Account");
		if (tmpadmin->canUpdateAccount())
			m_sessionCapabilities.emplace_back("Update Account");
		if (tmpadmin->canDeleteAccount())
			m_sessionCapabilities.emplace_back("Delete Account");
		if (tmpadmin->canActivateAccount())
			m_sessionCapabilities.emplace_back("Activate Account");
		if (tmpadmin->canDeactivateAccount())
			m_sessionCapabilities.emplace_back("Deactivate Account");
		if (tmpadmin->canListAllAccounts())
			m_sessionCapabilities.emplace_back("List All Accounts");
		if (tmpadmin->canPrintAccountInfo())
			m_sessionCapabilities.emplace_back("Print Account Information");
		if (tmpadmin->canCreateCustomer())
			m_sessionCapabilities.emplace_back("Create a new Customer");
		if (tmpadmin->canUpdateCustomer())
			m_sessionCapabilities.emplace_back("Update Existing Customer");
		if (tmpadmin->canDeleteCustomer())
			m_sessionCapabilities.emplace_back("Delete Customer");
		if (tmpadmin->canActivateCustomer())
			m_sessionCapabilities.emplace_back("Activate Customer");
		if (tmpadmin->canDeactivateCustomer())
			m_sessionCapabilities.emplace_back("Deactivate Customer");
		if (tmpadmin->canPrintCustomerInfo())
			m_sessionCapabilities.emplace_back("Print Customer Information");
		if (tmpadmin->canListAllCustomers())
			m_sessionCapabilities.emplace_back("List All Customers");
		m_sessionCapabilities.emplace_back("Print my Administrator Information");
		m_sessionCapabilities.emplace_back("Transfer Money between accounts");
		m_sessionCapabilities.emplace_back("Deposit Money to Customer");
		m_sessionCapabilities.emplace_back("Change Password");
		m_sessionCapabilities.emplace_back("Change own Password");
		break;
	}
	default:
		break;
	}
}

Person* Session::getPerson(const string &username) {
	return m_db->retrievePerson(username);
}

Customer* Session::getCustomer(const string &username) {
	return dynamic_cast<Customer*>(getPerson(username));
}

Employee* Session::getEmployee(const string &username) {
	return dynamic_cast<Employee*>(getPerson(username));
}

Admin* Session::getAdmin(const string &username) {
	return dynamic_cast<Admin*>(getPerson(username));
}

Account* Session::getAccount(const int accountid) {
	return m_db->retrieveAccount(accountid);
}

bool Session::firstRun() const {
	return (m_totalUsers == 0);
}

int Session::genUserId() {
	return m_db->generatePersonNumber();
}

int Session::genAccountId() {
	return m_db->generateAccountNumber();
}

Customer* Session::getCustomerByAccount(const int accountid) {
	return m_db->retrieveCustomerByAccount(accountid);
}

Account* Session::getAccountByCustomer(const int customerid) {
	return m_db->retrieveAccountByCustomer(customerid);
}
