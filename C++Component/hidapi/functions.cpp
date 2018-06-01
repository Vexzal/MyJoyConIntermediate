#pragma once


#include <stdint.h>
#include "hidapi.h"
#include "tools.h"
#define EXPORT __declspec(dllexport)

#define JOYCON_VENDOR 0x057e
#define JOYCON_L_BT 0x2006
#define JOYCON_R_BT 0x2007
#define PRO_CONTROLLER 0x2009
#define JOYCON_CHARGING_GRIP 0x200e
#define SERIAL_LEN 18
#define PI 3.14159265359
#define L_OR_R(lr) (lr == 1 ? 'L' : (lr == 2 ? 'R' : '?'))

int res;
bool bluetooth;
int global_count = 0;
unsigned char buf[65];

struct calibrationData {
	float acc_cal_coeff[3];
	float gyro_cal_coeff[3];
	float cal_x[1] = { 0.0f };
	float cal_y[1] = { 0.0f };

	bool has_user_cal_stick_l = false;
	bool has_usr_cal_stick_r = false;
	bool has_user_cal_sensor = false;

	unsigned char factory_stick_cal[0x12];
	unsigned char user_stick_cal[0x16];
	unsigned char sensor_model[0x6];
	unsigned char stick_model[0x24];
	unsigned char factory_sensor_cal[0x18];
	unsigned char user_sensor_cal[0x1A];
	uint16_t factory_sensor_cal_calm[0xC];
	uint16_t user_sensor_cal_calm[0xC];
	int16_t sensor_cal[0x2][0x3];
	uint16_t stick_cal_x_l[0x3];
	uint16_t stick_cal_y_l[0x3];
	uint16_t stick_cal_x_r[0x3];
	uint16_t stick_cal_y_r[0x3];

};

struct buttonStates {
	
	int RightA = 0;
	int DownB = 0;
	int UpX = 0;
	int LeftY = 0;
	int Trigger = 0;
	int ZTrigger = 0;
	int PlusMinus = 0;
	int HomeCapture = 0;

	int SL = 0;
	int SR = 0;
	int stick_button = 0;
};
struct joyStick {
	uint16_t x = 0;
	uint16_t y = 0;
	float CalX = 0;
	float CalY = 0;
};
struct Gyroscope {
	float pitch = 0;
	float yaw = 0;
	float roll = 0;

	struct Offset {
		int n = 0;

		float pitch = 0;
		float yaw = 0;
		float roll = 0;
	}offset;
};

struct Accelerometer {
	float lastX = 0;
	float lastY = 0;
	float lastZ = 0;

	float x = 0;
	float y = 0;
	float z = 0;
};
struct JoyconState {
	
	uint16_t buttons = 0;
	int8_t dstick;
	uint8_t battery;
	bool isConnected;

	buttonStates bttnStates;
	joyStick Stick;
	Gyroscope gyrescope;
	Accelerometer accelerometer;	
};
struct brdcm_hdr {
	uint16_t cmd;
	uint16_t rumble[9];
};
struct brdcm_cmd_01 {
	uint8_t subcmd;
	uint32_t offset;
	uint8_t size;
};
int timing_byte = 0x0;



void hid_exchange(hid_device *handle, unsigned char *buf, int len)
{
	if (!handle) return;
	
	int res;

	res = hid_write(handle, buf, len);

	res = hid_read(handle, buf, 0x40);
}

void send_command(hid_device *handle, int command, uint8_t *data, int len)
{
	unsigned char buf[0x40];
	memset(buf, 0, 0x40);
	
	buf[0x0] = command;
	if (data != nullptr && len != 0)
	{
		memcpy(buf + (0x1), data, len);
	}

	hid_exchange(handle, buf, len + (0x1));

	if (data) {
		memcpy(data, buf, 0x40);
	}
}

void send_subcommand(hid_device *handle, int command, int subcommand, uint8_t *data, int len)
{
	unsigned char buf[0x40];
	memset(buf, 0, 0x40);

	uint8_t rumble_base[9] = { (global_count++) & 0xf,0x00,0x01,0x40,0x40,0x00,0x01,0x40,0x40 };
	memcpy(buf, rumble_base, 9);

	if (global_count > 0xf) {
		global_count = 0x0;
	}
	buf[9] = subcommand;
	if (data && len != 0) {
		memcpy(buf + 10, data, len);
	}

	send_command(handle,command, buf, 10 + len);

	if (data) {
		memcpy(data, buf, 0x40);
	}
}

