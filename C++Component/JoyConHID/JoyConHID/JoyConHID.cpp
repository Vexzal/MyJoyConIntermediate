// JoyConHID.cpp : Defines the exported functions for the DLL application.
//
//#include <bitset>

//#include<chrono>

#include "stdafx.h"
#include <vector>
#include <hidapi.h>
#include "Joycon.h"
#define JOYCON_VENDOR 0x057e
#define JOYCON_L_BT 0x2006
#define JOYCON_R_BT 0x2007
#define PRO_CONTROLLER 0x2009
#define JOYCON_CHARGING_GRIP 0x200e
#define SERIAL_LEN 18
#define PI 3.14159265359
#define L_OR_R(lr) (lr == 1 ? 'L' : (lr == 2 ? 'R' : '?'))

std::vector<Joycon> joycons;

unsigned char buf[65];
int res = 0;
//lets just make sure all the junk that actually grabs the device works

void grabDevices();

void handle_input(Joycon *jc, uint8_t *packet, int length) {
	//bluetooth button pressed packet:
	if (packet[0] == 0x3F) {
		uint16_t old_buttons = jc->buttons;
		int8_t old_dstick = jc->dstick;

		jc->dstick = packet[3];
	}

	//input update packet:
	//0x21 is just bottons, 0x30 includs gyro, 0x31 includs nfc (large packet size)
	if (packet[0] == 0x21 || packet[0] == 0x30 || packet[0] == 0x31) {
		int offset = jc->bluetooth ? 0 : 10;

		uint8_t *btn_data = packet + offset + 3;
		{
			uint16_t states = 0;
			uint16_t states2 = 0;

			if (jc->left_right == 1) {
				states = (btn_data[1] << 8) | (btn_data[2] & 0xFF);
			}
			else if (jc->left_right == 2) {
				states = (btn_data[1] << 8) | (btn_data[0] & 0xFF);
			}

			jc->buttons = states;
		}

		//get stick data:
		uint8_t *stick_data = packet + offset;
		if (jc->left_right == 1) {
			stick_data += 6;
		}
		else if (jc->left_right == 2) {
			stick_data += 9;
		}

		uint16_t stick_x = stick_data[0] | ((stick_data[1] & 0xF) << 8);
		uint16_t stick_y = (stick_data[1] >> 4) | (stick_data[2] << 4);

		jc->stick.x = stick_x;
		jc->stick.y = stick_y;

		//use calibration data:
		jc->CalcAnalogStick();

		jc->battery = (stick_data[1] & 0xF0) >> 4;

		//accelerometer:
		//data is absolute (m/s^2) 
		{
			//get x
			jc->accel.x = (float)uint16_to_int16(packet[13] | (packet[14] << 8) & 0xff00) * jc->acc_cal_coeff[0];
			//get y
			jc->accel.y = (float)uint16_to_int16(packet[15] | (packet[16] << 8) & 0xff00) * jc->acc_cal_coeff[1];
			//get z
			jc->accel.z = (float)uint16_to_int16(packet[17] | (packet[18] << 8) & 0xff00) * jc->acc_cal_coeff[2];
		}

		//gyroscope:
		//data is relativ(rad/s)
		{
			//get roll:
			jc->gyro.roll = (float)((uint16_to_int16(packet[19] | (packet[20] << 8) & 0xFF00)) - jc->sensor_cal[1][0]) * jc->gyro_cal_coeff[0];

			// get pitch:
			jc->gyro.pitch = (float)((uint16_to_int16(packet[21] | (packet[22] << 8) & 0xFF00)) - jc->sensor_cal[1][1]) * jc->gyro_cal_coeff[1];

			// get yaw:
			jc->gyro.yaw = (float)((uint16_to_int16(packet[23] | (packet[24] << 8) & 0xFF00)) - jc->sensor_cal[1][2]) * jc->gyro_cal_coeff[2];
		}

		//offsets
		{
			jc->setGyroOffsets();
			jc->gyro.roll -= jc->gyro.offset.roll;
			jc->gyro.pitch -= jc->gyro.offset.pitch;
			jc->gyro.yaw -= jc->gyro.offset.yaw;
		}

	}
	//handle button combos:
	{
		if (jc->left_right == 1) {
			jc->btns.down = (jc->buttons &(1 << 0)) ? 1 : 0;
			jc->btns.up = (jc->buttons & (1 << 1)) ? 1 : 0;
			jc->btns.right = (jc->buttons & (1 << 2)) ? 1 : 0;
			jc->btns.left = (jc->buttons & (1 << 3)) ? 1 : 0;
			jc->btns.sr = (jc->buttons & (1 << 4)) ? 1 : 0;
			jc->btns.sl = (jc->buttons & (1 << 5)) ? 1 : 0;
			jc->btns.l = (jc->buttons & (1 << 6)) ? 1 : 0;
			jc->btns.zl = (jc->buttons & (1 << 7)) ? 1 : 0;
			jc->btns.minus = (jc->buttons & (1 << 8)) ? 1 : 0;
			jc->btns.stick_button = (jc->buttons & (1 << 11)) ? 1 : 0;
			jc->btns.capture = (jc->buttons & (1 << 13)) ? 1 : 0;
		}
		if (jc->left_right == 2) {
			jc->btns.y = (jc->buttons & (1 << 0)) ? 1 : 0;
			jc->btns.x = (jc->buttons & (1 << 1)) ? 1 : 0;
			jc->btns.b = (jc->buttons & (1 << 2)) ? 1 : 0;
			jc->btns.a = (jc->buttons & (1 << 3)) ? 1 : 0;
			jc->btns.sr = (jc->buttons & (1 << 4)) ? 1 : 0;
			jc->btns.sl = (jc->buttons & (1 << 5)) ? 1 : 0;
			jc->btns.r = (jc->buttons & (1 << 6)) ? 1 : 0;
			jc->btns.zr = (jc->buttons & (1 << 7)) ? 1 : 0;
			jc->btns.plus = (jc->buttons & (1 << 9)) ? 1 : 0;
			jc->btns.stick_button = (jc->buttons & (1 << 10)) ? 1 : 0;
			jc->btns.home = (jc->buttons & (1 << 12)) ? 1 : 0;
		}

	}
}


void grabDevics()
{
	int read;
	int written;
	const char *device_name;

	struct hid_device_info *devs, *cur_dev;

	res = hid_init();

	devs = hid_enumerate(JOYCON_VENDOR, 0x0);
	cur_dev = devs;
	while (cur_dev) {
		if (cur_dev->vendor_id == JOYCON_VENDOR) {
			if (cur_dev->product_id == JOYCON_L_BT || cur_dev->product_id == JOYCON_R_BT)
			{
				Joycon jc = Joycon(cur_dev);
				joycons.push_back(jc);
			}
			
		}
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);

	//init joycons
	for (int i = 0; i < joycons.size(); i++)
	{
		joycons[i].init_bt();
	}

	//set lights: - move to its own function
	for (int r = 0; r < 5; r++)
	{
		for (int i = 0; i < joycons.size(); i++)
		{
			Joycon *jc = &joycons[i];
			memset(buf, 0x00, 0x40);
			if (i == 0)
				buf[0] = 0x0 | 0x0 | 0x0 | 0x1; //solid 1
			if (i == 1)
				buf[0] = 0x0 | 0x0 | 0x2 | 0x0;//solid 2


			jc->send_subcommand(0x01, 0x30, buf, 1);
		}
	}

	//give a small rumble
	

	

}
