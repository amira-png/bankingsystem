/*
 * userinterface.h
 *
 *  Created on: Nov 10, 2020
 *      Author: amira
 */

#ifndef SRC_USERINTERFACE_H_
#define SRC_USERINTERFACE_H_

#include <string>
#include <vector>
#include <map>
#include <functional>
#include "session.h"
#include "person.h"
#include "customer.h"
#include "employee.h"
#include "admin.h"

#define VERSION "1.2"

class Ui {

private:
	Session *m_session;
	vector<string> m_capabilitiesLabels;
	std::function<void(void)> m_execute;
	void getCallBack(const string &desc);
	map<int, string> m_operationsMap;
	inline void printNewLines(int c);

	void registerSuperAdmin();
	void showWelcomeScreen();
	void listWhatToDo();
	void logout();

	void ui_create_admin();
	void ui_update_admin();
	void ui_delete_admin();
	void ui_activate_admin();
	void ui_deactivate_admin();
	void ui_print_admin();
	void ui_print_own_admin();
	void ui_listall_admin();

	void ui_create_employee();
	void ui_update_employee();
	void ui_delete_employee();
	void ui_activate_employee();
	void ui_deactivate_employee();
	void ui_print_employee();
	void ui_print_own_employee();
	void ui_listall_employee();

	void ui_create_customer();
	void ui_update_customer();
	void ui_delete_customer();
	void ui_activate_customer();
	void ui_deactivate_customer();
	void ui_print_customer();
	void ui_listall_customer();

	void ui_create_account();
	void ui_update_account();
	void ui_delete_account();
	void ui_activate_account();
	void ui_deactivate_account();
	void ui_print_account();
	void ui_listall_account();

	void ui_transfer();
	void ui_transfer_own();
	void ui_deposit();
	void ui_deposit_own();
	void ui_withdraw();

	void ui_print_own_customer();
	void ui_print_own_account();
	void ui_change_password();
	void ui_change_own_password();

public:
	Ui();
	~Ui();
	int run();
};

#endif /* SRC_USERINTERFACE_H_ */
