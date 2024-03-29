/*
 * employee.h
 *
 *  Created on: Nov 3, 2020
 *      Author: Amira
 */

#ifndef SRC_EMPLOYEE_H_
#define SRC_EMPLOYEE_H_

#include <string>
#include "person.h"
#include "customer.h"
#include "account.h"

class Employee : public Person {

private:
	bool bAcctCreate;
	bool bAcctUpdate;
	bool bAcctDelete;
	bool bAcctDeactivate;
	bool bAcctActivate;
	bool bAcctListAll;
	bool bAcctPrintInfo;

	bool bCustCreate;
	bool bCustUpdate;
	bool bCustDelete;
	bool bCustActivate;
	bool bCustDeactivate;
	bool bCustListAll;
	bool bCustPrintInfo;

public:
	Employee() : bAcctCreate(false), bAcctUpdate(false),
				bAcctDelete(false), bAcctDeactivate(false), bAcctActivate(false),
				bAcctListAll(false), bAcctPrintInfo(false), bCustCreate(false),
				bCustUpdate(false), bCustDelete(false), bCustActivate(false), bCustDeactivate(false),
				bCustListAll(false), bCustPrintInfo(false) {}
	~Employee() override = default;

	void cap_acctCreate(bool value) {bAcctCreate = value;}
	void cap_acctUpdate(bool value) {bAcctUpdate = value;}
	void cap_acctDelete(bool value) {bAcctDelete = value;}
	void cap_acctDeactivate(bool value) {bAcctDeactivate = value;}
	void cap_acctActivate(bool value) {bAcctActivate = value;}
	void cap_acctListAll(bool value) {bAcctListAll = value;}
	void cap_acctPrintInfo(bool value) {bAcctPrintInfo = value;}
	void cap_custCreate(bool value) {bCustCreate = value;}
	void cap_custUpdate(bool value) {bCustUpdate = value;}
	void cap_custDelete(bool value) {bCustDelete = value;}
	void cap_custActivate(bool value) {bCustActivate = value;}
	void cap_custDeactivate(bool value) {bCustDeactivate = value;}
	void cap_custPrintInfo(bool value) {bCustPrintInfo = value;}
	void cap_custListAll(bool value) {bCustListAll = value;}
	void lock() override {Person::bLocked = true;}
	void unlock() override {Person::bLocked = false;}

	bool canCreateAccount() const {return bAcctCreate;}
	bool canUpdateAccount() const {return bAcctUpdate;}
	bool canDeleteAccount() const {return bAcctDelete;}
	bool canDeactivateAccount() const {return bAcctDeactivate;}
	bool canActivateAccount() const {return bAcctActivate;}
	bool canListAllAccounts() const {return bAcctListAll;}
	bool canPrintAccountInfo() const {return bAcctPrintInfo;}
	bool canPrintCustomerInfo() const {return bCustPrintInfo;}
	bool canListAllCustomers() const {return bCustListAll;}
	bool canCreateCustomer() const {return bCustCreate;}
	bool canUpdateCustomer() const {return bCustUpdate;}
	bool canDeleteCustomer() const {return bCustDelete;}
	bool canActivateCustomer() const {return bCustActivate;}
	bool canDeactivateCustomer() const {return bCustDeactivate;}
};

#endif /* SRC_EMPLOYEE_H_ */