int get_spi_data(hid_device *handle, uint32_t offset, const uint16_t read_len, uint8_t *test_buf)
{
	int res;
	uint8_t buf[0x100];
	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brdcm_hdr*)buf;
		auto pkt = (brdcm_cmd_01*)(hdr + 1);
		hdr->cmd = 1;
		hdr->rumble[0] = timing_byte;
		buf[1] = timing_byte;

		timing_byte++;
		if (timing_byte > 0xf)
			timing_byte = 0x0;

		pkt->subcmd = 0x10;
		pkt->offset = offset;
		pkt->size = read_len;

		for (int i = 11; i < 22; i++) {
			buf[i] = buf[i + 3];
		}

		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt));
		res = hid_read(handle, buf, sizeof(buf));
		if ((*(uint16_t*)&buf[0xD] == 0x1090) && (*(uint16_t*)&buf[0xF] == offset)) {
			break;
		}
	}
	if (res >= 0x14 + read_len) {
		for (int i = 0; i < read_len; i++) {
			test_buf[i] = buf[0x14 + 1];
		}
	}
	return 0;
}

int write_api_data(hid_device *handle,uint32_t offset, const uint16_t write_len, uint8_t *test_buf)
{
	int res;
	uint8_t buf[0x100];
	int error_writing = 0;
	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brdcm_hdr*)buf;
		auto pkt = (brdcm_cmd_01*)(hdr + 1);

		hdr->cmd = 1;
		hdr->rumble[0] = timing_byte;
		timing_byte++;
		if (timing_byte > 0xf)
			timing_byte = 0x0;
		pkt->subcmd = 0x11;
		pkt->offset = offset;
		pkt->size = write_len;
		for (int i = 0; i < write_len; i++)
		{
			buf[0x10 + i] = test_buf[i];
		}
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt) + write_len);
		res = hid_read(handle, buf, sizeof(buf));

		if (*(uint16_t*)&buf[0xd] == 0x1180)
			break;
		error_writing++;
		if (error_writing == 125)
			return 1;
	}
	return 0;
}

