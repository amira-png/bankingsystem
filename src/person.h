/*
 * employee.h
 *
 *  Created on: Oct 19, 2020
 *      Author: amira
 */

#ifndef SRC_PERSON_H_
#define SRC_PERSON_H_

#include <string>

using namespace std;

class Person{

private:
	int m_id;
	string m_firstName;
	string m_lastName;
	string m_nationalId;
	string m_password;
	string m_userName;
	size_t m_caps;
	int m_type;

protected:
	bool bLocked;

public:
	Person():m_id(0),m_firstName(""), m_lastName(""), m_nationalId(""),
			m_password(""), m_userName("") , m_caps(0), m_type(0), bLocked(true) {}
	virtual ~Person() = default;

	void setFirstName(const string &name) {m_firstName = name;}
	void setLastName(const string &name) {m_lastName = name;}
	void setId(int id) {m_id = id;}
	void setNationalId(const string &natid) {m_nationalId = natid;}
	void setPassword(const string &pass) {m_password = pass;}
	void setUserName(const string &username) {m_userName = username;}
	void setUserType(const int type) {m_type = type;}
	void setCaps(size_t caps) {m_caps = caps;}

	int    getId() const        {return m_id;}
	int    getUserType() const  {return m_type;}
	string getFirstName() {return m_firstName;}
	string getLastName()  {return m_lastName;}
	string getNationalId(){return m_nationalId;}
	string getPassword()  {return m_password;}
	string getUserName()  {return m_userName;}
	size_t getCaps() const	  {return m_caps;}

    virtual void lock(){bLocked = true;}

    virtual void unlock() {bLocked = false;}
	bool isLocked() const {return bLocked;}
};

#endif /* SRC_PERSON_H_ */
