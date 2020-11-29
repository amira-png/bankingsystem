/*
 * admin.h
 *
 *  Created on: Oct 19, 2020
 *      Author: amira
 */

#ifndef SRC_ADMIN_H_
#define SRC_ADMIN_H_

#include "employee.h"

class Admin : public Employee {
private:
	bool bAdminCreate;
	bool bAdminUpdate;
	bool bAdminDelete;
	bool bAdminListAll;
	bool bAdminPrintInfo;
	bool bAdminActivate;
	bool bAdminDeactivate;

	bool bEmployeeCreate;
	bool bEmployeeUpdate;
	bool bEmployeeDelete;
	bool bEmployeePrintInfo;
	bool bEmployeeListAll;
	bool bEmployeeActivate;
	bool bEmployeeDeactivate;

public:
	Admin() : bAdminCreate(false),bAdminUpdate(false),
				bAdminDelete(false), bAdminListAll(false),
				bAdminPrintInfo(false),bAdminActivate(false),
				bAdminDeactivate(false),
				bEmployeeCreate(true), bEmployeeUpdate(true),bEmployeeDelete(true),
				bEmployeePrintInfo(true),bEmployeeListAll(true),bEmployeeActivate(true),
				bEmployeeDeactivate(true)
				{
		Employee::cap_acctActivate(true);
		Employee::cap_acctCreate(true);
		Employee::cap_acctDeactivate(true);
		Employee::cap_acctDelete(true);
		Employee::cap_acctListAll(true);
		Employee::cap_acctPrintInfo(true);
		Employee::cap_acctUpdate(true);
		Employee::cap_custCreate(true);
		Employee::cap_custDelete(true);
		Employee::cap_custUpdate(true);
		Employee::cap_custActivate(true);
		Employee::cap_custDeactivate(true);
		Employee::cap_custPrintInfo(true);
		Employee::cap_custListAll(true);
		Employee::lock();
	}
	virtual ~Admin() = default;
	void cap_AdminCreate(bool value) {bAdminCreate = value;}
	void cap_AdminUpdate(bool value) {bAdminUpdate = value;}
	void cap_AdminDelete(bool value) {bAdminDelete = value;}
	void cap_AdminListAll(bool value) {bAdminListAll = value;}
	void cap_AdminPrintInfo(bool value) {bAdminPrintInfo = value;}
	void cap_AdminActivate(bool value) {bAdminActivate = value;}
	void cap_AdminDeactivate(bool value) {bAdminDeactivate = value;}

	void cap_EmployeeCreate(bool value) {bEmployeeCreate = value;}
	void cap_EmployeeUpdate(bool value) {bEmployeeUpdate = value;}
	void cap_EmployeeDelete(bool value) {bEmployeeDelete = value;}
	void cap_EmployeeListAll(bool value) {bEmployeeListAll = value;}
	void cap_EmployeePrintInfo(bool value) {bEmployeePrintInfo = value;}
	void cap_EmployeeActivate(bool value) {bEmployeeActivate = value;}
	void cap_EmployeeDeactivate(bool value) {bEmployeeDeactivate = value;}

	bool canCreateAdmin() {return bAdminCreate;}
	bool canUpdateAdmin() {return bAdminUpdate;}
	bool canDeleteAdmin() {return bAdminDelete;}
	bool canActivateAdmin() {return bAdminActivate;}
	bool canDeactivateAdmin() {return bAdminDeactivate;}
	bool canListAllAdmin() {return bAdminListAll;}
	bool canPrintAdminInfo() {return bAdminPrintInfo;}

	bool canCreateEmployee() {return bEmployeeCreate;}
	bool canUpdateEmployee() {return bEmployeeUpdate;}
	bool canDeleteEmployee() {return bEmployeeDelete;}
	bool canPrintEmployeeInfo() {return bEmployeePrintInfo;}
	bool canListAllEmployee() {return bEmployeeListAll;}
	bool canActivateEmployee() {return bEmployeeActivate;}
	bool canDeactivateEmployee() {return bEmployeeDeactivate;}
};

#endif /* SRC_ADMIN_H_ */
