#include "stdafx.h"
#include "Joycon.h"

Joycon::Joycon(struct hid_device_info *dev)
{
	if (dev->product_id == JOYCON_L_BT) {
		this->name = std::string("Joy-Con (L)");
		this->left_right = 1;
	}
	else if (dev->product_id == JOYCON_R_BT) {
		this->name = std::string("Joy-Con (R)");
		this->left_right = 2;
	}

	this->handle = hid_open_path(dev->path);
	
	if (this->handle == nullptr)
	{
		printf("Could not find JoyCon");
		throw;
	}
}

void Joycon::hid_exchange(hid_device *handle, unsigned char *buf, int len)
{
	if (!handle) return;

	int res;

	res = hid_write(handle, buf, len);

	res = hid_read(handle, buf, 0x40);
}

void Joycon::send_command(int command, uint8_t *data, int len)
{
	unsigned char buf[0x40];
	memset(buf, 0, 0x40);

	buf[bluetooth ? 0x0 : 0x8] = command;
	if (data != nullptr && len != 0) {
		memcpy(buf + (bluetooth ? 0x1 : 0x9), data, len);
	}

	hid_exchange(this->handle, buf, len + (bluetooth ? 0x1 : 0x9));

	if (data) {
		memcpy(data, buf, 0x40);
	}
}

void Joycon::send_subcommand(int command, int subcommand, uint8_t *data, int len)
{
	unsigned char buf[0x40];
	memset(buf, 0, 0x40);

	uint8_t rumble_base[9] = { (global_count++) & 0xF,0x00,0x01,0x40,0x40,0x00,0x01,0x40,0x40 };
	memcpy(buf, rumble_base, 9);

	if (global_count > 0xF) {
		global_count = 0x0;
	}
	buf[9] = subcommand;
	if (data && len != 0) {
		memcpy(buf + 10, data, len);
	}

	send_command(command, buf, 10 + len);

	if (data) {
		memcpy(data, buf, 0x40);//Original has a TODO here so I have no idea what that means, so rip
	}

}


void Joycon::rumble(int frequency, int intensity) {
	
	unsigned char buf[0x400];
	memset(buf, 0, 0x40);
	//I think this is turning on the intensity for each controller. 
	buf[1 + 0 + intensity] = 0x1;
	buf[1 + 4 + intensity] = 0x1;
	
	//set frequency to increase
	if (this->left_right == 1) {
		buf[1 + 0] = frequency;//(0, 255)		
	}
	else {
		buf[1 + 4] = frequency;//(0, f255)
	}

	//set non-blocking:
	hid_set_nonblocking(this->handle, 1);

	send_command(0x10, (uint8_t*)buf, 0x9);
}

//void rumble2(uint16_t hf, uint8_t hfa, uint8_t lf, uint16_t lfa);

//setting gyro offsets
void Joycon::setGyroOffsets() {
	float threshold = 0.1;
	//if gyro position is close enough to 0 don't set offsets.
	if (abs(this->gyro.roll) > threshold || abs(this->gyro.pitch) > threshold || abs(this->gyro.yaw) > threshold) {
		return;
	}
	this->gyro.offset.n += 1;
	this->gyro.offset.roll = this->gyro.offset.roll + ((this->gyro.roll - this->gyro.offset.roll) / this->gyro.offset.n);
	this->gyro.offset.pitch = this->gyro.offset.pitch + ((this->gyro.pitch - this->gyro.offset.pitch) / this->gyro.offset.n);
	this->gyro.offset.yaw = this->gyro.offset.yaw + ((this->gyro.yaw - this->gyro.offset.yaw) / this->gyro.offset.n);
}

