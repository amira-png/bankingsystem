#include "session_test.h"

SessionTest::SessionTest() {
	SetUp();
}

SessionTest::~SessionTest() {}

void SessionTest::SetUp() {
	sess = new Session();
}

void SessionTest::TearDown() {}

void SessionTest::TestBody() {}


TEST_F(SessionTest, CreateSuperUser) {
        SessionTest s;
	Admin *super = new Admin();
	super->setId(1);
	super->setUserName("ahmed");
	super->setFirstName("Ahmed");
	super->setLastName("Mostafa");
	super->setNationalId("2222222");
	super->setUserType(1);
	string password = s.sess->encrypt("abc123");
	super->setPassword(password);
	super->cap_AdminCreate(true);
	super->cap_AdminUpdate(true);
	super->cap_AdminDelete(true);
	super->cap_AdminActivate(true);
	super->cap_AdminDeactivate(true);
	super->cap_AdminPrintInfo(true);
	super->cap_AdminListAll(true);

	EXPECT_TRUE(s.sess->createAdmin(super));

}

TEST_F(SessionTest, CreateAdminUser_NotLoggedIn_Session) {
    	SessionTest s;
	Admin *admin = new Admin();
	admin->setId(2);
	admin->setUserName("shimaa");
	admin->setFirstName("Shimaa");
	admin->setLastName("Mostafa");
	admin->setNationalId("12121212");
	admin->setUserType(1);
	string password = s.sess->encrypt("abc123");
	admin->setPassword(password);
	admin->cap_AdminCreate(true);
	admin->cap_AdminUpdate(true);
	admin->cap_AdminDelete(true);
	admin->cap_AdminActivate(true);
	admin->cap_AdminDeactivate(true);
	admin->cap_AdminPrintInfo(true);
	admin->cap_AdminListAll(true);

	EXPECT_FALSE(s.sess->createAdmin(admin));
}
