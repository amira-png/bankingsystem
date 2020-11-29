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



TEST_F(SessionTest, AdminLogin) {
	SessionTest s;
	EXPECT_TRUE(s.sess->login("kmoussa", "intruder"));
}


TEST_F(SessionTest, EmpLogin) {
        SessionTest s;
        EXPECT_TRUE(s.sess->login("amira", "abc123"));
}
