/*
 * account.h
 *
 *  Created on: Oct 19, 2020
 *      Author: amira
 */

#ifndef SRC_ACCOUNT_H_
#define SRC_ACCOUNT_H_

#include <string>

using namespace std;

class Account{

private:
	bool bLocked;
	int m_id;
	float m_balance;
	string m_label;
	int m_customerId;

public:
	Account() : bLocked(true), m_id(0), m_balance(0), m_label(""), m_customerId(0) {}
	virtual ~Account() = default;
	void lock(){bLocked = true;}
	void unlock(){bLocked = false;}
	bool isLocked() const {return bLocked;}
	Account* operator = (const Account &acct) {
		this->m_id = acct.m_id;
		this->bLocked = acct.bLocked;
		this->m_balance = acct.m_balance;
		this->m_customerId = acct.m_customerId;
		this->m_label = acct.m_label;
		return this;
	}
	void setId(const int id) {m_id = id;}
	int getId() const {return m_id;}
	void setBalance(const float newBalance) {m_balance = newBalance;}
	double getBalance() const {return m_balance;}
	void setAccountLabel(const string& label) {m_label = label;}
	string getAccountLabel() {return m_label;}
	void setCustomerId(const int custid) {m_customerId = custid;}
	int getCustomerId() const {return m_customerId;}
};

#endif /* SRC_ACCOUNT_H_ */
