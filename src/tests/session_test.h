#include <iostream>
#include <string>
#include <gtest/gtest.h>
#include "../person.h"
#include "../customer.h"
#include "../employee.h"
#include "../admin.h"
#include "../database.h"
#include "../session.h"

using namespace std;
using std::string;

class SessionTest : public ::testing::Test
{
public:
        SessionTest();
        ~SessionTest();
	Session *sess;
	Admin* makeAdmin(const string& username) const;
	Employee* makeEmployee(const string& username) const;
	Customer* makeCustomer(const string& username) const;
	Account* makeAccount(int owner) const;

protected:
        virtual void SetUp();
        virtual void TearDown();
        virtual void TestBody();
};

