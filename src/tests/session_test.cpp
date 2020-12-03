#include "session_test.h"
#include <cstdlib>

SessionTest::SessionTest() : sess(nullptr){SetUp();}

SessionTest::~SessionTest() {}

void SessionTest::SetUp() {sess = new Session();}

void SessionTest::TearDown() {}

void SessionTest::TestBody() {}

Admin* SessionTest::makeAdmin(const string username) {
	string natid = to_string(rand());
	Admin *admin = new Admin();
	admin->setId(sess->genUserId());
	admin->setUserName(username);
	admin->setFirstName(username + "_FirstName");
	admin->setLastName(username + "_LastName");
	admin->setNationalId(natid);
	admin->setUserType(1);
	string password = this->sess->encrypt("abc123");
	admin->setPassword(password);
	admin->cap_AdminCreate(true);
	admin->cap_AdminUpdate(true);
	admin->cap_AdminDelete(true);
	admin->cap_AdminActivate(true);
	admin->cap_AdminDeactivate(true);
	admin->cap_AdminPrintInfo(true);
	admin->cap_AdminListAll(true);
	admin->unlock();
	return admin;
}

Employee* SessionTest::makeEmployee(const string username) {
	string natid = to_string(rand());
	Employee *emp = new Employee();
	emp->setId(sess->genUserId());
	emp->setUserName(username);
	emp->setFirstName(username + "_FirstName");
	emp->setLastName(username + "_LastName");
	emp->setNationalId(natid);
	emp->setUserType(2);
	string password = this->sess->encrypt("abc123");
	emp->setPassword(password);

	emp->cap_acctCreate(true);
	emp->cap_acctUpdate(true);
	emp->cap_acctDelete(true);
	emp->cap_acctDeactivate(true);
	emp->cap_acctActivate(true);
	emp->cap_acctListAll(true);
	emp->cap_acctPrintInfo(true);
	emp->cap_custCreate(true);
	emp->cap_custUpdate(true);
	emp->cap_custDelete(true);
	emp->cap_custActivate(true);
	emp->cap_custDeactivate(true);
	emp->cap_custPrintInfo(true);
	emp->cap_custListAll(true);
	emp->unlock();
	return emp;
}

Customer* SessionTest::makeCustomer(const string username) {
	string natid = to_string(rand());
	Customer *customer = new Customer();
	customer->setId(sess->genUserId());
	customer->setUserName(username);
	customer->setFirstName(username + "_FirstName");
	customer->setLastName(username + "_LastName");
	customer->setNationalId(natid);
	customer->setUserType(3);
	string password = this->sess->encrypt("abc123");
	customer->setPassword(password);
	customer->setAccount(nullptr);
	customer->unlock();
	return customer;
}

Account* SessionTest::makeAccount(int owner) {
	Account *account = new Account();
	account->setId(sess->genAccountId());
	account->setAccountLabel("Current");
	account->setBalance(0);
	account->setCustomerId(owner);
	account->unlock();
	return account;
}

TEST_F(SessionTest, CreateSuperUser) {
	SessionTest s;
	Admin *super = makeAdmin("admin0");
	if (s.sess->firstRun())
		EXPECT_TRUE(s.sess->createAdmin(super));
	else
		EXPECT_FALSE(s.sess->createAdmin(super));
}

TEST_F(SessionTest, SuperUserLogin) {
	SessionTest s;
	EXPECT_TRUE(s.sess->login("admin0", "abc123"));
}

TEST_F(SessionTest, CreateAdminUser_NotLoggedIn_Session) {
	SessionTest s;
	Admin *admin = makeAdmin("admin1");
	EXPECT_FALSE(s.sess->createAdmin(admin));
}

TEST_F(SessionTest, CreateAdminUser_LoggedIn_Session) {
	SessionTest s;
	s.sess->login("admin0", "abc123");
	Admin *admin = makeAdmin("admin2");
	if (s.sess->getAdmin("admin2"))
		EXPECT_FALSE(s.sess->createAdmin(admin));
	else
		EXPECT_TRUE(s.sess->createAdmin(admin));
}

