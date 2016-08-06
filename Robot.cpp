/*
 * Logic_Struct.cpp
 *
 *  Created on: Dec 24, 2015
 *      Author: zhangyalei
 */

#include <sstream>
#include "Common_Func.h"
#include "Log.h"
#include "Robot.h"
#include "Robot_Manager.h"
#include "Robot_Config.h"
#include "Robot_Timer.h"
#include "Robot_Msg.h"

Robot::Robot(void):login_success_(false), login_cid_(0), gate_cid_(0), cost_time_total_(0), msg_count_(0) { }

Robot::~Robot(void) { }

void Robot::reset(void) { 
	login_success_ = false;
	login_cid_ = 0;
	gate_cid_ = 0;
	login_tick_ = Time_Value::zero;
	heart_tick_ = Time_Value::zero;
	send_tick_ = Time_Value::zero;
	robot_info_.reset();
	msg_info_.reset();
};

int Robot::tick(Time_Value &now) {
	if (login_success_) {
		// 心跳
		if (heart_tick_ < now) {
			send_heartbeat(now);
			heart_tick_ = now + Time_Value(30, 0);
		}

		if(ROBOT_MANAGER->robot_mode() == 0){
			if(now - send_tick_ >= Time_Value(0, ROBOT_MANAGER->send_msg_interval() * 1000)){
				//发送机器人消息到服务器
				//send_robot_msg();
				send_tick_ = now;
			}
		}
	}
	return 0;
}

int Robot::watcher_tick(int sec, int usec) {
	Time_Value time_tick(sec, usec);
	ROBOT_TIMER->watcher().add(this, Epoll_Watcher::EVENT_TIMEOUT, &time_tick);
	return 0;
}

int Robot::auto_send_msg() {
	const Json::Value &robot_msg = ROBOT_CONFIG->robot_msg();
	for (Json::Value::iterator iter = robot_msg.begin(); iter != robot_msg.end(); ++iter) {
		int msg_id = (*iter)["msg_id"].asInt();
		set_msg_time(msg_id);
		Block_Buffer buf;
		make_message(buf, msg_id);
		const Json::Value &msg_param = (*iter)["msg_param"];
		for (uint i = 0; i < msg_param.size(); ++i) {
			int param_type = msg_param[i][(uint)0].asInt();
			switch(param_type)
			{
			case 1:
				buf.write_int8(msg_param[i][1].asInt());
				break;
			case 2:
				buf.write_int16(msg_param[i][1].asInt());
				break;
			case 3:
				buf.write_int32(msg_param[i][1].asInt());
				break;
			case 4:
				buf.write_int64(msg_param[i][1].asInt());
				break;
			case 5:
				buf.write_bool(msg_param[i][1].asBool());
				break;
			case 6:
				buf.write_double(msg_param[i][1].asDouble());
				break;
			case 7:
				buf.write_string(msg_param[i][1].asString());
				break;
			default:
				break;
			}
		}
		buf.finish_message();
		ROBOT_MANAGER->send_to_gate(gate_cid_, buf);
	}
	return 0;
}

int Robot::manual_send_msg(Args_Info &args) {
	int msg_id = args.msg_id;
	set_msg_time(msg_id);
	Block_Buffer buf;
	make_message(buf, msg_id);
	for (std::vector<int>::iterator iter = args.args_list.begin(); iter != args.args_list.end(); ++iter) {
		switch(*iter)
		{
		case 1:
			buf.write_int8(args.int8_args[args.cursor[0]++]);
			break;
		case 2:
			buf.write_int16(args.int16_args[args.cursor[1]++]);
			break;
		case 3:
			buf.write_int32(args.int32_args[args.cursor[2]++]);
			break;
		case 4:
			buf.write_int64(args.int64_args[args.cursor[3]++]);
			break;
		case 5:
			buf.write_bool(args.bool_args[args.cursor[4]++]);
			break;
		case 6:
			buf.write_double(args.double_args[args.cursor[5]++]);
			break;
		case 7:
			buf.write_string(args.string_args[args.cursor[6]++]);
			break;
		default:
			break;
		}
	}
	buf.finish_message();
	ROBOT_MANAGER->send_to_gate(gate_cid_, buf);
	return 0;
}

