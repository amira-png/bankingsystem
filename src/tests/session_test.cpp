#include "session_test.h"
#include <cstdlib>

SessionTest::SessionTest() : sess(nullptr){
	SetUp();
}

SessionTest::~SessionTest() {}

void SessionTest::SetUp() {
	sess = new Session();
}

void SessionTest::TearDown() {}

void SessionTest::TestBody() {}

Admin* SessionTest::makeAdmin(const string username) {
	string natid = to_string(rand());
	Admin *admin = new Admin();
        admin->setId(sess->genUserId());
        admin->setUserName(username);
        admin->setFirstName(username+"_FirstName");
        admin->setLastName(username+"_LastName");
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
	emp->setFirstName(username+"_FirstName");
	emp->setLastName(username+"_LastName");
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
        customer->setFirstName(username+"_FirstName");
        customer->setLastName(username+"_LastName");
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
	return account;
}

TEST_F(SessionTest, CreateSuperUser) {
        SessionTest s;
	Admin *super = makeAdmin("admin0");
	EXPECT_TRUE(s.sess->createAdmin(super));
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
        EXPECT_TRUE(s.sess->createAdmin(admin));
}

TEST_F(SessionTest, CreateAdminUser_DUP_LoggedIn_Session) {
        SessionTest s;
	s.sess->login("admin0", "abc123");
	Admin *admin = makeAdmin("admin3");
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


TEST_F(SessionTest, DeleteAdminUser_LoggedIn_Session) {
        SessionTest s;
        s.sess->login("admin0", "abc123");
        Admin *admin = s.sess->getAdmin("admin2");
        EXPECT_TRUE(s.sess->deleteAdmin(admin));
}



TEST_F(SessionTest, CreateCustomer) {
	SessionTest s;
	s.sess->login("admin0", "abc123");
	Customer *cust = makeCustomer("cust1");
	EXPECT_TRUE(s.sess->createCustomer(cust));
}


TEST_F(SessionTest, CreateAccount) {
	SessionTest s;
	s.sess->login("admin0", "abc123");
	Customer *cust = s.sess->getCustomer("cust1");
	Account *acct = makeAccount(cust->getId());
	s.sess->createAccount(acct);
	cust->setAccount(acct);
	EXPECT_TRUE(s.sess->updateCustomer(cust)); 
}