calibrationData getCalibration(hid_device *handle, int left_right)
{
	calibrationData data;

	memset(data.factory_stick_cal, 0, 0x12);
	memset(data.user_stick_cal, 0, 0x16);
	memset(data.sensor_model, 0, 0x12);
	memset(data.stick_model, 0, 0x12);
	memset(data.factory_sensor_cal, 0, 0x18);
	memset(data.user_sensor_cal, 0, 0x1A);
	memset(data.factory_sensor_cal_calm, 0, 0xC);
	memset(data.user_sensor_cal_calm, 0, 0xC);
	memset(data.sensor_cal, 0, sizeof(data.sensor_cal));
	memset(data.stick_cal_x_l, 0, sizeof(data.stick_cal_x_l));
	memset(data.stick_cal_y_l, 0, sizeof(data.stick_cal_y_l));
	memset(data.stick_cal_x_r, 0, sizeof(data.stick_cal_x_r));
	memset(data.stick_cal_y_r, 0, sizeof(data.stick_cal_y_r));

	get_spi_data(handle,0x6020, 0x18, data.factory_sensor_cal);
	get_spi_data(handle,0x603D, 0x12, data.factory_stick_cal);
	get_spi_data(handle,0x6080, 0x6, data.sensor_model);
	get_spi_data(handle,0x6086, 0x12, data.stick_model);
	get_spi_data(handle,0x6098, 0x12, &data.stick_model[0x12]);
	get_spi_data(handle,0x8010, 0x16, data.user_stick_cal);
	get_spi_data(handle,0x8026, 0x1A, data.user_sensor_cal);
	// get stick calibration data:

	// factory calibration:

	if (left_right == 1 || left_right == 3) {
		data.stick_cal_x_l[1] = (data.factory_stick_cal[4] << 8) & 0xF00 | data.factory_stick_cal[3];
		data.stick_cal_y_l[1] = (data.factory_stick_cal[5] << 4) | (data.factory_stick_cal[4] >> 4);
		data.stick_cal_x_l[0] = data.stick_cal_x_l[1] - ((data.factory_stick_cal[7] << 8) & 0xF00 | data.factory_stick_cal[6]);
		data.stick_cal_y_l[0] = data.stick_cal_y_l[1] - ((data.factory_stick_cal[8] << 4) | (data.factory_stick_cal[7] >> 4));
		data.stick_cal_x_l[2] = data.stick_cal_x_l[1] + ((data.factory_stick_cal[1] << 8) & 0xF00 | data.factory_stick_cal[0]);
		data.stick_cal_y_l[2] = data.stick_cal_y_l[1] + ((data.factory_stick_cal[2] << 4) | (data.factory_stick_cal[2] >> 4));

	}

	if (left_right == 2 || left_right == 3) {
		data.stick_cal_x_r[1] = (data.factory_stick_cal[10] << 8) & 0xF00 | data.factory_stick_cal[9];
		data.stick_cal_y_r[1] = (data.factory_stick_cal[11] << 4) | (data.factory_stick_cal[10] >> 4);
		data.stick_cal_x_r[0] = data.stick_cal_x_r[1] - ((data.factory_stick_cal[13] << 8) & 0xF00 | data.factory_stick_cal[12]);
		data.stick_cal_y_r[0] = data.stick_cal_y_r[1] - ((data.factory_stick_cal[14] << 4) | (data.factory_stick_cal[13] >> 4));
		data.stick_cal_x_r[2] = data.stick_cal_x_r[1] + ((data.factory_stick_cal[16] << 8) & 0xF00 | data.factory_stick_cal[15]);
		data.stick_cal_y_r[2] = data.stick_cal_y_r[1] + ((data.factory_stick_cal[17] << 4) | (data.factory_stick_cal[16] >> 4));
	}


	// if there is user calibration data:
	if ((data.user_stick_cal[0] | data.user_stick_cal[1] << 8) == 0xA1B2) {
		data.stick_cal_x_l[1] = (data.user_stick_cal[6] << 8) & 0xF00 | data.user_stick_cal[5];
		data.stick_cal_y_l[1] = (data.user_stick_cal[7] << 4) | (data.user_stick_cal[6] >> 4);
		data.stick_cal_x_l[0] = data.stick_cal_x_l[1] - ((data.user_stick_cal[9] << 8) & 0xF00 | data.user_stick_cal[8]);
		data.stick_cal_y_l[0] = data.stick_cal_y_l[1] - ((data.user_stick_cal[10] << 4) | (data.user_stick_cal[9] >> 4));
		data.stick_cal_x_l[2] = data.stick_cal_x_l[1] + ((data.user_stick_cal[3] << 8) & 0xF00 | data.user_stick_cal[2]);
		data.stick_cal_y_l[2] = data.stick_cal_y_l[1] + ((data.user_stick_cal[4] << 4) | (data.user_stick_cal[3] >> 4));
		//FormJoy::myform1->textBox_lstick_ucal->Text = String::Format(L"L Stick User:\r\nCenter X,Y: ({0:X3}, {1:X3})\r\nX: [{2:X3} - {4:X3}] Y: [{3:X3} - {5:X3}]",
		//stick_cal_x_l[1], stick_cal_y_l[1], stick_cal_x_l[0], stick_cal_y_l[0], stick_cal_x_l[2], stick_cal_y_l[2]);
	}
	else {
		//FormJoy::myform1->textBox_lstick_ucal->Text = L"L Stick User:\r\nNo calibration";
		//printf("no user Calibration data for left stick.\n");
	}

	if ((data.user_stick_cal[0xB] | data.user_stick_cal[0xC] << 8) == 0xA1B2) {
		data.stick_cal_x_r[1] = (data.user_stick_cal[14] << 8) & 0xF00 | data.user_stick_cal[13];
		data.stick_cal_y_r[1] = (data.user_stick_cal[15] << 4) | (data.user_stick_cal[14] >> 4);
		data.stick_cal_x_r[0] = data.stick_cal_x_r[1] - ((data.user_stick_cal[17] << 8) & 0xF00 | data.user_stick_cal[16]);
		data.stick_cal_y_r[0] = data.stick_cal_y_r[1] - ((data.user_stick_cal[18] << 4) | (data.user_stick_cal[17] >> 4));
		data.stick_cal_x_r[2] = data.stick_cal_x_r[1] + ((data.user_stick_cal[20] << 8) & 0xF00 | data.user_stick_cal[19]);
		data.stick_cal_y_r[2] = data.stick_cal_y_r[1] + ((data.user_stick_cal[21] << 4) | (data.user_stick_cal[20] >> 4));
		//FormJoy::myform1->textBox_rstick_ucal->Text = String::Format(L"R Stick User:\r\nCenter X,Y: ({0:X3}, {1:X3})\r\nX: [{2:X3} - {4:X3}] Y: [{3:X3} - {5:X3}]",
		//stick_cal_x_r[1], stick_cal_y_r[1], stick_cal_x_r[0], stick_cal_y_r[0], stick_cal_x_r[2], stick_cal_y_r[2]);
	}
	else {
		//FormJoy::myform1->textBox_rstick_ucal->Text = L"R Stick User:\r\nNo calibration";
		//printf("no user Calibration data for right stick.\n");
	}

	// get gyro / accelerometer calibration data:

	// factory calibration:

	// Acc cal origin position
	data.sensor_cal[0][0] = uint16_to_int16(data.factory_sensor_cal[0] | data.factory_sensor_cal[1] << 8);
	data.sensor_cal[0][1] = uint16_to_int16(data.factory_sensor_cal[2] | data.factory_sensor_cal[3] << 8);
	data.sensor_cal[0][2] = uint16_to_int16(data.factory_sensor_cal[4] | data.factory_sensor_cal[5] << 8);

	// Gyro cal origin position
	data.sensor_cal[1][0] = uint16_to_int16(data.factory_sensor_cal[0xC] | data.factory_sensor_cal[0xD] << 8);
	data.sensor_cal[1][1] = uint16_to_int16(data.factory_sensor_cal[0xE] | data.factory_sensor_cal[0xF] << 8);
	data.sensor_cal[1][2] = uint16_to_int16(data.factory_sensor_cal[0x10] | data.factory_sensor_cal[0x11] << 8);

	// user calibration:
	if ((data.user_sensor_cal[0x0] | data.user_sensor_cal[0x1] << 8) == 0xA1B2) {
		//FormJoy::myform1->textBox_6axis_ucal->Text = L"6-Axis User (XYZ):\r\nAcc:  ";
		//for (int i = 0; i < 0xC; i = i + 6) {
		//	FormJoy::myform1->textBox_6axis_ucal->Text += String::Format(L"{0:X4} {1:X4} {2:X4}\r\n      ",
		//		user_sensor_cal[i + 2] | user_sensor_cal[i + 3] << 8,
		//		user_sensor_cal[i + 4] | user_sensor_cal[i + 5] << 8,
		//		user_sensor_cal[i + 6] | user_sensor_cal[i + 7] << 8);
		//}
		// Acc cal origin position
		data.sensor_cal[0][0] = uint16_to_int16(data.user_sensor_cal[2] | data.user_sensor_cal[3] << 8);
		data.sensor_cal[0][1] = uint16_to_int16(data.user_sensor_cal[4] | data.user_sensor_cal[5] << 8);
		data.sensor_cal[0][2] = uint16_to_int16(data.user_sensor_cal[6] | data.user_sensor_cal[7] << 8);
		//FormJoy::myform1->textBox_6axis_ucal->Text += L"\r\nGyro: ";
		//for (int i = 0xC; i < 0x18; i = i + 6) {
		//	FormJoy::myform1->textBox_6axis_ucal->Text += String::Format(L"{0:X4} {1:X4} {2:X4}\r\n      ",
		//		user_sensor_cal[i + 2] | user_sensor_cal[i + 3] << 8,
		//		user_sensor_cal[i + 4] | user_sensor_cal[i + 5] << 8,
		//		user_sensor_cal[i + 6] | user_sensor_cal[i + 7] << 8);
		//}
		// Gyro cal origin position
		data.sensor_cal[1][0] = uint16_to_int16(data.user_sensor_cal[0xE] | data.user_sensor_cal[0xF] << 8);
		data.sensor_cal[1][1] = uint16_to_int16(data.user_sensor_cal[0x10] | data.user_sensor_cal[0x11] << 8);
		data.sensor_cal[1][2] = uint16_to_int16(data.user_sensor_cal[0x12] | data.user_sensor_cal[0x13] << 8);
	}
	else {
		//FormJoy::myform1->textBox_6axis_ucal->Text = L"\r\n\r\nUser:\r\nNo calibration";
	}

	// Use SPI calibration and convert them to SI acc unit
	data.acc_cal_coeff[0] = (float)(1.0 / (float)(16384 - uint16_to_int16(data.sensor_cal[0][0]))) * 4.0f  * 9.8f;
	data.acc_cal_coeff[1] = (float)(1.0 / (float)(16384 - uint16_to_int16(data.sensor_cal[0][1]))) * 4.0f  * 9.8f;
	data.acc_cal_coeff[2] = (float)(1.0 / (float)(16384 - uint16_to_int16(data.sensor_cal[0][2]))) * 4.0f  * 9.8f;

	// Use SPI calibration and convert them to SI gyro unit
	data.gyro_cal_coeff[0] = (float)(936.0 / (float)(13371 - uint16_to_int16(data.sensor_cal[1][0])) * 0.01745329251994);
	data.gyro_cal_coeff[1] = (float)(936.0 / (float)(13371 - uint16_to_int16(data.sensor_cal[1][1])) * 0.01745329251994);
	data.gyro_cal_coeff[2] = (float)(936.0 / (float)(13371 - uint16_to_int16(data.sensor_cal[1][2])) * 0.01745329251994);

	return data;
}

