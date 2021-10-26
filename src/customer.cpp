/*
 * customer.cpp
 *
 *  Created on: Nov 6, 2020
 *      Author: Amira
 */

#include <sstream>
#include "session.h"
#include "userinterface.h"
#include <limits>
#ifdef __linux__
#include <libsmartcols/libsmartcols.h>
#include <clocale>
#endif

using namespace std;

/*
 * Session methods
 * */

bool Session::withdraw(const float sum) {
	auto *cust = dynamic_cast<Customer*>(m_user);
	if (!cust || !bIsLoggedIn)
		return false;

	if (cust->getAccount()->getId() <= 0)
		return false;

	float oldBalance = cust->getAccount()->getBalance();
	if (cust->getAccount()->getBalance() < sum)
		return false;

	cust->getAccount()->setBalance(oldBalance - sum);
	// Strictly verify since it is user money
	if (cust->getAccount()->getBalance() == (oldBalance - sum))
		if (m_db->insertAccount(cust->getAccount()))
			return true;
	return false;
}

bool Session::transfer(const int to, const float sum) {
	auto *cust = dynamic_cast<Customer*>(this->m_user);
	if (!cust || !bIsLoggedIn)
		return false;

	if (cust->getAccount()->getId() <= 0)
		return false;

	float oldBalance = cust->getAccount()->getBalance();
	if (cust->getAccount()->getBalance() < sum)
		return false;

	Account *toAccount = m_db->retrieveAccount(to);
	if (toAccount) {
		cust->getAccount()->setBalance(oldBalance - sum);
        int oldAccountBalance;
        oldAccountBalance = toAccount->getBalance();
		toAccount->setBalance(oldAccountBalance + sum);

		// Strictly verify since it is user money
		if (toAccount->getBalance() == (oldAccountBalance + sum))
			if (m_db->insertAccount(cust->getAccount())
					&& m_db->insertAccount(toAccount))
				return true;
	}
	return false;
}

bool Session::deposit(const float sum) {
	auto *cust = dynamic_cast<Customer*>(m_user);
	if (!cust || !bIsLoggedIn)
		return false;

	if (cust->getAccount()->getId() <= 0)
		return false;

	float oldBalance = cust->getAccount()->getBalance();
	cust->getAccount()->setBalance(sum + oldBalance);

	// Strictly verify since it is user money
	if (cust->getAccount()->getBalance() == (oldBalance + sum))
		if (m_db->insertAccount(cust->getAccount()))
			return true;
	return false;
}

bool Session::printCustomerInfo() {
	auto *cust = dynamic_cast<Customer*>(m_user);
	if (!bIsLoggedIn || !cust) return false;
	return printCustomerInfo(cust);
}

bool Session::printAccountInfo() {
	auto *cust = dynamic_cast<Customer*>(m_user);
	if (!bIsLoggedIn) return false;
	if (cust->getAccount()->getId() == 0) return false;
	return printAccountInfo(cust->getAccount());
}

/*
 * User interface methods
 * */

void Ui::ui_transfer_own() {
	int account_number;
	float sum;

	cout << "Enter destination account number: ";
	cin >> account_number;

	while (true) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter destination account number: ";
			cin >> account_number;
		} else
			break;
	}

	cout << "Enter sum: ";
	cin >> sum;
	while (true) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter sum: ";
			cin >> sum;
		} else
			break;
	}

	if (!m_session->transfer(account_number, sum)) {
		cerr << "Amount or bank account are invalid" << endl;
		return;
	}
	else
		cout << "Successful transfer" << endl;
}

void Ui::ui_deposit_own() {
	float sum;
	cout << "Enter sum: ";
	cin >> sum;
	while (true) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter sum: ";
			cin >> sum;
		} else
			break;
	}

	if (!m_session->deposit(sum)) {
		cerr << "Invalid deposit" << endl;
		return;
	}
	else
		cout << "Successful deposit" << endl;
}

void Ui::ui_withdraw() {
	float sum;
	cout << "Enter sum: ";
	cin >> sum;
	while (true) {
		if (cin.fail()) {
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Enter sum: ";
			cin >> sum;
		} else
			break;
	}

	if (!m_session->withdraw(sum)) {
		cerr << "Invalid withdrawal" << endl;
		return;
	}
	else
		cout << "Successful withdrawal" << endl;
}

void Ui::ui_print_own_customer() {
	if (!m_session->printCustomerInfo())
		cerr << "Error printing my customer info" << endl;
}

void Ui::ui_print_own_account() {
	if (!m_session->printAccountInfo())
		cerr << "Error printing my account info or no account associated" << endl;
}