TEST_F(SessionTest, CreateAdminUser_DUP_LoggedIn_Session) {
	SessionTest s;
	s.sess->login("admin0", "abc123");
	Admin *admin = makeAdmin("admin3");
	if (s.sess->getAdmin("admin3"))
		EXPECT_FALSE(s.sess->createAdmin(admin));
	else
		EXPECT_TRUE(s.sess->createAdmin(admin));
}

TEST_F(SessionTest, UpdateAdminUser_LoggedIn_Session) {
	SessionTest s;
	s.sess->login("admin0", "abc123");
	Admin *admin = s.sess->getAdmin("admin2");
	admin->setNationalId("834756");
	admin->cap_AdminCreate(false);
	admin->cap_AdminUpdate(false);
	admin->cap_AdminDelete(false);
	admin->cap_AdminActivate(true);
	admin->cap_AdminDeactivate(true);
	admin->cap_AdminPrintInfo(true);
	admin->cap_AdminListAll(true);
	EXPECT_TRUE(s.sess->updateAdmin(admin));
}

TEST_F(SessionTest, CreateCustomer1) {
	SessionTest s;
	s.sess->login("admin0", "abc123");
	Customer *cust = makeCustomer("cust1");
	if (s.sess->getCustomer("cust1"))
		EXPECT_FALSE(s.sess->createCustomer(cust));
	else
		EXPECT_TRUE(s.sess->createCustomer(cust));
}

TEST_F(SessionTest, CreateAccount1) {
	SessionTest s;
	s.sess->login("admin0", "abc123");
	Customer *cust = s.sess->getCustomer("cust1");
	Account *acct = makeAccount(cust->getId());
	s.sess->createAccount(acct);
	cust->setAccount(acct);
	EXPECT_TRUE(s.sess->updateCustomer(cust));
}

TEST_F(SessionTest, CreateCustomer2) {
	SessionTest s;
	s.sess->login("admin0", "abc123");
	Customer *cust = makeCustomer("cust2");
	if (s.sess->getCustomer("cust2"))
		EXPECT_FALSE(s.sess->createCustomer(cust));
	else
		EXPECT_TRUE(s.sess->createCustomer(cust));
}

TEST_F(SessionTest, CreateAccount2) {
	SessionTest s;
	s.sess->login("admin0", "abc123");
	Customer *cust = s.sess->getCustomer("cust2");
	Account *acct = makeAccount(cust->getId());
	s.sess->createAccount(acct);
	cust->setAccount(acct);
	EXPECT_TRUE(s.sess->updateCustomer(cust));
}

TEST_F(SessionTest, DepositAccount1) {
	SessionTest s;
	s.sess->login("cust1", "abc123");
	Customer *cust = s.sess->getCustomer("cust1");
	int balance = cust->getAccount()->getBalance();
	s.sess->withdraw(balance);
	s.sess->deposit(2000);
	cust = s.sess->getCustomer("cust1");
	EXPECT_EQ(2000, cust->getAccount()->getBalance());
}

TEST_F(SessionTest, WithdrawAccount1) {
	SessionTest s;
	s.sess->login("cust1", "abc123");
	Customer *cust = s.sess->getCustomer("cust1");
	s.sess->withdraw(1000);
	cust = s.sess->getCustomer("cust1");
	EXPECT_EQ(1000, cust->getAccount()->getBalance());
}

TEST_F(SessionTest, CustomerTransferFromOwnToAnother) {
	SessionTest s;
	s.sess->login("cust1", "abc123");
	Customer *cust = s.sess->getCustomer("cust1");
	Customer *cust2 = s.sess->getCustomer("cust2");
	s.sess->transfer(cust2->getAccount()->getId(), 300);
	cust = s.sess->getCustomer("cust1");
	EXPECT_EQ(700, cust->getAccount()->getBalance());
}

TEST_F(SessionTest, CustomerTransferFromOwnToAnother_NoEnoughCredit) {
	SessionTest s;
	s.sess->login("cust1", "abc123");
	Customer *cust = s.sess->getCustomer("cust1");
	EXPECT_FALSE(s.sess->transfer(2, 900));
}

TEST_F(SessionTest, CreateEmployee1) {
	SessionTest s;
	s.sess->login("admin2", "abc123");
	Employee *emp = makeEmployee("emp1");
	if (s.sess->getEmployee("emp1"))
		EXPECT_FALSE(s.sess->createEmployee(emp));
	else
		EXPECT_TRUE(s.sess->createEmployee(emp));
}

