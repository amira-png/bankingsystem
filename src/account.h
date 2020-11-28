/*
 * account.h
 *
 *  Created on: Oct 19, 2020
 *      Author: amira
 */

#ifndef SRC_ACCOUNT_H_
#define SRC_ACCOUNT_H_


class Account{

private:
	bool bLocked;
	int m_id;
	float m_balance;
	int m_customerId;

public:
	Account() : bLocked(true), m_id(0), m_balance(0), m_customerId(0) {}
	virtual ~Account() = default;
	void lock(){bLocked = true;}
	void unlock(){bLocked = false;}
	bool isLocked() {return bLocked;}
	Account* operator = (const Account &acct) {
		this->m_id = acct.m_id;
		this->bLocked = acct.bLocked;
		this->m_balance = acct.m_balance;
		this->m_customerId = acct.m_customerId;
		return this;
	}
	void setId(const int id) {m_id = id;}
	int getId() {return m_id;}
	double getBalance() {return m_balance;}
	void setBalance(double newBalance) {m_balance = newBalance;}
	int getCustomerId() {return m_customerId;}
	void setCustomerId(const int custid) {m_customerId = custid;}
};

#endif /* SRC_ACCOUNT_H_ */
