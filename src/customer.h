/*
 * customer.h
 *
 *  Created on: Oct 19, 2020
 *      Author: amira
 */

#ifndef SRC_CUSTOMER_H_
#define SRC_CUSTOMER_H_

#include "person.h"
#include "account.h"


class Customer : public Person{

private:
	Account *m_account;

public:
	Customer() : m_account(nullptr) {}
	virtual ~Customer() = default;
	void lock() {Person::bLocked = true;}
	void unlock() {Person::bLocked = false;}
	bool isLocked(){return Person::bLocked;}
	void setAccount(Account *acct) {m_account = acct;}
	Account* getAccount() {return this->m_account;}
};

#endif /* SRC_CUSTOMER_H_ */