void CalcAnalogStick(
	float &pOutX,
	float &pOutY,
	uint16_t x,
	uint16_t y,
	uint16_t x_calc[3],
	uint16_t y_calc[3])
{
	float x_f, y_f;

	float deadZoneCenter = 0.15f;

	float deadZoneOuter = 0.10f;

	x = clamp(x, x_calc[0], x_calc[2]);
	y = clamp(y, y_calc[0], y_calc[2]);
	if (x >= x_calc[1]) {
		x_f = (float)(x - x_calc[1]) / (float)(x_calc[2] - x_calc[1]);
	}
	else {
		x_f = -((float)(x - x_calc[1]) / (float)(x_calc[0] - x_calc[1]));
	}
	if (y >= y_calc[1]) {
		y_f = (float)(y - y_calc[1]) / (float)(y_calc[2] - y_calc[1]);
	}
	else {
		y_f = -((float)(y - y_calc[1]) / (float)(y_calc[0] - y_calc[1]));
	}

	float magnitude = sqrtf(x_f*x_f + y_f * y_f);
	if (magnitude > deadZoneCenter) {
		float legalRange = 1.0f - deadZoneOuter - deadZoneCenter;
		float normalizedMag = fmin(1.0f, (magnitude - deadZoneCenter) / legalRange);
		float scale = normalizedMag / magnitude;
		pOutX = (x_f * scale);
		pOutY = (y_f * scale);

	}
	else {
		pOutX = 0;
		pOutY = 0;
	}

}

