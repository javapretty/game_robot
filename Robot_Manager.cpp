/*
* Robot_Manager.cpp
*
*  Created on: Jun 18, 2012
*      Author: zhangyalei
*/

#include <sstream>
#include "Log.h"
#include "Robot_Connector.h"
#include "Robot_Manager.h"
#include "Robot_Config.h"
#include "Msg_Struct.h"

Robot_Manager::Robot_Manager(void) :
	login_port_(0),
	gate_port_(0),
	robot_count_(0),
	login_interval_(1),
	send_msg_interval_(1),
	robot_lifetime_(1),
	robot_mode_(0),
	robot_index_(0),
	server_tick_(Time_Value::zero),
	first_login_tick_(Time_Value::zero),
  last_login_tick_(Time_Value::zero)
{ }

Robot_Manager::~Robot_Manager(void) { }

Robot_Manager *Robot_Manager::instance_ = 0;

Robot_Manager *Robot_Manager::instance(void) {
	if (! instance_)
		instance_ = new Robot_Manager;
	return instance_;
}

void Robot_Manager::destroy(void) {
	if (instance_) {
		delete instance_;
		instance_ = 0;
	}
}

Robot *Robot_Manager::pop_robot_player(void) {
	return robot_pool_.pop();
}

void Robot_Manager::push_robot_player(Robot *robot) {
	robot_pool_.push(robot);
}

void Robot_Manager::run_handler(void) {
	process_list();
}

int Robot_Manager::init(void) {
	const Json::Value &robot_config = ROBOT_CONFIG->robot_config();
	login_ip_ = robot_config["login_ip"].asString();
	login_port_ = robot_config["login_port"].asInt();
	gate_ip_ = robot_config["gate_ip"].asString();
	gate_port_ = robot_config["gate_port"].asInt();
	robot_count_ = robot_config["robot_count"].asInt();
	login_interval_ = robot_config["login_interval"].asInt();
	send_msg_interval_ = robot_config["send_msg_interval_"].asInt();
	robot_lifetime_ = robot_config["robot_lifetime"].asInt();
	robot_mode_ = robot_config["robot_mode"].asInt();
	robot_index_ = 0;

	load_struct("config/game_struct.xml");
	load_struct("config/msg_struct.xml");
	return 0;
}

int Robot_Manager::load_struct(const char *path){
	Xml xml;
	xml.load_xml(path);
	TiXmlNode *node = xml.get_root_node();
	XML_LOOP_BEGIN(node)
		Base_Struct *base_struct = new Msg_Struct(xml, node);
		if(base_struct->msg_id() > 0) {
			msg_struct_id_map_.insert(std::pair<int32_t, Base_Struct*>(base_struct->msg_id(), base_struct));
		}
		msg_struct_name_map_.insert(std::pair<std::string, Base_Struct*>(base_struct->struct_name(), base_struct));
	XML_LOOP_END(node)
	return 0;
}

int Robot_Manager::process_list(void) {
	int32_t cid = 0;
	Block_Buffer *buf = 0;

	while (1) {
		bool all_empty = true;
		if ((buf = login_data_list_.pop_front()) != 0) {
			all_empty = false;
			buf->peek_int32(cid);
			process_block(*buf);
			ROBOT_LOGIN_CONNECTOR->push_block(cid, buf);
		}
		if ((buf = gate_data_list_.pop_front()) != 0) {
			all_empty = false;
			buf->peek_int32(cid);
			process_block(*buf);
			ROBOT_GATE_CONNECTOR->push_block(cid, buf);
		}
		if (!tick_list_.empty()) {
			all_empty = false;
			tick_list_.pop_front();
			tick();
		}
		if (all_empty) {
			Time_Value::sleep(Time_Value(0, 100));
		}
	}

	return 0;
}

int Robot_Manager::process_block(Block_Buffer &buf) {
	int32_t cid = 0;
	int16_t len = 0;
	int32_t msg_id = 0;
	int32_t status = 0;
	buf.read_int32(cid);
	buf.read_int16(len);
	buf.read_int32(msg_id);
	buf.read_int32(status);

	Robot *robot = nullptr;
	if (msg_id == RES_CONNECT_LOGIN) {
		robot = login_robot(cid);
	} else {
		robot = gate_robot(cid);
	}
	if (robot == nullptr) {
		return -1;
	}

	switch (msg_id) {
	case RES_CONNECT_LOGIN:{
		robot->connect_login_res(status, buf);
		break;
	}
	case RES_CONNECT_GATE: {
		robot->connect_gate_res(status, buf);
		break;
	}
	case RES_FETCH_ROLE_INFO: {
		robot->fetch_role_info_res(status, buf);
		break;
	}
	case RES_CREATE_ROLE: {
		robot->create_role_res(status, buf);
		break;
	}
	default: {
		robot->recv_server_msg(status, msg_id, buf);
		break;
	}
	}

	return 0;
}

int Robot_Manager::push_login_data_block(Block_Buffer *buf) {
	login_data_list_.push_back(buf);
	return 0;
}

int Robot_Manager::push_gate_data_block(Block_Buffer *buf) {
	gate_data_list_.push_back(buf);
	return 0;
}

int Robot_Manager::push_tick(int tick_v) {
	tick_list_.push_back(tick_v);
	return 0;
}

