/*
 * Robot_Config.cpp
 *
 *  Created on: 2016年3月1日
 *      Author: zhangyalei
 */

#include "Robot_Config.h"

Robot_Config::Robot_Config(void) { }

Robot_Config::~Robot_Config(void) { }

Robot_Config * Robot_Config::instance_;

Robot_Config *Robot_Config::instance() {
	if (! instance_)
		instance_ = new Robot_Config;
	return instance_;
}

void Robot_Config::load_robot_config(void) {
	load_json_file("config/robot/robot_config.json", robot_config_.robot_config.get_cur_json());
	load_json_file("config/robot/robot_msg.json", robot_config_.robot_msg.get_cur_json());
}

const Json::Value &Robot_Config::robot_config(void) {
	return get_json_value_with_rlock(robot_config_.lock, robot_config_.robot_config);
}

const Json::Value &Robot_Config::robot_msg(void) {
	return get_json_value_with_rlock(robot_config_.lock, robot_config_.robot_msg);
}
