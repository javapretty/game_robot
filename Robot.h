/*
 * Logic_Struct.cpp
 *
 *  Created on: Dec 24, 2015
 *      Author: zhangyalei
 */

#ifndef ROBOT_H_
#define ROBOT_H_

#include "Event_Handler.h"
#include "Public_Struct.h"

class Robot : public Event_Handler {
public:
	Robot(void);
	virtual ~Robot(void);

	void reset(void);
	int tick(Time_Value &now);
	int watcher_tick(int sec, int usec);

	Robot_Info &robot_info(void) { return robot_info_; }

	int get_login_cid(void) { return login_cid_; };
	void set_login_cid(int cid) { login_cid_ = cid; };

	int get_gate_cid(void) { return gate_cid_; };
	void set_gate_cid(int cid) { gate_cid_ = cid; };

	int auto_send_msg();
	int manual_send_msg(Args_Info &args);
	int recv_server_msg(int status, int msg_id, Block_Buffer &buf);

	//client->login
	int connect_login();

	//client->gate
	int connect_gate(std::string& account, std::string& session);
	int send_heartbeat(Time_Value &now);

	//client->game
	int fetch_role_info(const std::string &account);
	int create_role(void);

	//login->client
	int connect_login_res(int status, Block_Buffer &buf);

	//gate->client
	int connect_gate_res(int status, Block_Buffer &buf);

	//game->client
	int fetch_role_info_res(int status, Block_Buffer &buf);
	int create_role_res(int status, Block_Buffer &buf);
	void login_game_success(Robot_Info robot_info);

	void make_message(Block_Buffer &buf, int msg_id);

	inline int32_t get_cost_time_total(){return cost_time_total_;}
	inline int32_t get_msg_count(){return msg_count_;}

	void set_msg_time(int msg_id);
	Time_Value get_msg_time(int msg_id);

private:
	bool login_success_;
	int login_cid_;
	int gate_cid_;

	Time_Value login_tick_;
	Time_Value heart_tick_;
	Time_Value send_tick_;
	Robot_Info robot_info_;
	MSG_Info msg_info_;

	Msg_Cost_Time_Map msg_cost_time_map_;
	int32_t cost_time_total_;
	int32_t msg_count_;
};

inline void Robot::set_msg_time(int msg_id) {
	msg_cost_time_map_[msg_id] = Time_Value::gettimeofday();
}

inline Time_Value Robot::get_msg_time(int msg_id){
	Msg_Cost_Time_Map::iterator iter = msg_cost_time_map_.find(msg_id - 400000);
	if(iter != msg_cost_time_map_.end()){
		return iter->second;
	}
	return Time_Value::zero;
}

#endif /* ROBOT_H_ */