int Robot::recv_server_msg(int status, int msg_id, Block_Buffer &buf) {
	Time_Value now = Time_Value::gettimeofday();
	int cost_msec = now.msec() - get_msg_time(msg_id).msec();
	cost_time_total_ += cost_msec;
	msg_count_++;
	if (status == 0) {
		LOG_INFO("account:%s, msg_id:%d success, cost_msec: %d", robot_info_.account.c_str(), msg_id, cost_msec);
		Robot_Manager::Struct_Id_Map::iterator iter = ROBOT_MANAGER->msg_struct_id_map().find(msg_id);
		if (iter != ROBOT_MANAGER->msg_struct_id_map().end()) {
			Msg_Struct *msg_struct = dynamic_cast<Msg_Struct*>(iter->second);
			if (msg_struct) {
				msg_struct->print_msg(buf);
			}
		}
	} else {
		LOG_ERROR("account:%s, msg_id:%d fail, status:%d cost_msec: %d", robot_info_.account.c_str(), msg_id, status, cost_msec);
	}

	return 0;
}

int Robot::client_register() {
	MSG_100000 msg;
	msg.reset();
	msg.account = robot_info_.account;
	msg.password = robot_info_.account;

	Block_Buffer buf;
	make_message(buf, REQ_CLIENT_REGISTER);
	msg.serialize(buf);
	buf.finish_message();
	ROBOT_MANAGER->send_to_login(login_cid_, buf);

	login_tick_ = Time_Value::gettimeofday();
	return 0;
}

int Robot::client_login() {
	MSG_100001 msg;
	msg.reset();
	msg.account = robot_info_.account;
	msg.password = robot_info_.account;

	Block_Buffer buf;
	make_message(buf, REQ_CLIENT_LOGIN);
	msg.serialize(buf);
	buf.finish_message();
	ROBOT_MANAGER->send_to_login(login_cid_, buf);
	return 0;
}

int Robot::connect_gate(std::string& account, std::string& session) {
	MSG_100002 msg;
	msg.reset();
	msg.account = account;
	msg.session = session;

	Block_Buffer buf;
	make_message(buf, REQ_CONNECT_GATE);
	msg.serialize(buf);
	buf.finish_message();
	ROBOT_MANAGER->send_to_gate(gate_cid_, buf);
	return 0;
}

int Robot::send_heartbeat(Time_Value &now) {
	MSG_100003 msg;
	msg.reset();
	msg.client_time = now.sec();

	Block_Buffer buf;
	make_message(buf, REQ_HEARTBEAT);
	msg.serialize(buf);
	buf.finish_message();
	ROBOT_MANAGER->send_to_gate(gate_cid_, buf);
	set_msg_time(REQ_HEARTBEAT);
	return 0;
}

int Robot::fetch_role_info(const std::string &account) {
	Block_Buffer buf;
	make_message(buf, REQ_FETCH_ROLE_INFO);
	buf.write_string(account);
	buf.finish_message();
	ROBOT_MANAGER->send_to_gate(gate_cid_, buf);
	return 0;
}

int Robot::create_role(void) {
	Block_Buffer buf;
	make_message(buf, REQ_CREATE_ROLE);
	buf.write_string(robot_info_.account);
	buf.write_string(robot_info_.role_name);
	buf.write_int8(rand() % 2);
	buf.write_int8(rand() % 4);
	buf.finish_message();
	ROBOT_MANAGER->send_to_gate(gate_cid_, buf);
	return 0;
}

int Robot::client_register_res(int status, Block_Buffer &buf) {
	int login_msec = Time_Value::gettimeofday().msec() - login_tick_.msec();
	if (status == 0) {
		MSG_500000 msg;
		msg.deserialize(buf);
		LOG_INFO("register success, gate_ip:%s, gate_port:%d, session:%s, account:%s, login_msec:%d",
				msg.ip.c_str(), msg.port, msg.session.c_str(), robot_info_.account.c_str(), login_msec);
		ROBOT_MANAGER->connect_gate(login_cid_, msg.ip, msg.port, msg.session, robot_info_.account) ;
	} else {
		LOG_ERROR("client register fail, account exist, status:%d, login_msec:%d", status, login_msec);
	}

	return 0;
}

