/*
 * Robot_Connecotr.h
 *
 *  Created on: Dec 24, 2015
 *      Author: zhangyalei
 */

#ifndef ROBOT_CONNECTOR_H_
#define ROBOT_CONNECTOR_H_

#include "Connector.h"

class Robot_Login_Connector: public Connector {
public:
	static Robot_Login_Connector *instance(void);

	virtual void process_list(void);

private:
	Robot_Login_Connector(void);
	virtual ~Robot_Login_Connector(void);
	Robot_Login_Connector(const Robot_Login_Connector &);
	const Robot_Login_Connector &operator=(const Robot_Login_Connector &);

private:
	static Robot_Login_Connector *instance_;
};

#define ROBOT_LOGIN_CONNECTOR Robot_Login_Connector::instance()

////////////////////////////////////////////////////////////////////////////////

class Robot_Gate_Connector: public Connector {
public:
	static Robot_Gate_Connector *instance(void);

	virtual void process_list(void);

private:
	Robot_Gate_Connector(void);
	virtual ~Robot_Gate_Connector(void);
	Robot_Gate_Connector(const Robot_Gate_Connector &);
	const Robot_Gate_Connector &operator=(const Robot_Gate_Connector &);

private:
	static Robot_Gate_Connector *instance_;
};

#define ROBOT_GATE_CONNECTOR Robot_Gate_Connector::instance()

#endif /* ROBOT_CONNECTOR_H_ */