int Joycon::init_bt()
{
	this->bluetooth = true;

	unsigned char buf[0x40];
	memset(buf, 0, 0x40);


	//set blocking to nsure command is recieved:
	hid_set_nonblocking(this->handle, 0);

	//Enable vibration
	buf[0] = 0x01; //enabled;
	send_subcommand(0x1, 0x48, buf, 1);

	//enable IMU (gyro and accel) data
	buf[0] = 0x01; //Enabled
	send_subcommand(0x01, 0x40, buf, 1);
	
	// Set input report mode (to push at 60hz)
	// x00	Active polling mode for IR camera data. Answers with more than 300 bytes ID 31 packet
	// x01	Active polling mode
	// x02	Active polling mode for IR camera data.Special IR mode or before configuring it ?
	// x21	Unknown.An input report with this ID has pairing or mcu data or serial flash data or device info
	// x23	MCU update input report ?
	// 30	NPad standard mode. Pushes current state @60Hz. Default in SDK if arg is not in the list
	// 31	NFC mode. Pushes large packets @60Hz
	buf[0] = 0x30;
	send_subcommand(0x01, 0x03, buf, 1);

	//get calibration data
	memset(factory_stick_cal, 0, 0x12);
	memset(user_stick_cal, 0, 0x16);
	memset(sensor_model, 0, 0x12);
	memset(stick_model, 0, 0x12);
	memset(factory_sensor_cal, 0, 0x18);
	memset(user_sensor_cal, 0, 0x1A);
	memset(factory_sensor_cal_calm, 0, 0xC);
	memset(user_sensor_cal_calm, 0, 0xC);
	memset(sensor_cal, 0, sizeof(sensor_cal));
	memset(stick_cal_x_l, 0, sizeof(stick_cal_x_l));
	memset(stick_cal_y_l, 0, sizeof(stick_cal_y_l));
	memset(stick_cal_x_r, 0, sizeof(stick_cal_x_r));
	memset(stick_cal_y_r, 0, sizeof(stick_cal_y_r));

	get_spi_data(0x6020, 0x18, factory_sensor_cal);
	get_spi_data(0x603D, 0x12, factory_stick_cal);
	get_spi_data(0x6080, 0x6, sensor_model);
	get_spi_data(0x6086, 0x12, stick_model);
	get_spi_data(0x6098, 0x12, &stick_model[0x12]);
	get_spi_data(0x8010, 0x16, user_stick_cal);
	get_spi_data(0x8026, 0x1A, user_sensor_cal);
	

	// get stick calibration data:

	// factory calibration:

	if (this->left_right == 1 || this->left_right == 3) {
		stick_cal_x_l[1] = (factory_stick_cal[4] << 8) & 0xF00 | factory_stick_cal[3];
		stick_cal_y_l[1] = (factory_stick_cal[5] << 4) | (factory_stick_cal[4] >> 4);
		stick_cal_x_l[0] = stick_cal_x_l[1] - ((factory_stick_cal[7] << 8) & 0xF00 | factory_stick_cal[6]);
		stick_cal_y_l[0] = stick_cal_y_l[1] - ((factory_stick_cal[8] << 4) | (factory_stick_cal[7] >> 4));
		stick_cal_x_l[2] = stick_cal_x_l[1] + ((factory_stick_cal[1] << 8) & 0xF00 | factory_stick_cal[0]);
		stick_cal_y_l[2] = stick_cal_y_l[1] + ((factory_stick_cal[2] << 4) | (factory_stick_cal[2] >> 4));

	}

	if (this->left_right == 2 || this->left_right == 3) {
		stick_cal_x_r[1] = (factory_stick_cal[10] << 8) & 0xF00 | factory_stick_cal[9];
		stick_cal_y_r[1] = (factory_stick_cal[11] << 4) | (factory_stick_cal[10] >> 4);
		stick_cal_x_r[0] = stick_cal_x_r[1] - ((factory_stick_cal[13] << 8) & 0xF00 | factory_stick_cal[12]);
		stick_cal_y_r[0] = stick_cal_y_r[1] - ((factory_stick_cal[14] << 4) | (factory_stick_cal[13] >> 4));
		stick_cal_x_r[2] = stick_cal_x_r[1] + ((factory_stick_cal[16] << 8) & 0xF00 | factory_stick_cal[15]);
		stick_cal_y_r[2] = stick_cal_y_r[1] + ((factory_stick_cal[17] << 4) | (factory_stick_cal[16] >> 4));
	}


	// if there is user calibration data:
	if ((user_stick_cal[0] | user_stick_cal[1] << 8) == 0xA1B2) {
		stick_cal_x_l[1] = (user_stick_cal[6] << 8) & 0xF00 | user_stick_cal[5];
		stick_cal_y_l[1] = (user_stick_cal[7] << 4) | (user_stick_cal[6] >> 4);
		stick_cal_x_l[0] = stick_cal_x_l[1] - ((user_stick_cal[9] << 8) & 0xF00 | user_stick_cal[8]);
		stick_cal_y_l[0] = stick_cal_y_l[1] - ((user_stick_cal[10] << 4) | (user_stick_cal[9] >> 4));
		stick_cal_x_l[2] = stick_cal_x_l[1] + ((user_stick_cal[3] << 8) & 0xF00 | user_stick_cal[2]);
		stick_cal_y_l[2] = stick_cal_y_l[1] + ((user_stick_cal[4] << 4) | (user_stick_cal[3] >> 4));
		//FormJoy::myform1->textBox_lstick_ucal->Text = String::Format(L"L Stick User:\r\nCenter X,Y: ({0:X3}, {1:X3})\r\nX: [{2:X3} - {4:X3}] Y: [{3:X3} - {5:X3}]",
		//stick_cal_x_l[1], stick_cal_y_l[1], stick_cal_x_l[0], stick_cal_y_l[0], stick_cal_x_l[2], stick_cal_y_l[2]);
	}
	else {
		//FormJoy::myform1->textBox_lstick_ucal->Text = L"L Stick User:\r\nNo calibration";
		//printf("no user Calibration data for left stick.\n");
	}

	if ((user_stick_cal[0xB] | user_stick_cal[0xC] << 8) == 0xA1B2) {
		stick_cal_x_r[1] = (user_stick_cal[14] << 8) & 0xF00 | user_stick_cal[13];
		stick_cal_y_r[1] = (user_stick_cal[15] << 4) | (user_stick_cal[14] >> 4);
		stick_cal_x_r[0] = stick_cal_x_r[1] - ((user_stick_cal[17] << 8) & 0xF00 | user_stick_cal[16]);
		stick_cal_y_r[0] = stick_cal_y_r[1] - ((user_stick_cal[18] << 4) | (user_stick_cal[17] >> 4));
		stick_cal_x_r[2] = stick_cal_x_r[1] + ((user_stick_cal[20] << 8) & 0xF00 | user_stick_cal[19]);
		stick_cal_y_r[2] = stick_cal_y_r[1] + ((user_stick_cal[21] << 4) | (user_stick_cal[20] >> 4));
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
	sensor_cal[0][0] = uint16_to_int16(factory_sensor_cal[0] | factory_sensor_cal[1] << 8);
	sensor_cal[0][1] = uint16_to_int16(factory_sensor_cal[2] | factory_sensor_cal[3] << 8);
	sensor_cal[0][2] = uint16_to_int16(factory_sensor_cal[4] | factory_sensor_cal[5] << 8);

	// Gyro cal origin position
	sensor_cal[1][0] = uint16_to_int16(factory_sensor_cal[0xC] | factory_sensor_cal[0xD] << 8);
	sensor_cal[1][1] = uint16_to_int16(factory_sensor_cal[0xE] | factory_sensor_cal[0xF] << 8);
	sensor_cal[1][2] = uint16_to_int16(factory_sensor_cal[0x10] | factory_sensor_cal[0x11] << 8);

	// user calibration:
	if ((user_sensor_cal[0x0] | user_sensor_cal[0x1] << 8) == 0xA1B2) {
		//FormJoy::myform1->textBox_6axis_ucal->Text = L"6-Axis User (XYZ):\r\nAcc:  ";
		//for (int i = 0; i < 0xC; i = i + 6) {
		//	FormJoy::myform1->textBox_6axis_ucal->Text += String::Format(L"{0:X4} {1:X4} {2:X4}\r\n      ",
		//		user_sensor_cal[i + 2] | user_sensor_cal[i + 3] << 8,
		//		user_sensor_cal[i + 4] | user_sensor_cal[i + 5] << 8,
		//		user_sensor_cal[i + 6] | user_sensor_cal[i + 7] << 8);
		//}
		// Acc cal origin position
		sensor_cal[0][0] = uint16_to_int16(user_sensor_cal[2] | user_sensor_cal[3] << 8);
		sensor_cal[0][1] = uint16_to_int16(user_sensor_cal[4] | user_sensor_cal[5] << 8);
		sensor_cal[0][2] = uint16_to_int16(user_sensor_cal[6] | user_sensor_cal[7] << 8);
		//FormJoy::myform1->textBox_6axis_ucal->Text += L"\r\nGyro: ";
		//for (int i = 0xC; i < 0x18; i = i + 6) {
		//	FormJoy::myform1->textBox_6axis_ucal->Text += String::Format(L"{0:X4} {1:X4} {2:X4}\r\n      ",
		//		user_sensor_cal[i + 2] | user_sensor_cal[i + 3] << 8,
		//		user_sensor_cal[i + 4] | user_sensor_cal[i + 5] << 8,
		//		user_sensor_cal[i + 6] | user_sensor_cal[i + 7] << 8);
		//}
		// Gyro cal origin position
		sensor_cal[1][0] = uint16_to_int16(user_sensor_cal[0xE] | user_sensor_cal[0xF] << 8);
		sensor_cal[1][1] = uint16_to_int16(user_sensor_cal[0x10] | user_sensor_cal[0x11] << 8);
		sensor_cal[1][2] = uint16_to_int16(user_sensor_cal[0x12] | user_sensor_cal[0x13] << 8);
	}
	else {
		//FormJoy::myform1->textBox_6axis_ucal->Text = L"\r\n\r\nUser:\r\nNo calibration";
	}

	// Use SPI calibration and convert them to SI acc unit
	acc_cal_coeff[0] = (float)(1.0 / (float)(16384 - uint16_to_int16(sensor_cal[0][0]))) * 4.0f  * 9.8f;
	acc_cal_coeff[1] = (float)(1.0 / (float)(16384 - uint16_to_int16(sensor_cal[0][1]))) * 4.0f  * 9.8f;
	acc_cal_coeff[2] = (float)(1.0 / (float)(16384 - uint16_to_int16(sensor_cal[0][2]))) * 4.0f  * 9.8f;

	// Use SPI calibration and convert them to SI gyro unit
	gyro_cal_coeff[0] = (float)(936.0 / (float)(13371 - uint16_to_int16(sensor_cal[1][0])) * 0.01745329251994);
	gyro_cal_coeff[1] = (float)(936.0 / (float)(13371 - uint16_to_int16(sensor_cal[1][1])) * 0.01745329251994);
	gyro_cal_coeff[2] = (float)(936.0 / (float)(13371 - uint16_to_int16(sensor_cal[1][2])) * 0.01745329251994);

	return 0;
}
//do I need this? probably not
void init_usb();

void Joycon::CalcAnalogStick()
{
	if (this->left_right == 1) {
		CalcAnalogStick(
			this->stick.CalX,
			this->stick.CalY,
			this->stick.x,
			this->stick.y,
			this->stick_cal_x_l,
			this->stick_cal_y_l);

	}
	else if (this->left_right == 2) {
		CalcAnalogStick(
			this->stick.CalX,
			this->stick.CalY,
			this->stick.x,
			this->stick.y,
			this->stick_cal_x_r,
			this->stick_cal_y_r);

	}
	
	
}

void Joycon::CalcAnalogStick(
	float &pOutX,	//out: rsulting stick X value
	float &pOutY,	//out: rsulting stick Y value
	uint16_t x,		//in: initial stick X value
	uint16_t y,		//in: initial stick Y value
	uint16_t x_calc[3], //calc -X, CenterX, +X
	uint16_t y_calc[3]  //calc -Y, CenterY, +Y
){
	float x_f, y_f;
	// Apply Joy-Con center deadzone. 0xAE translates approx to 15%. Pro controller has a 10% () deadzone
	float deadZoneCenter = 0.15f;
	// Add a small ammount of outer deadzone to avoid edge cases or machine variety.
	float deadZoneOuter = 0.10f;

	// convert to float based on calibration and valid ranges per +/-axis
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

	// Interpolate zone between deadzones
	float mag = sqrtf(x_f*x_f + y_f * y_f);
	if (mag > deadZoneCenter) {
		// scale such that output magnitude is in the range [0.0f, 1.0f]
		float legalRange = 1.0f - deadZoneOuter - deadZoneCenter;
		float normalizedMag = min(1.0f, (mag - deadZoneCenter) / legalRange);
		float scale = normalizedMag / mag;
		pOutX = (x_f * scale);
		pOutY = (y_f * scale);
	}
	else {
		// stick is in the inner dead zone
		pOutX = 0.0f;
		pOutY = 0.0f;
	}
}

int Joycon::get_spi_data(uint32_t offset, const uint16_t read_len, uint8_t *test_buf)
{
	int res;
	uint8_t buf[0x100];
	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 1;
		hdr->rumble[0] = timing_byte;

		buf[1] = timing_byte;

		timing_byte++;
		if (timing_byte > 0xF)
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

int Joycon::write_api_data(uint32_t offset, const uint16_t write_len, uint8_t *test_buf)
{
	int res;
	uint8_t buf[0x100];
	int error_writing = 0;
	while (1) {
		memset(buf, 0, sizeof(buf));
		auto hdr = (brcm_hdr *)buf;
		auto pkt = (brcm_cmd_01 *)(hdr + 1);
		hdr->cmd = 1;
		hdr->rumble[0] = timing_byte;
		timing_byte++;
		if (timing_byte > 0xF)
			timing_byte = 0x0;
		pkt->subcmd = 0x11;
		pkt->offset = offset;
		pkt->size = write_len;
		for (int i = 0; i < write_len; i++) {
			buf[0x10 + i] = test_buf[i];
		}
		res = hid_write(handle, buf, sizeof(*hdr) + sizeof(*pkt) + write_len);

		res = hid_read(handle, buf, sizeof(buf));

		if (*(uint16_t*)&buf[0xD] == 0x1180)
			break;
		error_writing++;
		if (error_writing == 125)
		{
			return 1;
		}
	}
	return 0;
}