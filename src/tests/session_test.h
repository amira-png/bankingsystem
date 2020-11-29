#include <iostream>
#include <string>
#include <gtest/gtest.h>
#include "../person.h"
#include "../customer.h"
#include "../employee.h"
#include "../admin.h"
#include "../database.h"
#include "../database.cpp"
#include "../session.h"
#include "../session.cpp"

using namespace std;
using std::string;

class SessionTest : public ::testing::Test
{
public:
        SessionTest();
        ~SessionTest();
	Session *sess;

protected:
        virtual void SetUp();
        virtual void TearDown();
        virtual void TestBody();
};

