/*
 * main.cpp
 *
 *  Created on: Dec 24, 2015
 *      Author: zhangyalei
 */

#include "Robot_Manual.h"
#include "Robot_Manager.h"
#include "Robot_Timer.h"
#include "Robot_Config.h"
#include "Robot_Connector.h"

int main(int argc, char *argv[]) {
	srand(Time_Value::gettimeofday().sec() + Time_Value::gettimeofday().usec());

	// 配制文件
	ROBOT_CONFIG->load_robot_config();

	// 启动主逻辑线程
	ROBOT_MANAGER->init();
	ROBOT_MANAGER->thr_create();

	// 启动通信线程
	Time_Value send_interval(0, 100);
	ROBOT_LOGIN_CONNECTOR->set(ROBOT_MANAGER->login_ip(), ROBOT_MANAGER->login_port(), send_interval);
	ROBOT_LOGIN_CONNECTOR->init();
	ROBOT_LOGIN_CONNECTOR->start();
	ROBOT_LOGIN_CONNECTOR->thr_create();

	ROBOT_GATE_CONNECTOR->set(ROBOT_MANAGER->gate_ip(), ROBOT_MANAGER->gate_port(), send_interval);
	ROBOT_GATE_CONNECTOR->init();
	ROBOT_GATE_CONNECTOR->start();
	ROBOT_GATE_CONNECTOR->thr_create();

	// 定时器线程
	ROBOT_TIMER->thr_create();

	if(ROBOT_MANAGER->robot_mode()) {
		ROBOT_MANUAL->init();
		ROBOT_MANUAL->run();
	}
	else {
		Epoll_Watcher watcher;
		watcher.loop();
	}

	return 0;
}

