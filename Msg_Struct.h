/*
 * Msg_Struct.h
 *
 *  Created on: 2016年8月2日
 *      Author: zhangyalei
 */

#ifndef MSG_STRUCT_H_
#define MSG_STRUCT_H_

#include "Base_Struct.h"

class Msg_Struct: public Base_Struct {
public:
	Msg_Struct(Xml &xml, TiXmlNode *node);
	virtual ~Msg_Struct();

	void print_msg(Block_Buffer &buffer);

private:
	void print_msg_arg(const Field_Info &field_info, Block_Buffer &buffer);
	void print_msg_vector(const Field_Info &field_info, Block_Buffer &buffer);
	void print_msg_struct(const Field_Info &field_info, Block_Buffer &buffer);
};

#endif /* MSG_STRUCT_H_ */
