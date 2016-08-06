/*
 * Public_Struct.h
 *
 *  Created on: 2016年8月6日
 *      Author: zhangyalei
 */

#ifndef PUBLIC_STRUCT_H_
#define PUBLIC_STRUCT_H_

#include "boost/unordered_map.hpp"
#include <vector>
#include "Time_Value.h"

typedef boost::unordered_map<int32_t, Time_Value> Msg_Cost_Time_Map;

enum Message {
	REQ_CLIENT_REGISTER 	= 100000, 	//注册
	REQ_CLIENT_LOGIN 			= 100001, 	//登录login
	REQ_CONNECT_GATE 			= 100002, 	//登录gate
	REQ_HEARTBEAT 				= 100003, 	//心跳
	RES_CLIENT_REGISTER 	= 500000, 	//注册(返回)
	RES_CLIENT_LOGIN 			= 500001, 	//登录login(返回)
	RES_CONNECT_GATE 			= 500002, 	//登录gate(返回)
	RES_HEARTBEAT 				= 500003, 	//心跳(返回)

	REQ_FETCH_ROLE_INFO 	= 120001, 	//获取角色信息
	REQ_CREATE_ROLE 			=	120002, 	//创建角色
	RES_FETCH_ROLE_INFO 	= 520001, 	//获取角色信息(返回)
	RES_CREATE_ROLE 			= 520002, 	//创建角色(返回)
};

struct Field_Info {
	std::string field_label;
	std::string field_type;
	std::string field_name;
	std::string key_type;
	std::string key_name;
};

struct Args_Info {
	int msg_id;
	std::vector<int> args_list;
	int cursor[7];
	 //类型说明：1(int8_t) 2(int16_t) 3(int32_t) 4(int64_t) 5(bool) 6(double) 7(string)
	std::vector<int8_t> int8_args;
	std::vector<int16_t> int16_args;
	std::vector<int32_t> int32_args;
	std::vector<int64_t> int64_args;
	std::vector<bool> bool_args;
	std::vector<double> double_args;
	std::vector<std::string> string_args;
};

struct Robot_Info {
	int64_t role_id;
	std::string role_name;
	std::string account;
	int32_t level;
	int32_t exp;
	int8_t gender;
	int8_t career;

	void reset(void) {
		role_id = 0;
		role_name.clear();
		account.clear();
		level = 0;
		exp = 0;
		gender = 0;
		career = 0;
	}
};

struct MSG_Info {
	bool is_inited;
	int cid;
	int hash_key;				/// 用于加解密的hash key
	uint32_t msg_serial;				/// 上一条消息序号
	Time_Value msg_timestamp;			/// 上一条消息时间戳
	uint32_t msg_interval_count_;		/// 操作频率统计
	Time_Value msg_interval_timestamp;

	void reset(void) {
		is_inited = false;
		cid = -1;
		hash_key = 0;
		msg_serial = 0;
		msg_timestamp = Time_Value::zero;
		msg_interval_count_ = 0;
		msg_interval_timestamp = Time_Value::zero;
	}
};

#endif /* PUBLIC_STRUCT_H_ */