TEST_F(SessionTest, CreateCustomer3) {
	SessionTest s;
	s.sess->login("emp1", "abc123");
	Customer *cust = makeCustomer("cust3");
	if (s.sess->getCustomer("cust3"))
		EXPECT_FALSE(s.sess->createCustomer(cust));
	else
		EXPECT_TRUE(s.sess->createCustomer(cust));
}

TEST_F(SessionTest, CreateAccount3) {
	SessionTest s;
	s.sess->login("emp1", "abc123");
	Customer *cust = s.sess->getCustomer("cust3");
	Account *acct = makeAccount(cust->getId());
	s.sess->createAccount(acct);
	cust->setAccount(acct);
	EXPECT_TRUE(s.sess->updateCustomer(cust));
}

TEST_F(SessionTest, RemoveUpdCustPrivFromEmployee1) {
	SessionTest s;
	s.sess->login("admin2", "abc123");
	Employee *emp = s.sess->getEmployee("emp1");
	emp->cap_custUpdate(false);
	s.sess->updateEmployee(emp);
	s.sess->logout();
	s.sess->login("emp1", "abc123");
	Customer *cust = s.sess->getCustomer("cust1");
	cust->setFirstName("Dummy");
	EXPECT_FALSE(s.sess->updateCustomer(cust));
}

TEST_F(SessionTest, DeleteAdminUser_LoggedIn_Session) {
	SessionTest s;
	s.sess->login("admin0", "abc123");
	Admin *admin = s.sess->getAdmin("admin2");
	EXPECT_TRUE(s.sess->deleteAdmin(admin));
}


TEST_F(SessionTest, DeleteAccount1) {
        SessionTest s;
        s.sess->login("emp1", "abc123");
        Customer *cust = s.sess->getCustomer("cust1");
        EXPECT_TRUE(s.sess->deleteAccount(cust->getAccount()));
}

TEST_F(SessionTest, DeleteAccount2) {
        SessionTest s;
        s.sess->login("emp1", "abc123");
        Customer *cust = s.sess->getCustomer("cust2");
        EXPECT_TRUE(s.sess->deleteAccount(cust->getAccount()));
}

TEST_F(SessionTest, DeleteAccount3) {
        SessionTest s;
        s.sess->login("emp1", "abc123");
        Customer *cust = s.sess->getCustomer("cust3");
        EXPECT_TRUE(s.sess->deleteAccount(cust->getAccount()));
}

TEST_F(SessionTest, DeleteCustomer1) {
        SessionTest s;
        s.sess->login("emp1", "abc123");
        Customer *cust = s.sess->getCustomer("cust1");
        EXPECT_TRUE(s.sess->deleteCustomer(cust));
}

TEST_F(SessionTest, DeleteCustomer2) {
        SessionTest s;
        s.sess->login("emp1", "abc123");
        Customer *cust = s.sess->getCustomer("cust2");
        EXPECT_TRUE(s.sess->deleteCustomer(cust));
}

TEST_F(SessionTest, DeleteCustomer3) {
        SessionTest s;
        s.sess->login("emp1", "abc123");
        Customer *cust = s.sess->getCustomer("cust3");
        EXPECT_TRUE(s.sess->deleteCustomer(cust));
}

TEST_F(SessionTest, DeleteEmployee1) {
        SessionTest s;
        s.sess->login("admin0", "abc123");
        Employee *emp = s.sess->getEmployee("emp1");
        EXPECT_TRUE(s.sess->deleteEmployee(emp));
}

TEST_F(SessionTest, DeleteAdmin3) {
        SessionTest s;
        s.sess->login("admin0", "abc123");
        Admin *admin = s.sess->getAdmin("admin3");
        EXPECT_TRUE(s.sess->deleteAdmin(admin));
}

TEST_F(SessionTest, DeleteSuperAdmin) {
        SessionTest s;
        s.sess->login("admin0", "abc123");
        Admin *admin = s.sess->getAdmin("admin0");
        EXPECT_TRUE(s.sess->deleteAdmin(admin));
}

TEST(Database, DISABLED_DeleteDatabaseFile) {
        EXPECT_FALSE(remove("future_bank.db")); // remove() returns 0 when the file is successfully deleted
}

