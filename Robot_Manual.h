/*
 * User_Interface.h
 *
 *  Created on: Mar 19, 2016
 *      Author: lijunliang
 */

#ifndef _ROBOT_MANUAL_H_
#define _ROBOT_MANUAL_H_

#include "Robot.h"

class Robot_Manual {
public:
	static Robot_Manual *instance();
	int init();
	void run();

	int get_cmd();
	int get_args();
	int fill_args(int type);
	void clear();

private:
	Robot_Manual();
	~Robot_Manual();

private:
	static Robot_Manual *instance_;
	Args_Info args_;
	Robot *robot_;
};

#define ROBOT_MANUAL Robot_Manual::instance()

#endif