int Robot_Manager::tick(void) {
	Time_Value now(Time_Value::gettimeofday());
	server_tick_ = now;

	if(!robot_mode_){
		login_tick(now);
	}
	robot_tick(now);

	//if(now - first_login_tick_ >= Time_Value(run_time_, 0)){
	//print_report();
	//exit(0);
	//}

	return 0;
}

int Robot_Manager::login_tick(Time_Value &now) {
	if (robot_index_ >= robot_count_) {
		return 0;
	}

	if (now - last_login_tick_ > Time_Value(0, login_interval_ * 1000)) {
		last_login_tick_ = now;
		if (robot_index_ == 0) {
			first_login_tick_ = now;
		}

		connect_login();
	}

	return 0;
}

int Robot_Manager::robot_tick(Time_Value &now) {
	for (Cid_Robot_Map::iterator it = cid_gate_robot_map_.begin(); it != cid_gate_robot_map_.end(); ++it) {
		it->second->tick(now);
	}

	return 0;
}

Robot *Robot_Manager::connect_login(char *account) {
	int login_cid = ROBOT_LOGIN_CONNECTOR->connect_server();
	if (login_cid < 2) {
		LOG_ERROR("login_cid = %d", login_cid);
		return NULL;
	}

	Robot *robot = robot_pool_.pop();
	if (! robot) {
		LOG_FATAL("robot_pool_.pop return 0");
	}
	robot->reset();

	if (! cid_login_robot_map_.insert(std::make_pair(login_cid, robot)).second) {
		Cid_Robot_Map::iterator it = cid_login_robot_map_.find(login_cid);
		if (it != cid_login_robot_map_.end()) {
			LOG_ERROR("insert failure replace login_cid:%d", login_cid);
		}
		return NULL;
	}

	robot->set_login_cid(login_cid);
	if(account == NULL){
		std::stringstream account_stream;
		int rand_num = random() % 1000;
		account_stream << "robot" << robot_index_++ << "_" << rand_num;
		robot->robot_info().account = account_stream.str();
	}
	else{
		robot->robot_info().account = account;
	}
	robot->connect_login();
	return robot;
}

int Robot_Manager::connect_gate(int login_cid, std::string& ip, int port, std::string& session, std::string& account) {
	Time_Value send_interval(0, 100);
	ROBOT_GATE_CONNECTOR->set(ip, port, send_interval);
	int gate_cid = ROBOT_GATE_CONNECTOR->connect_server();
	if (gate_cid < 2) {
		LOG_ERROR("gate_cid = %d", gate_cid);
		return -1;
	}

	Cid_Robot_Map::iterator robot_iter = cid_login_robot_map_.find(login_cid);
	if (robot_iter == cid_login_robot_map_.end()) {
		LOG_ERROR("cannot find login_cid = %d robot", login_cid);
		return -1;
	}

	Robot *robot = robot_iter->second;
	if (! cid_gate_robot_map_.insert(std::make_pair(gate_cid, robot)).second) {
		Cid_Robot_Map::iterator it = cid_gate_robot_map_.find(gate_cid);
		if (it != cid_gate_robot_map_.end()) {
			LOG_ERROR("insert failure replace gate_cid:%d", gate_cid);
		}
		return 0;
	}

	robot->set_gate_cid(gate_cid);
	robot->robot_info().account = account;
	robot->connect_gate(robot->robot_info().account, session);

	return 0;
}

int Robot_Manager::send_to_login(int cid, Block_Buffer &buf)  {
	if (cid < 2) {
		LOG_ERROR("cid = %d", cid);
		return -1;
	}
	return ROBOT_LOGIN_CONNECTOR->send_block(cid, buf);
}

int Robot_Manager::send_to_gate(int cid, Block_Buffer &buf)  {
	if (cid < 2) {
		LOG_ERROR("cid = %d", cid);
		return -1;
	}
	return ROBOT_GATE_CONNECTOR->send_block(cid, buf);
}

Robot* Robot_Manager::login_robot(int cid) {
	Cid_Robot_Map::iterator robot_iter = cid_login_robot_map_.find(cid);
	if (robot_iter == cid_login_robot_map_.end()) {
		LOG_ERROR("cannot find login_cid = %d robot", cid);
		return nullptr;
	}

	return robot_iter->second;
}

Robot* Robot_Manager::gate_robot(int cid) {
	Cid_Robot_Map::iterator robot_iter = cid_gate_robot_map_.find(cid);
	if (robot_iter == cid_gate_robot_map_.end()) {
		LOG_ERROR("cannot find gate_cid = %d robot", cid);
		return nullptr;
	}

	return robot_iter->second;
}

int Robot_Manager::print_report(void) {
	uint64_t cost_time = 0;
	uint64_t msg_count = 0;
	FILE *fp = fopen("./report.txt", "ab+");
	for(Cid_Robot_Map::iterator iter = cid_gate_robot_map_.begin();
			iter != cid_gate_robot_map_.end(); iter++){
		cost_time += (iter->second)->get_cost_time_total();
		msg_count += (iter->second)->get_msg_count();
	}
	char temp[512] = {};
	sprintf(temp, "%ld player login, %lu msg recv, average cost time: %lu\n",
			cid_gate_robot_map_.size(), msg_count, cost_time / msg_count);
	fputs(temp, fp);

	return 0;
}
