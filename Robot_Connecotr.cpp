/*
 * Robot_Connecotr.cpp
 *
 *  Created on: Dec 24, 2015
 *      Author: zhangyalei
 */

#include "Robot_Connector.h"
#include "Robot_Manager.h"

Robot_Login_Connector::Robot_Login_Connector(void) { }

Robot_Login_Connector::~Robot_Login_Connector(void) { }

Robot_Login_Connector *Robot_Login_Connector::instance_ = 0;

Robot_Login_Connector *Robot_Login_Connector::instance(void) {
	if (! instance_)
		instance_ = new Robot_Login_Connector;
	return instance_;
}

void Robot_Login_Connector::process_list(void) {
	Block_Buffer *buf = 0;
	while (1) {
		if (!block_list_.empty()) {
			buf = block_list_.pop_front();
			ROBOT_MANAGER->push_login_data_block(buf);
		} else {
			//没有数据时候延迟
			Time_Value::sleep(Time_Value(0, 100));
		}
	}
}

////////////////////////////////////////////////////////////////////////

Robot_Gate_Connector::Robot_Gate_Connector(void) { }

Robot_Gate_Connector::~Robot_Gate_Connector(void) { }

Robot_Gate_Connector *Robot_Gate_Connector::instance_ = 0;

Robot_Gate_Connector *Robot_Gate_Connector::instance(void) {
	if (! instance_)
		instance_ = new Robot_Gate_Connector;
	return instance_;
}

void Robot_Gate_Connector::process_list(void) {
	Block_Buffer *buf = 0;
	while (1) {
		if (!block_list_.empty()) {
			buf = block_list_.pop_front();
			ROBOT_MANAGER->push_gate_data_block(buf);
		} else {
			//没有数据时候延迟
			Time_Value::sleep(Time_Value(0, 100));
		}
	}
}
