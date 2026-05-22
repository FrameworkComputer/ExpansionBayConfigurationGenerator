#include <config_definition.h>
#include <ssd.h>

struct default_pcie_thermal_cfg pcie_thermal_cfg = {
	.descriptor = {
		.magic = {0x32, 0xac, 0x00, 0x00},
		.length = sizeof(struct gpu_cfg_descriptor),
		.descriptor_version_major = 0,
		.descriptor_version_minor = 1,
		.hardware_version = 0x0008,
		.hardware_revision = 0,
		.serial = {'F', 'R', 'A', 'P', 'C', 'I', 'E', '0', 'P', 'R',
					'O', 'G', 'R', 'A', 'M', '0', 'M', 'E', '\0', '\0'},
		.descriptor_length = sizeof(struct default_pcie_thermal_cfg) - sizeof(struct gpu_cfg_descriptor),
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
	.vendor = GPU_PCIE_ACCESSORY,

	/* Power enable for 12V */
	.hdr4 = {.block_type = GPUCFG_TYPE_GPIO, .block_length = sizeof(struct gpu_cfg_gpio) * 8},
	.gpio0 = {.gpio = GPU_1G1_GPIO0_EC, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S3},
	/* Power enable for 3.3V */
	.gpio1 = {.gpio = GPU_1H1_GPIO1_EC, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S3},
	/* UNUSED */
	.gpio2 = {.gpio = GPU_2A2_GPIO2_EC, .function = GPIO_FUNC_UNUSED, .flags = GPIO_INPUT, .power_domain = POWER_G3},
	/* UNUSED */
	.gpio3 = {.gpio = GPU_2L7_GPIO3_EC, .function = GPIO_FUNC_UNUSED, .flags = GPIO_INPUT, .power_domain = POWER_G3},
	/* set mux configuration on mainboard for SSD */
	.gpio_edpaux = {.gpio = GPU_PCIE_MUX_SEL, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S3},
	/* GPU_VSYS_EN */
	.gpio_vsys = {.gpio = GPU_VSYS_EN, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S3},

	.gpio_fan = {.gpio = GPU_FAN_EN, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S0},

	.gpu_3v_5v_en = {.gpio = GPU_3V_5V_EN, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S5},

	.hdr5 = {.block_type = GPUCFG_TYPE_THERMAL_SENSOR, .block_length = sizeof(struct gpu_cfg_thermal)},
	.therm = {.thermal_type = GPU_THERM_F75303, .address = 0x4D},

	.hdr6 = {.block_type = GPUCFG_TYPE_CUSTOM_TEMP, .block_length = sizeof(struct gpu_cfg_custom_temp) * 3},
	.custom_temp1 = {.idx = CUSTOM_TEMP_gpu_ambient_f75303, .temp_fan_off = C_TO_K(50), .temp_fan_max = C_TO_K(80)},
	.custom_temp2 = {.idx = CUSTOM_TEMP_temp_sensor_gpu, .temp_fan_off = C_TO_K(50), .temp_fan_max = C_TO_K(80)},
	.custom_temp3 = {.idx = CUSTOM_TEMP_gpu_vram_f75303, .temp_fan_off = C_TO_K(50), .temp_fan_max = C_TO_K(80)},

	.hdr7 = {.block_type = GPUCFG_TYPE_SUBSYS, .block_length = sizeof(struct gpu_subsys_serial)},
	.pcba_serial = {.gpu_subsys = GPU_PCB, .serial = {'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
					'X', '0', '0', 'P', 'C', 'B', '0', '0', '\0', '\0'},}
};


void pcie_thermal_write_pcb(char * serial)
{
	strncpy(pcie_thermal_cfg.pcba_serial.serial, serial, GPU_SERIAL_LEN);
}