/*
* Robot_Manager.h
*
 *  Created on: Dec 24, 2015
 *      Author: zhangyalei
*/

#ifndef ROBOT_MANAGER_H_
#define ROBOT_MANAGER_H_

#include "Thread.h"
#include "List.h"
#include "Block_List.h"
#include "Object_Pool.h"
#include "Robot.h"
#include "Msg_Struct.h"

class Robot_Manager: public Thread {
public:
	typedef Object_Pool<Block_Buffer, Thread_Mutex> Block_Pool;
	typedef Object_Pool<Robot> Robot_Pool;
	typedef Block_List<Thread_Mutex> Data_List;
	typedef List<int, Thread_Mutex> Tick_List;

	typedef boost::unordered_map<int, Robot *> Cid_Robot_Map;
	typedef boost::unordered_set<int> Cid_Robot_Set;

	typedef boost::unordered_map<int32_t, Base_Struct *> Struct_Id_Map;
	typedef boost::unordered_map<std::string, Base_Struct *> Struct_Name_Map;

public:
	static Robot_Manager *instance(void);
	static void destroy(void);
	void run_handler(void);

	int init(void);
	int load_struct(const char *path);

	int process_list();
	int process_block(Block_Buffer &buf);
	int push_login_data_block(Block_Buffer *buf);
	int push_gate_data_block(Block_Buffer *buf);
	int push_tick(int tick_v);

	Robot *pop_robot_player(void);
	void push_robot_player(Robot *robot);

	int tick(void);
	int login_tick(Time_Value &now);
	int robot_tick(Time_Value &now);

	Robot *connect_login(char *account = NULL);
	int connect_gate(int login_cid, std::string& ip, int port, std::string& session, std::string& account);

	int send_to_login(int cid, Block_Buffer &buf);
	int send_to_gate(int cid, Block_Buffer &buf);

	Robot* login_robot(int cid);
	Robot* gate_robot(int cid);

	inline Time_Value &server_tick() { return server_tick_; };
	inline std::string &login_ip() { return login_ip_; }
	inline int login_port() { return login_port_; }
	inline std::string &gate_ip() { return gate_ip_; }
	inline int gate_port() { return gate_port_; }
	inline int send_msg_interval() { return send_msg_interval_; }
	inline int robot_lifetime() { return robot_lifetime_; }
	inline int robot_mode() { return robot_mode_; }
	inline Struct_Id_Map& msg_struct_id_map() { return msg_struct_id_map_; }
	inline Struct_Name_Map& msg_struct_name_map() { return msg_struct_name_map_; }

private:
	Robot_Manager(void);
	virtual ~Robot_Manager(void);
	int print_report(void);

private:
	static Robot_Manager *instance_;

	std::string login_ip_;		//服务器ip
	int login_port_;					//服务器端口
	std::string gate_ip_;		//服务器ip
	int gate_port_;					//服务器端口

	int robot_count_;					//机器人数量
	int login_interval_;			//机器人登录间隔(毫秒)
	int send_msg_interval_;		//发送数据间隔,单位是毫秒
	int robot_lifetime_;    	//机器人运行时间,单位是秒
	int robot_mode_;					//机器人模式，0是自动化模式，1是交互模式
	int robot_index_;

	Time_Value server_tick_;
	Time_Value first_login_tick_;
	Time_Value last_login_tick_;

	Block_Pool block_pool_;
	Robot_Pool robot_pool_;

	Tick_List tick_list_;
	Data_List login_data_list_;
	Data_List gate_data_list_;

	Cid_Robot_Map cid_login_robot_map_;
	Cid_Robot_Map cid_gate_robot_map_;

	Struct_Id_Map msg_struct_id_map_;
	Struct_Name_Map msg_struct_name_map_;
};

#define ROBOT_MANAGER Robot_Manager::instance()

#endif /* ROBOT_MANAGER_H_ */