int Robot::client_login_res(int status, Block_Buffer &buf) {
	int login_msec = Time_Value::gettimeofday().msec() - login_tick_.msec();
	if (status == 0) {
		MSG_500001 msg;
		msg.deserialize(buf);
		LOG_INFO("login success, gate_ip:%s, gate_port:%d, session:%s, account:%s, login_msec:%d",
				msg.ip.c_str(), msg.port, msg.session.c_str(), robot_info_.account.c_str(), login_msec);
		ROBOT_MANAGER->connect_gate(login_cid_, msg.ip, msg.port, msg.session, robot_info_.account) ;
	} else {
		LOG_ERROR("client login fail, account exist, status:%d, login_msec:%d", status, login_msec);
	}

	return 0;
}

int Robot::connect_gate_res(int status, Block_Buffer &buf) {
	if (status == 0) {
		MSG_500002 msg;
		msg.deserialize(buf);
		int login_msec = Time_Value::gettimeofday().msec() - login_tick_.msec();
		LOG_INFO("connect gate success, gate_cid = %d, account = %s, login_msec = %d", gate_cid_, robot_info_.account.c_str(), login_msec);

		fetch_role_info(msg.account);
	} else {
		LOG_ERROR("connect gate fail,repeat connect,please connect after 2s, status = %d", status);
	}
	return 0;
}

int Robot::fetch_role_info_res(int status, Block_Buffer &buf) {
	if (status == 0) {
		Robot_Info robot_info;
		robot_info.role_id = buf.read_int64();
		robot_info.account = buf.read_string();
		robot_info.role_name = buf.read_string();
		robot_info.level = buf.read_int32();
		robot_info.exp = buf.read_int32();
		robot_info.gender = buf.read_int8();
		robot_info.career = buf.read_int8();
		login_game_success(robot_info);
	} else {	/// 角色不存在, 创建角色
		int login_msec = Time_Value::gettimeofday().msec() - login_tick_.msec();
		LOG_ERROR("role not exist, need create role, status = %d, gate_cid = %d, account = %s, login_msec = %d", status, gate_cid_, robot_info_.account.c_str(), login_msec);
		robot_info_.role_name = robot_info_.account;
		create_role();
	}

	return 0;
}

int Robot::create_role_res(int status, Block_Buffer &buf) {
	int login_msec = Time_Value::gettimeofday().msec() - login_tick_.msec();
	if (status == 0) {
		login_success_ = true;
		int64_t role_id = buf.read_int64();
		LOG_INFO("create role success, account:%s, role_id:%ld, login_msec:%d", robot_info_.account.c_str(), role_id, login_msec);
	} else {
		LOG_ERROR("create role fail, status:%d, account:%s, login_msec:%d", status, robot_info_.account.c_str(), login_msec);
	}
	return 0;
}

void Robot::login_game_success(Robot_Info robot_info) {
	Time_Value now = Time_Value::gettimeofday();
	int login_msec = now.msec() - login_tick_.msec();
	Date_Time date(now);
	LOG_INFO("%ld:%ld:%ld login game success account:%s gate_cid = %d login_sec = %d level=%d", date.hour(), date.minute(), date.second()
			, robot_info_.account.c_str(), gate_cid_, login_msec, robot_info.level);
	robot_info_ = robot_info;
	login_success_ = true;

	auto_send_msg();
}

void Robot::make_message(Block_Buffer &buf, int msg_id) {
	if (! msg_info_.is_inited) {
		msg_info_.hash_key = elf_hash(robot_info_.account.c_str(), robot_info_.account.length());
	}

	uint32_t msg_time = Time_Value::gettimeofday().sec();
	uint32_t serial_cipher = msg_info_.msg_serial ^ msg_info_.hash_key;
	uint32_t msg_time_cipher = msg_time ^ serial_cipher;

	if (! msg_info_.is_inited) {
		msg_info_.is_inited = true;
		msg_info_.msg_timestamp = ROBOT_MANAGER->server_tick();
		msg_info_.msg_interval_count_ = 1;
		msg_info_.msg_interval_timestamp = ROBOT_MANAGER->server_tick();
	}

	msg_info_.msg_serial++;

	buf.make_client_message(serial_cipher, msg_time_cipher, msg_id, 0);
}
