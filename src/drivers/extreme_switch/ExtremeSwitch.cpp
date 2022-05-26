/****************************************************************************
 *
 *   Copyright (c) 2021 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include "ExtremeSwitch.hpp"

ExtremeSwitch::ExtremeSwitch(int bus, uint32_t device, int bus_frequency, spi_mode_e spi_mode) :
	SPI(DRV_DEVTYPE_UNUSED, MODULE_NAME, bus, device, spi_mode, bus_frequency),
	ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::test1)
{
}

ExtremeSwitch::~ExtremeSwitch()
{
	perf_free(_loop_perf);
	perf_free(_loop_interval_perf);
}

int ExtremeSwitch::init()
{
	/* Let's not do this yet
	// execute Run() on every sensor_accel publication
	if (!_sensor_accel_sub.registerCallback()) {
		PX4_ERR("sensor_accel callback registration failed");
		return false;
	}
	*/

	int ret = SPI::init();

	if(ret != OK) {
		printf("SPI::init() failed\n");
		DEVICE_DEBUG("SPI Init Failed");
		return -EIO;
	}

	// alternatively, Run on fixed interval
	ScheduleOnInterval(10000_us); // 2000 us interval, 200 Hz rate

	return PX4_OK;
}

void ExtremeSwitch::Run()
{
	if (should_exit()) {
		ScheduleClear();
		exit_and_cleanup();
		return;
	}

	perf_begin(_loop_perf);
	perf_count(_loop_interval_perf);
	
	uint8_t buf;
	uint8_t rbuf;

	printf("Starting loop\n\n");


	printf("Transferring OCR register \n");
	buf = 0b00000001;
	transfer(&buf, &rbuf, sizeof(uint8_t));
	printf("rbuf: 0x%X\n", rbuf);

	px4_usleep(500000);

	printf("Transferring ON command \n");
	buf = 0b00010001;
	transfer(&buf, &rbuf, sizeof(uint8_t));
	printf("rbuf: 0x%X\n", rbuf);
	
	px4_usleep(100000);
	
	printf("Transferring OFF command \n");
	buf = 0b00010000;
	transfer(&buf, &rbuf, sizeof(uint8_t));
	printf("rbuf: 0x%X\n", rbuf);

	printf("Ending loop\n\n");

	/*
	 * Just send a few pulses to enable the switch
	 * Wait a while between pulses
	 */
	/*
	// Example
	//  update vehicle_status to check arming state
	if (_vehicle_status_sub.updated()) {
		vehicle_status_s vehicle_status;

		if (_vehicle_status_sub.copy(&vehicle_status)) {

			const bool armed = (vehicle_status.arming_state == vehicle_status_s::ARMING_STATE_ARMED);

			if (armed && !_armed) {
				PX4_WARN("vehicle armed due to %d", vehicle_status.latest_arming_reason);

			} else if (!armed && _armed) {
				PX4_INFO("vehicle disarmed due to %d", vehicle_status.latest_disarming_reason);
			}

			_armed = armed;
		}
	}


	// Example
	//  grab latest accelerometer data
	if (_sensor_accel_sub.updated()) {
		sensor_accel_s accel;

		if (_sensor_accel_sub.copy(&accel)) {
			// DO WORK

			// access parameter value (SYS_AUTOSTART)
			if (_param_sys_autostart.get() == 1234) {
				// do something if SYS_AUTOSTART is 1234
			}
		}
	}


	// Example
	//  publish some data
	orb_test_s data{};
	data.val = 314159;
	data.timestamp = hrt_absolute_time();
	_orb_test_pub.publish(data);
	*/


	perf_end(_loop_perf);
}

int ExtremeSwitch::task_spawn(int argc, char *argv[])
{
	ExtremeSwitch *instance = new ExtremeSwitch(1, 0x10000000, 1000000, SPIDEV_MODE1);

	if (instance) {
		_object.store(instance);
		_task_id = task_id_is_work_queue;

		if (instance->init() == PX4_OK) {
			return PX4_OK;
		}

	} else {
		PX4_ERR("alloc failed");
	}

	delete instance;
	_object.store(nullptr);
	_task_id = -1;

	return PX4_ERROR;
}

int ExtremeSwitch::print_status()
{
	perf_print_counter(_loop_perf);
	perf_print_counter(_loop_interval_perf);
	return 0;
}

int ExtremeSwitch::custom_command(int argc, char *argv[])
{
	return print_usage("unknown command");
}

int ExtremeSwitch::print_usage(const char *reason)
{
	if (reason) {
		PX4_WARN("%s\n", reason);
	}

	PRINT_MODULE_DESCRIPTION(
		R"DESCR_STR(
### Description
Example of a simple module running out of a work queue.

)DESCR_STR");

	PRINT_MODULE_USAGE_NAME("extreme_switch", "template");
	PRINT_MODULE_USAGE_COMMAND("start");
	PRINT_MODULE_USAGE_DEFAULT_COMMANDS();

	return 0;
}

extern "C" __EXPORT int extreme_switch_main(int argc, char *argv[])
{
	return ExtremeSwitch::main(argc, argv);
}