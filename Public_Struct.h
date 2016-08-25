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
#include "Block_Buffer.h"

typedef boost::unordered_map<int32_t, Time_Value> Msg_Cost_Time_Map;

enum Message {
	REQ_CONNECT_LOGIN 		= 100001, 	//客户端登录login
	REQ_CONNECT_GATE 		= 100101, 	//客户端登录gate
	REQ_SEND_HEARTBEAT	 	= 100102, 	//发送心跳到gate

	RES_CONNECT_LOGIN 		= 500001, 	//客户端登录login(返回)
	RES_CONNECT_GATE 		= 500101, 	//客户端登录gate(返回)
	RES_SEND_HEARTBEAT 	= 500102, 	//发送心跳到gate(返回)

	REQ_FETCH_ROLE_INFO 	= 120001, 	//获取角色信息
	REQ_CREATE_ROLE 			=	120002, 	//创建角色

	RES_FETCH_ROLE_INFO 	= 520001, 	//获取角色信息(返回)
	RES_CREATE_ROLE 			= 520002, 	//创建角色(返回)
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

	int deserialize(Block_Buffer &buffer) {
		buffer.read_int64(role_id);
		buffer.read_string(role_name);
		buffer.read_string(account);
		buffer.read_int32(level);
		buffer.read_int32(exp);
		buffer.read_int8(gender);
		buffer.read_int8(career);
		return 0;
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