void CalcAnalogStick(JoyconState *jcState,calibrationData cal, int left_right)
{
	if (left_right == 1) {
		CalcAnalogStick(
			jcState->Stick.CalX,
			jcState->Stick.CalY,
			jcState->Stick.x,
			jcState->Stick.y,
			cal.stick_cal_x_l,
			cal.stick_cal_y_l
		);
	}
	else if (left_right == 2) {
		CalcAnalogStick(
			jcState->Stick.CalX,
			jcState->Stick.CalY,
			jcState->Stick.x,
			jcState->Stick.y,
			cal.stick_cal_x_r,
			cal.stick_cal_y_r
		);
	}
}



void setGyroOffsets(JoyconState* jc)
{
	float threshold = 0.1;
	if (abs(jc->gyrescope.roll) > threshold || abs(jc->gyrescope.pitch) > threshold || abs(jc->gyrescope.yaw) > threshold)
	{
		return;
	}

	jc->gyrescope.offset.n += 1;
	jc->gyrescope.offset.roll = jc->gyrescope.offset.roll + ((jc->gyrescope.roll - jc->gyrescope.offset.roll) / jc->gyrescope.offset.n);
	jc->gyrescope.offset.pitch = jc->gyrescope.offset.pitch + ((jc->gyrescope.pitch - jc->gyrescope.offset.pitch) / jc->gyrescope.offset.n);
	jc->gyrescope.offset.yaw = jc->gyrescope.offset.yaw + ((jc->gyrescope.yaw - jc->gyrescope.offset.yaw) / jc->gyrescope.offset.n);

}
extern "C"
{
	EXPORT void rumble(hid_device* handle, int left_right, int frequency, int intensity)
	{
		unsigned char buf[0x400];
		memset(buf, 0, 0x40);

		buf[1 + 0 + intensity] = 0x1;
		buf[1 + 4 + intensity] = 0x1;

		if (left_right == 1) {
			buf[1 + 0] = frequency; //(0,255)
		}
		else {
			buf[1 + 4] = frequency;
		}
		hid_set_nonblocking(handle, 1);

		send_command(handle, 0x10, (uint8_t*)buf, 0x9);

	}

	EXPORT int joyconCount()
	{
		int ret = 0;

		struct hid_device_info *devs, *curdev;

		devs = hid_enumerate(JOYCON_VENDOR, 0x0);
		curdev = devs;
		while (curdev)
		{
			if (curdev->vendor_id == JOYCON_VENDOR)
			{
				if (curdev->product_id == JOYCON_L_BT || curdev->product_id == JOYCON_R_BT)
				{
					ret++;
				}
			}
			curdev = curdev->next;
		}
		hid_free_enumeration(devs);
		return ret;
	}

	EXPORT hid_device* get_device(int device)
	{

		hid_device* handle;

		struct hid_device_info *devs, *cur_dev;

		//res = hid_init();
		int controller_count = 0;
		devs = hid_enumerate(JOYCON_VENDOR, 0x0);
		cur_dev = devs;
		while (cur_dev)
		{
			if (cur_dev->vendor_id == JOYCON_VENDOR)
			{
				if (cur_dev->product_id == JOYCON_L_BT || cur_dev->product_id == JOYCON_R_BT)
				{
					if (controller_count = device)
					{
						
						handle = hid_open_path(cur_dev->path);
						hid_free_enumeration(devs);
						return handle;
					}

					controller_count++;
				}
			}
			cur_dev = cur_dev->next;

		}
		hid_free_enumeration(devs);

	}

	EXPORT void init_bt(hid_device* handle, int reportMode, int left_right)
	{
		unsigned char buf[0x40];
		memset(buf, 0, 0x40);

		hid_set_nonblocking(handle, 0);
		//enable vibration
		buf[0] = 0x01;
		send_subcommand(handle, 0x1, 0x48, buf, 1);
		//enable IMU data
		buf[0] = 0x01;
		send_subcommand(handle, 0x01, 0x40, buf, 1);

		//getCalibration(handle, data, left_right);


		return;
	}
	
	EXPORT void handle_input(hid_device* handle, JoyconState* state, int left_right)
	{
		if (!handle)
			return;
		
		calibrationData cal = getCalibration(handle, left_right);

		//get input
		memset(buf, 0, 65);

		hid_read_timeout(handle, buf, 0x40, 20);
		//bluetooth button
		if (buf[0] == 0x3F) {
			uint16_t old_buttons = state->buttons;
			int8_t old_stick = state->dstick;

			state->dstick = buf[3];
		}


		//input update packet:
		//0x21 for buttons, 0x30 for gyro, 0x31 for NFC
		if (buf[0] == 0x21 || buf[0] == 0x30 || buf[0] == 0x31) {

			int offset = 0;
			uint8_t *btn_data = buf + offset + 3;

			{
				uint16_t states = 0;
				if (left_right == 1) {
					states = (btn_data[1] << 8) | (btn_data[2] & 0xFF);
				}
				else if (left_right == 2) {
					states = (btn_data[1] << 8) | (btn_data[0] & 0xFF);

				}
				state->buttons = states;
			}
			//stick data
			uint8_t *stick_data = buf + offset;
			if (left_right == 1) {
				stick_data += 6;
			}
			else if(left_right == 2)
			{
				stick_data += 9;
			}

			uint16_t stick_x = stick_data[0] | ((stick_data[1] & 0xF) << 8);
			uint16_t stick_y = (stick_data[1] >> 4 | (stick_data[2] << 4));
			state->Stick.x = stick_x;
			state->Stick.y = stick_y;
			//calibrate
			CalcAnalogStick(state,cal, left_right);

			state->battery = (stick_data[1] & 0xF0) >> 4;

			//Accelerometer
			//Accelerometer data is absolute (m/s^2)
			{
				// get accelerometer X:
				state->accelerometer.x = (float)(uint16_to_int16(buf[13] | (buf[14] << 8) & 0xFF00)) * cal.acc_cal_coeff[0];

				// get accelerometer Y:
				state->accelerometer.y = (float)(uint16_to_int16(buf[15] | (buf[16] << 8) & 0xFF00)) * cal.acc_cal_coeff[1];

				// get accelerometer Z:
				state->accelerometer.z = (float)(uint16_to_int16(buf[17] | (buf[18] << 8) & 0xFF00)) * cal.acc_cal_coeff[2];

			}
			// Gyroscope:
			// Gyroscope data is relative (rads/s)
			{

				// get roll:
				state->gyrescope.roll = (float)((uint16_to_int16(buf[19] | (buf[20] << 8) & 0xFF00)) - cal.sensor_cal[1][0]) * cal.gyro_cal_coeff[0];

				// get pitch:
				state->gyrescope.pitch = (float)((uint16_to_int16(buf[21] | (buf[22] << 8) & 0xFF00)) - cal.sensor_cal[1][1]) * cal.gyro_cal_coeff[1];

				// get yaw:
				state->gyrescope.yaw = (float)((uint16_to_int16(buf[23] | (buf[24] << 8) & 0xFF00)) - cal.sensor_cal[1][2]) * cal.gyro_cal_coeff[2];
			}
			// offsets:
			{
				setGyroOffsets(state);

				state->gyrescope.roll -= state->gyrescope.offset.roll;
				state->gyrescope.pitch -= state->gyrescope.offset.pitch;
				state->gyrescope.yaw -= state->gyrescope.offset.yaw;
			
			}

			if (left_right == 1)
			{
				state->bttnStates.DownB = (state->buttons & (1 << 0)) ? 1 : 0;
				state->bttnStates.UpX = (state->buttons & (1 << 1)) ? 1 : 0;
				state->bttnStates.RightA = (state->buttons & (1 << 2)) ? 1 : 0;
				state->bttnStates.LeftY = (state->buttons & (1 << 3)) ? 1 : 0;
				state->bttnStates.SR = (state->buttons & (1 << 4)) ? 1 : 0;
				state->bttnStates.SL = (state->buttons & (1 << 5)) ? 1 : 0;
				state->bttnStates.Trigger = (state->buttons & (1 << 6)) ? 1 : 0;
				state->bttnStates.ZTrigger = (state->buttons &(1 << 7)) ? 1 : 0;
				state->bttnStates.PlusMinus = (state->buttons &(1 << 8)) ? 1 : 0;
				state->bttnStates.stick_button = (state->buttons &(1 << 11)) ? 1 : 0;
				state->bttnStates.HomeCapture = (state->buttons &(1 << 13)) ? 1 : 0;
			}
			if (left_right == 2)
			{
				state->bttnStates.LeftY = (state->buttons & (1 << 0)) ? 1 : 0;
				state->bttnStates.UpX = (state->buttons & (1 << 1)) ? 1 : 0;
				state->bttnStates.DownB = (state->buttons & (1 << 2)) ? 1 : 0;
				state->bttnStates.RightA = (state->buttons & (1 << 3)) ? 1 : 0;
				state->bttnStates.SR = (state->buttons & (1 << 4)) ? 1 : 0;
				state->bttnStates.SL = (state->buttons & (1 << 5)) ? 1 : 0;
				state->bttnStates.Trigger = (state->buttons & (1 << 6)) ? 1 : 0;
				state->bttnStates.ZTrigger = (state->buttons & (1 << 7)) ? 1 : 0;
				state->bttnStates.PlusMinus = (state->buttons & (1 << 9)) ? 1 : 0;
				state->bttnStates.stick_button = (state->buttons & (1 << 10)) ? 1 : 0;
				state->bttnStates.HomeCapture = (state->buttons & (1 << 12)) ? 1 : 0;
			}
		}		
		
	}

}
 