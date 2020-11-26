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
	Employee() : bAcctCreate(true),bAcctUpdate(true),
				bAcctDelete(true), bAcctDeactivate(true), bAcctActivate(true),
				bAcctListAll(false),bAcctPrintInfo(true),bCustCreate(true),
				bCustUpdate(true),bCustDelete(true),bCustActivate(true),bCustDeactivate(true),
				bCustListAll(false), bCustPrintInfo(true) {}
	virtual ~Employee() = default;

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
	void lock() {Person::bLocked = true;}
	void unlock() {Person::bLocked = false;}

	bool canCreateAccount() {return bAcctCreate;}
	bool canUpdateAccount() {return bAcctUpdate;}
	bool canDeleteAccount() {return bAcctDelete;}
	bool canDeactivateAccount() {return bAcctDeactivate;}
	bool canActivateAccount() {return bAcctActivate;}
	bool canListAllAccounts() {return bAcctListAll;}
	bool canPrintAccountInfo() {return bAcctPrintInfo;}
	bool canPrintCustomerInfo() {return bCustPrintInfo;}
	bool canListAllCustomers() {return bCustListAll;}
	bool canCreateCustomer() {return bCustCreate;}
	bool canUpdateCustomer() {return bCustUpdate;}
	bool canDeleteCustomer() {return bCustDelete;}
	bool canActivateCustomer() {return bCustActivate;}
	bool canDeactivateCustomer() {return bCustDeactivate;}
};

#endif /* SRC_EMPLOYEE_H_ */
