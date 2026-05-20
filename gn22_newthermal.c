#include <config_definition.h>
#include <gn22.h>

struct gn22_newthermal_gpu_cfg gn22_gpu_cfg_newthermal = {
	.descriptor = {
		.magic = {0x32, 0xac, 0x00, 0x00},
		.length = sizeof(struct gpu_cfg_descriptor),
		.descriptor_version_major = 0,
		.descriptor_version_minor = 3,
		.hardware_version = 0x0005,
		.hardware_revision = 0,
		.serial = {'F', 'R', 'A', 'K', 'M', 'Q', 'C', 'P', 'A', '1',
					'5', '0', '0', 'A', 'S', 'S', 'Y', '0', '\0', '\0'},
		.descriptor_length = sizeof(struct gn22_newthermal_gpu_cfg) - sizeof(struct gpu_cfg_descriptor),
		.descriptor_crc32 = 0,
		.crc32 = 0
	},
	.hdr0 = {.block_type = GPUCFG_TYPE_PCIE, .block_length = sizeof(uint8_t)},
	.pcie_cfg = PCIE_8X1,

	.hdr1 = {.block_type = GPUCFG_TYPE_FAN, .block_length = sizeof(struct gpu_cfg_fan)},
	.fan0_cfg = {.idx = 0, .flags = 0, .min_rpm = 1000, .start_rpm = 1000, .max_rpm = 4700},

	.hdr2 = {.block_type = GPUCFG_TYPE_FAN, .block_length = sizeof(struct gpu_cfg_fan)},
	.fan1_cfg = {.idx = 1, .flags = 0, .min_rpm = 1000, .start_rpm = 1000, .max_rpm = 4500},

	.hdr3 = {.block_type = GPUCFG_TYPE_VENDOR, .block_length = sizeof(enum gpu_vendor)},
	.vendor = GPU_NV_GN22,

	.hdr4 = {.block_type = GPUCFG_TYPE_GPIO, .block_length = (sizeof(struct gpu_cfg_gpio) * 8)},
	/* Critical temperature fault input */
	.gpio0 = {.gpio = GPU_1G1_GPIO0_EC, .function = GPIO_FUNC_TEMPFAULT, .flags = GPIO_INPUT, .power_domain = POWER_S3},
	/* DP HPD status from PD */
	.gpio1 = {.gpio = GPU_1H1_GPIO1_EC, .function = GPIO_FUNC_HPD, .flags = GPIO_INPUT, .power_domain = POWER_S5},
	/* output from the GPU if it is throttling */
	.gpio2 = {.gpio = GPU_2A2_GPIO2_EC, .function = GPIO_FUNC_IS_THROTTLING, .flags = GPIO_INPUT, .power_domain = POWER_S0},
	/* DDS Mux CTRL  from dGPU */
	.gpio3 = {.gpio = GPU_2L7_GPIO3_EC, .function = GPIO_FUNC_UNUSED, .flags = GPIO_INPUT, .power_domain = POWER_S0},
	/* GPU_VSYS_EN */
	.gpio_vsys = {.gpio = GPU_VSYS_EN, .function = GPIO_FUNC_GPU_PWR, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S3},
	/* GPU_VADP_EN */
	.gpu_vadp_en = {.gpio = GPU_VADP_EN, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_G3},

	.gpio_fan = {.gpio = GPU_FAN_EN, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S0},

	.gpu_3v_5v_en = {.gpio = GPU_3V_5V_EN, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_G3},

	.hdr5 = {.block_type = GPUCFG_TYPE_PD, .block_length = sizeof(struct gpu_subsys_pd)},
	.pd = {.gpu_pd_type = PD_TYPE_CCG8S, .address = 0x42,
			.flags = 0, .pdo = 0, .rdo = 0, .power_domain = POWER_G3,
			.gpio_hpd = GPU_1H1_GPIO1_EC, .gpio_interrupt = GPU_1F2_I2C_S5_INT
	},

	.hdr6 = {.block_type = GPUCFG_TYPE_THERMAL_SENSOR, .block_length = sizeof(struct gpu_cfg_thermal)},
	.therm = {.thermal_type = GPU_THERM_F75303, .address = 0x4D},

	.hdr7 = {.block_type = GPUCFG_TYPE_CUSTOM_TEMP, .block_length = sizeof(struct gpu_cfg_custom_temp) * 1},
	.custom_temp = {.idx = CUSTOM_TEMP_temp_sensor_apu, .temp_fan_off = C_TO_K(48), .temp_fan_max = C_TO_K(68)},

	.hdr9 = {.block_type = GPUCFG_TYPE_SUBSYS, .block_length = sizeof(struct gpu_subsys_serial)},
	.pcba_serial = {.gpu_subsys = GPU_PCB, .serial = {'F', 'R', 'A', 'K', 'H', 'Z', 'C', 'P', 'A', '1',
					'5', '0', '0', 'P', 'C', 'B', '0', '0', '\0', '\0'},}
};

void gn22_newthermal_gpu_cfg_write_pcb(char * serial)
{
	strncpy(gn22_gpu_cfg_newthermal.pcba_serial.serial, serial, GPU_SERIAL_LEN);
}
