#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "crc.h"
#include "gpio_defines.h"
#include "config_definition.h"
#define C_TO_K(temp_c) ((temp_c) + 273)
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
	((byte) & 0x80 ? '1' : '0'), \
	((byte) & 0x40 ? '1' : '0'), \
	((byte) & 0x20 ? '1' : '0'), \
	((byte) & 0x10 ? '1' : '0'), \
	((byte) & 0x08 ? '1' : '0'), \
	((byte) & 0x04 ? '1' : '0'), \
	((byte) & 0x02 ? '1' : '0'), \
	((byte) & 0x01 ? '1' : '0')

static bool verbose = false;

enum power_state {
	/* Steady states */
	POWER_G3 = 0, /*
		* System is off (not technically all the way into G3,
		which means totally unpowered...)
		*/
	POWER_S5, /* System is soft-off */
	POWER_S4, /* System is suspended to disk */
	POWER_S3, /* Suspend; RAM on, processor is asleep */
	POWER_S0, /* System is on */
#if CONFIG_AP_PWRSEQ_S0IX
	POWER_S0ix,
#endif
	/* Transitions */
	POWER_G3S5, /* G3 -> S5 (at system init time) */
	POWER_S5S3, /* S5 -> S3 (skips S4 on non-Intel systems) */
	POWER_S3S0, /* S3 -> S0 */
	POWER_S0S3, /* S0 -> S3 */
	POWER_S3S5, /* S3 -> S5 (skips S4 on non-Intel systems) */
	POWER_S5G3, /* S5 -> G3 */
	POWER_S3S4, /* S3 -> S4 */
	POWER_S4S3, /* S4 -> S3 */
	POWER_S4S5, /* S4 -> S5 */
	POWER_S5S4, /* S5 -> S4 */
#if CONFIG_AP_PWRSEQ_S0IX
	POWER_S0ixS0, /* S0ix -> S0 */
	POWER_S0S0ix, /* S0 -> S0ix */
#endif
};


struct default_gpu_cfg {
	struct gpu_cfg_descriptor descriptor;

	struct gpu_block_header hdr0;
	enum gpu_pcie_cfg pcie_cfg;

	struct gpu_block_header hdr1;
	struct gpu_cfg_fan fan0_cfg;

	struct gpu_block_header hdr2;
	struct gpu_cfg_fan fan1_cfg;

	struct gpu_block_header hdr3;
	enum gpu_vendor vendor;

	struct gpu_block_header hdr4;
	struct gpu_cfg_gpio     gpio0;
	struct gpu_cfg_gpio     gpio1;
	struct gpu_cfg_gpio     gpio2;
	struct gpu_cfg_gpio     gpio3;
	struct gpu_cfg_gpio     gpio_vsys;
	struct gpu_cfg_gpio     gpio_fan;
	struct gpu_cfg_gpio     gpu_3v_5v_en;

	struct gpu_block_header hdr5;
	struct gpu_subsys_pd    pd;

	struct gpu_block_header hdr6;
	struct gpu_cfg_thermal therm;

	struct gpu_block_header hdr7;
	struct gpu_cfg_custom_temp custom_temp;

	struct gpu_block_header hdr8;
	struct gpu_subsys_serial pcba_serial;


} __packed;

static struct default_gpu_cfg gpu_cfg = {
	.descriptor = {
		.magic = {0x32, 0xac, 0x00, 0x00},
		.length = sizeof(struct gpu_cfg_descriptor),
		.descriptor_version_major = 0,
		.descriptor_version_minor = 1,
		.hardware_version = 0x0008,
		.hardware_revision = 0,
		.serial = {'F', 'R', 'A', 'K', 'M', 'B', 'C', 'P', '8', '1',
					'3', '3', '1', 'A', 'S', 'S', 'Y', '0', '\0', '\0'},
		.descriptor_length = sizeof(struct default_gpu_cfg) - sizeof(struct gpu_cfg_descriptor),
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
	.vendor = GPU_AMD_R23M,

	.hdr4 = {.block_type = GPUCFG_TYPE_GPIO, .block_length = (sizeof(struct gpu_cfg_gpio) * 7)},
	/* Critical temperature fault input */
	.gpio0 = {.gpio = GPU_1G1_GPIO0_EC, .function = GPIO_FUNC_TEMPFAULT, .flags = GPIO_INPUT, .power_domain = POWER_S3},
	/* DP HPD status from PD */
	.gpio1 = {.gpio = GPU_1H1_GPIO1_EC, .function = GPIO_FUNC_HPD, .flags = GPIO_INPUT, .power_domain = POWER_S5},
	/* AC/DC mode setting */
	.gpio2 = {.gpio = GPU_2A2_GPIO2_EC, .function = GPIO_FUNC_ACDC, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S3},
	/* UNUSED */
	.gpio3 = {.gpio = GPU_2L7_GPIO3_EC, .function = GPIO_FUNC_UNUSED, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_G3},
	/* GPU_VSYS_EN */
	.gpio_vsys = {.gpio = GPU_VSYS_EN, .function = GPIO_FUNC_GPU_PWR, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S3},

	.gpio_fan = {.gpio = GPU_FAN_EN, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S0},

	.gpu_3v_5v_en = {.gpio = GPU_3V_5V_EN, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S5},

	.hdr5 = {.block_type = GPUCFG_TYPE_PD, .block_length = sizeof(struct gpu_subsys_pd)},
	.pd = {.gpu_pd_type = PD_TYPE_ETRON_EJ889I, .address = 0x60,
			.flags = 0, .pdo = 0, .rdo = 0, .power_domain = POWER_S5,
			.gpio_hpd = GPU_1H1_GPIO1_EC, .gpio_interrupt = GPU_1F2_I2C_S5_INT
	},

	.hdr6 = {.block_type = GPUCFG_TYPE_THERMAL_SENSOR, .block_length = sizeof(struct gpu_cfg_thermal)},
	.therm = {.thermal_type = GPU_THERM_F75303, .address = 0x4D},

	.hdr7 = {.block_type = GPUCFG_TYPE_CUSTOM_TEMP, .block_length = sizeof(struct gpu_cfg_custom_temp)},
	.custom_temp = {.idx = 2, .temp_fan_off = C_TO_K(47), .temp_fan_max = C_TO_K(62)},

	.hdr8 = {.block_type = GPUCFG_TYPE_SUBSYS, .block_length = sizeof(struct gpu_subsys_serial)},
	.pcba_serial = {.gpu_subsys = GPU_PCB, .serial = {'F', 'R', 'A', 'G', 'M', 'A', 'S', 'P', '8', '1',
					'3', '3', '1', 'P', 'C', 'B', '0', '0', '\0', '\0'},}
};

struct default_ssd_cfg {
	struct gpu_cfg_descriptor descriptor;

	struct gpu_block_header hdr0;
	enum gpu_pcie_cfg pcie_cfg;

	struct gpu_block_header hdr1;
	struct gpu_cfg_fan fan0_cfg;

	struct gpu_block_header hdr2;
	struct gpu_cfg_fan fan1_cfg;

	struct gpu_block_header hdr3;
	enum gpu_vendor vendor;

	struct gpu_block_header hdr4;
	struct gpu_cfg_gpio     gpio0;
	struct gpu_cfg_gpio     gpio1;
	struct gpu_cfg_gpio     gpio2;
	struct gpu_cfg_gpio     gpio3;
	struct gpu_cfg_gpio     gpio_edpaux;
	struct gpu_cfg_gpio     gpio_vsys;
	struct gpu_cfg_gpio     gpio_fan;
	struct gpu_cfg_gpio     gpu_3v_5v_en;

} __packed;

static struct default_ssd_cfg ssd_cfg = {
	.descriptor = {
		.magic = {0x32, 0xac, 0x00, 0x00},
		.length = sizeof(struct gpu_cfg_descriptor),
		.descriptor_version_major = 0,
		.descriptor_version_minor = 1,
		.hardware_version = 0x0008,
		.hardware_revision = 0,
		.serial = {'F', 'R', 'A', 'G', 'M', 'B', 'S', 'P', '8', '1',
					'3', '3', '1', 'D', 'U', 'M', 'M', 'Y', '\0', '\0'},
		.descriptor_length = sizeof(struct default_ssd_cfg) - sizeof(struct gpu_cfg_descriptor),
		.descriptor_crc32 = 0,
		.crc32 = 0
	},
	.hdr0 = {.block_type = GPUCFG_TYPE_PCIE, .block_length = sizeof(uint8_t)},
	.pcie_cfg = PCIE_4X2,

	.hdr1 = {.block_type = GPUCFG_TYPE_FAN, .block_length = sizeof(struct gpu_cfg_fan)},
	.fan0_cfg = {.idx = 0, .flags = 0, .min_rpm = 1000, .start_rpm = 1000, .max_rpm = 3700},

	.hdr2 = {.block_type = GPUCFG_TYPE_FAN, .block_length = sizeof(struct gpu_cfg_fan)},
	.fan1_cfg = {.idx = 1, .flags = 0, .min_rpm = 1000, .start_rpm = 1000, .max_rpm = 3700},

	.hdr3 = {.block_type = GPUCFG_TYPE_VENDOR, .block_length = sizeof(enum gpu_vendor)},
	.vendor = GPU_SSD,

	/* Power enable for SSD1 */
	.hdr4 = {.block_type = GPUCFG_TYPE_GPIO, .block_length = sizeof(struct gpu_cfg_gpio) * 8},
	.gpio0 = {.gpio = GPU_1G1_GPIO0_EC, .function = GPIO_FUNC_SSD1_POWER, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S3},
	/* Power enable for SSD2 */
	.gpio1 = {.gpio = GPU_1H1_GPIO1_EC, .function = GPIO_FUNC_SSD2_POWER, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S3},
	/* UNUSED */
	.gpio2 = {.gpio = GPU_2A2_GPIO2_EC, .function = GPIO_FUNC_UNUSED, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_G3},
	/* UNUSED */
	.gpio3 = {.gpio = GPU_2L7_GPIO3_EC, .function = GPIO_FUNC_UNUSED, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_G3},
	/* set mux configuration on mainboard for SSD */
	.gpio_edpaux = {.gpio = GPU_PCIE_MUX_SEL, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S3},
	/* GPU_VSYS_EN */
	.gpio_vsys = {.gpio = GPU_VSYS_EN, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S3},

	.gpio_fan = {.gpio = GPU_FAN_EN, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S0},

	.gpu_3v_5v_en = {.gpio = GPU_3V_5V_EN, .function = GPIO_FUNC_HIGH, .flags = GPIO_OUTPUT_LOW, .power_domain = POWER_S5},
};

void print_descriptor(struct gpu_cfg_descriptor *desc)
{

	if (verbose) {
		printf("Descriptor\n");
		printf("  Magic         %02X%02X%02X%02X\n", (uint8_t)desc->magic[0], (uint8_t)desc->magic[1], (uint8_t)desc->magic[2], (uint8_t)desc->magic[3]);
		printf("  Length:       %d\n", desc->length);
		printf("  Desc Version: %d.%d\n", desc->descriptor_version_major, desc->descriptor_version_minor);
		printf("  HW Version:   %04X\n", desc->hardware_version);
		printf("  HW Rev:       %d\n", desc->hardware_revision);
		printf("  Serialnum:    %s\n", desc->serial);
		printf("  Desc Length:  %d\n", desc->descriptor_length);
		printf("  Desc CRC32:   %08X\n", desc->descriptor_crc32);
		printf("  CRC32:        %08X\n", desc->crc32);
	} else {
		printf("Serialnum:   %s\n", desc->serial);
	}
}

void print_subsys(struct gpu_subsys_serial* subsys)
{
	printf("    Type:   ");
	switch (subsys->gpu_subsys) {
		case GPU_PCB:
				printf("PCB\n");
			break;
		case GPU_LEFT_FAN:
				printf("Left Fan\n");
			break;
		case GPU_RIGHT_FAN:
				printf("Right Fan\n");
			break;
		case GPU_HOUSING:
				printf("Housing\n");
			break;
		default:
				printf("???\n");
				break;
	}
	printf("    Serial: %s\n", subsys->serial);
}

void print_gpio(uint8_t block_length, struct gpu_cfg_gpio *block_body) {
	uint8_t blocks = block_length / sizeof(struct gpu_cfg_gpio);
	struct gpu_cfg_gpio *block;
	for (int i = 0; i < blocks; i++) {
		block = &block_body[i];
		printf("  GPIO %d\n", block->gpio);
		printf("    Name:        ");
		switch (block->gpio) {
			case GPU_1G1_GPIO0_EC:
				printf("GPU_1G1_GPIO0_EC\n");
				break;
			case GPU_1H1_GPIO1_EC:
				printf("GPU_1H1_GPIO1_EC\n");
				break;
			case GPU_2A2_GPIO2_EC:
				printf("GPU_2A2_GPIO2_EC\n");
				break;
			case GPU_2L7_GPIO3_EC:
				printf("GPU_2L7_GPIO3_EC\n");
				break;
			case GPU_2L5_TH_OVERTn:
				printf("GPU_2L5_TH_OVERTn\n");
				break;
			case GPU_1F2_I2C_S5_INT:
				printf("GPU_1F2_I2C_S5_INT\n");
				break;
			case GPU_1L1_DGPU_PWROK:
				printf("GPU_1L1_DGPU_PWROK\n");
				break;
			case GPU_1C3_ALW_CLK:
				printf("GPU_1C3_ALW_CLK\n");
				break;
			case GPU_1D3_ALW_DAT:
				printf("GPU_1D3_ALW_DAT\n");
				break;
			case GPU_1F3_MUX1:
				printf("GPU_1F3_MUX1\n");
				break;
			case GPU_1G3_MUX2:
				printf("GPU_1G3_MUX2\n");
				break;
			case GPU_2B5_ALERTn:
				printf("GPU_2B5_ALERTn\n");
				break;
			case GPU_EDP_MUX_SEL:
				printf("GPU_EDP_MUX_SEL\n");
				break;
			case GPU_ECPWM_EN:
				printf("GPU_ECPWM_EN\n");
				break;
			case GPU_PCIE_MUX_SEL:
				printf("GPU_PCIE_MUX_SEL\n");
				break;
			case GPU_VSYS_EN:
				printf("GPU_VSYS_EN\n");
				break;
			case GPU_VADP_EN:
				printf("GPU_VADP_EN\n");
				break;
			case GPU_FAN_EN:
				printf("GPU_FAN_EN\n");
				break;
			case GPU_3V_5V_EN:
				printf("GPU_3V_5V_EN\n");
				break;
			default:
				printf("Unknown\n");
			break;
		}
		printf("    Function:    ");
		switch (block->function) {
			case GPIO_FUNC_HIGH:
				printf("High\n");
				break;
			case GPIO_FUNC_TEMPFAULT:
				printf("Tempfault\n");
				break;
			case GPIO_FUNC_ACDC:
				printf("ACDC\n");
				break;
			case GPIO_FUNC_HPD:
				printf("HPD\n");
				break;
			case GPIO_FUNC_PD_INT:
				printf("PD_INT\n");
				break;
			case GPIO_FUNC_SSD1_POWER:
				printf("SSD1_POWER\n");
				break;
			case GPIO_FUNC_SSD2_POWER:
				printf("SSD2_POWER\n");
				break;
			case GPIO_FUNC_EC_PWM_EN:
				printf("EC_PWM_EN\n");
				break;
			case GPIO_FUNC_EDP_MUX_SEL:
				printf("EDP_MUX_SEL\n");
				break;
			case GPIO_FUNC_VSYS_EN:
				printf("VSYS_EN\n");
				break;
			case GPIO_FUNC_VADP_EN:
				printf("VADP_EN\n");
				break;
			case GPIO_FUNC_GPU_PWR:
				printf("GPU Power\n");
				break;
			default:
				printf("Unknown\n");
				break;
		}
		printf("    Flags:       (");
		if ((block->flags & GPIO_INPUT) != 0) {
			printf("Input,");
		}
		if ((block->flags & GPIO_OUTPUT) != 0) {
			printf("Output,");
		}
		if ((block->flags & GPIO_OUTPUT_INIT_LOW) != 0) {
			printf("Low,");
		}
		if ((block->flags & GPIO_OUTPUT_INIT_HIGH) != 0) {
			printf("High,");
		}
		if ((block->flags & GPIO_OUTPUT_INIT_LOGICAL) != 0) {
			printf("Logical,");
		}
		printf(")\n");
		// printf(""BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN"\n",
		// 	BYTE_TO_BINARY(block->flags),
		// 	BYTE_TO_BINARY((block->flags >> 8) & 0xFF),
		// 	BYTE_TO_BINARY((block->flags >> 16) & 0xFF),
		// 	BYTE_TO_BINARY((block->flags >> 24) & 0xFF)
		// 	);
		
		printf("    Power Domain:");
		switch (block->power_domain) {
			case POWER_G3:
				printf("G3\n");
				break;
			case POWER_S5:
				printf("S5\n");
				break;
			case POWER_S4:
				printf("S4\n");
				break;
			case POWER_S3:
				printf("S3\n");
				break;
			case POWER_S0:
				printf("S0\n");
				break;
#if CONFIG_AP_PWRSEQ_S0IX
			case POWER_S0ix:
				printf("S0ix\n");
				break;
#endif
			case POWER_G3S5:
				printf("G3S5\n");
				break;
			case POWER_S5S3:
				printf("S5S3\n");
				break;
			case POWER_S3S0:
				printf("S3S0\n");
				break;
			case POWER_S0S3:
				printf("S0S3\n");
				break;
			case POWER_S3S5:
				printf("S3S5\n");
				break;
			case POWER_S5G3:
				printf("S5G3\n");
				break;
			case POWER_S3S4:
				printf("S3S4\n");
				break;
			case POWER_S4S3:
				printf("S4S3\n");
				break;
			case POWER_S4S5:
				printf("S4S5\n");
				break;
			case POWER_S5S4:
				printf("S5S4\n");
				break;
#if CONFIG_AP_PWRSEQ_S0IX
			case POWER_S0ixS0:
				printf("S0ixS0\n");
				break;
			case POWER_S0S0ix:
				printf("S0S0ix\n");
				break;
#endif
			default:
				printf("Unknown\n");
				break;
		}
	}
}

void print_pd(struct gpu_subsys_pd *pd) {
	printf("    Type:   ");
	switch (pd->gpu_pd_type) {
		case PD_TYPE_ETRON_EJ889I:
			printf("EJ899I\n");
			break;
		default:
			printf("Invalid (%d)\n", pd->gpu_pd_type);
			break;
	}
	printf("    Address:     %d\n", pd->address);
	printf("    Flags:       %d\n", pd->flags);
	printf("    PDO:         %d\n", pd->pdo);
	printf("    RDO:         %d\n", pd->rdo);
	printf("    Power Domain:%d\n", pd->power_domain);
	printf("    GPIO HPD:    %d\n", pd->gpio_hpd);
	printf("    GPIO INT:    %d\n", pd->gpio_interrupt);
}

void print_vendor(enum gpu_vendor vendor) {
	switch (vendor) {
		case GPU_VENDOR_INITIALIZING:
			printf("Vendor Initializing\n");
			break;
		case GPU_FAN_ONLY:
			printf("Fan Only\n");
			break;
		case GPU_AMD_R23M:
			printf("AMD R23M GPU\n");
			break;
		case GPU_SSD:
			printf("SSD\n");
			break;
		case GPU_PCIE_ACCESSORY:
			printf("PCI-E Accessory\n");
			break;
		default:
			printf("Invalid (%d)\n", vendor);
			break;
	}
}


void read_eeprom(const char * infilename)
{
	FILE *fptr;
	fptr = fopen(infilename,"rb");

	struct gpu_cfg_descriptor descriptor;
	fread((void *)&descriptor, sizeof(descriptor), 1, fptr);

	print_descriptor(&descriptor);

	void *blocks = malloc(descriptor.descriptor_length);
	if (!blocks) {
		fclose(fptr);
		return;
	}
	fread(blocks, descriptor.descriptor_length, 1, fptr);
	fclose(fptr);

	int offset = 0;
	int n = 0;
	struct gpu_block_header *block_header;
	while (offset < descriptor.descriptor_length) {
		block_header = (struct gpu_block_header *)(blocks + offset);
		void *block_body = blocks + offset + sizeof(struct gpu_block_header);

		if (verbose) {
			uint8_t *pcie;
			struct gpu_cfg_fan *fan;
			struct gpu_cfg_power *power;
			struct gpu_cfg_battery *battery;
			struct gpu_cfg_custom_temp *custom_temp;
			printf("---\n");
			// printf("Block %d\n", n);
			// printf("  Length: %d\n", block_header->block_length);
			printf("  Type:   ");
			switch (block_header->block_type) {
				case GPUCFG_TYPE_UNINITIALIZED:
					printf("Uninitialized\n");
					break;
				case GPUCFG_TYPE_GPIO:
					printf("GPIO\n");
					print_gpio(block_header->block_length, (struct gpu_cfg_gpio *)block_body);
					break;
				case GPUCFG_TYPE_THERMAL_SENSOR:
					printf("Thermal Sensor\n");
					if (*((enum gpu_thermal_sensor *)block_body) == GPU_THERM_F75303) {
						printf("    F75303\n");
					} else {
						printf("    Invalid\n");
					}
					break;
				case GPUCFG_TYPE_FAN:
					fan = block_body;
					printf("Fan\n");
					printf("    ID:        %d\n", fan->idx);
					printf("    Flags:     %d\n", fan->flags);
					printf("    Min RPM:   %d\n", fan->min_rpm);
					printf("    Min Temp:  %d\n", fan->min_temp);
					printf("    Start RPM: %d\n", fan->start_rpm);
					printf("    Max RPM:   %d\n", fan->max_rpm);
					printf("    Max Temp:  %d\n", fan->max_temp);
					break;
				case GPUCFG_TYPE_POWER:
					power = block_body;
					printf("Power\n");
					printf("    Device ID:   %d\n", power->device_idx);
					printf("    Battery:     %d\n", power->battery_power);
					printf("    Average:     %d\n", power->average_power);
					printf("    Long Term:   %d\n", power->long_term_power);
					printf("    Short Term:  %d\n", power->short_term_power);
					printf("    Peak:        %d\n", power->peak_power);
					break;
				case GPUCFG_TYPE_BATTERY:
					battery = block_body;
					printf("Battery\n");
					printf("    Max Current: %d\n", battery->max_current);
					printf("    Max Voltage: %dmV\n", battery->max_mv);
					printf("    Min Voltage: %dmV\n", battery->min_mv);
					printf("    Max Charge I:%d\n", battery->max_charge_current);
					break;
				case GPUCFG_TYPE_PCIE:
					printf("PCI-E\n");
					pcie = block_body;
					switch (*pcie) {
						case PCIE_8X1:
							printf("    Lanes: 8X1\n");
							break;
						case PCIE_4X1:
							printf("    Lanes: 4X1\n");
							break;
						case PCIE_4X2:
							printf("    Lanes: 4X2\n");
							break;
						default:
							printf("    Invalid (%d)\n", *pcie);
							break;
					}
					break;
				case GPUCFG_TYPE_DPMUX:
					printf("DP-MUX\n");
					// TODO: Decode. Unused so far
					break;
				case GPUCFG_TYPE_POWEREN:
					printf("POWER-EN\n");
					// TODO: Decode. Unused so far
					break;
				case GPUCFG_TYPE_SUBSYS:
					printf("Subsystem\n");
					print_subsys((struct gpu_subsys_serial *)block_body);
					break;
				case GPUCFG_TYPE_VENDOR:
					printf("Vendor\n");
					printf("  Value:  ");
					print_vendor(*(enum gpu_vendor *) block_body);
					break;
				case GPUCFG_TYPE_PD:
					printf("PD\n");
					print_pd((struct gpu_subsys_pd *) block_body);
					break;
				case GPUCFG_TYPE_GPUPWR:
					printf("GPU Power\n");
					// TODO: Decode. Unused so far
					break;
				case GPUCFG_TYPE_CUSTOM_TEMP:
					custom_temp = block_body;
					printf("Custom Temp\n");
					printf("    ID:          %d\n", custom_temp->idx);
					printf("    Temp Fan Off:%d\n", custom_temp->temp_fan_off);
					printf("    Temp Fan Max:%d\n", custom_temp->temp_fan_max);
					break;
				default:
					printf("Unknown\n");
					break;
			}
		} else {
			if (block_header->block_type == GPUCFG_TYPE_SUBSYS) {
				struct gpu_subsys_serial *subsys = block_body;
				if (subsys->gpu_subsys == GPU_PCB) {
					printf("PCBA Serial: %s\n", subsys->serial);
				}
			}
			if (block_header->block_type == GPUCFG_TYPE_VENDOR) {
				printf("Type:        ");
				print_vendor(*(enum gpu_vendor *) block_body);
			}
		}

		offset += sizeof(struct gpu_block_header) + block_header->block_length;
		n++;
	}

	free(blocks);
}

void program_eeprom(const char * serial, struct gpu_cfg_descriptor * descriptor, size_t len, const char * outpath)
{
	crc_t crc;
	FILE *fptr;
	printf("generating EEPROM\n");
	memset(descriptor->serial, 0x00, GPU_SERIAL_LEN);
	strncpy(descriptor->serial, serial, GPU_SERIAL_LEN);

	crc = crc_init();
	crc = crc_update(crc, (uint8_t *)descriptor + sizeof(struct gpu_cfg_descriptor), len - sizeof(struct gpu_cfg_descriptor));
	descriptor->descriptor_crc32 = crc_finalize(crc);

	crc = crc_init();
	crc = crc_update(crc, descriptor, sizeof(struct gpu_cfg_descriptor)-sizeof(uint32_t));
	descriptor->crc32 = crc_finalize(crc);

	printf("writing EEPROM to %s\n", outpath);

	fptr = fopen(outpath,"wb");
	fwrite(descriptor, len, 1, fptr);
	fclose(fptr);

}

int main(int argc, char *argv[]) {
	int gpuflag = 0;
	int ssdflag = 0;
	char *serialvalue = NULL;
	char *pcbvalue = NULL;
	char *outfilename = "eeprom.bin";
	char *infilename = NULL;
	int c;

	opterr = 0;

	while ((c = getopt (argc, argv, "gdvs:p:o:i:")) != -1)
	switch (c)
	{
	case 'g':
		gpuflag = 1;
		break;
	case 'd':
		ssdflag = 1;
		break;
	case 's':
		serialvalue = optarg;
		break;
	case 'p':
		pcbvalue = optarg;
		break;
	case 'o':
		outfilename = optarg;
		break;
	case 'i':
		infilename = optarg;
		break;
	case 'v':
		verbose = true;
		break;
	case '?':
		if (optopt == 'c')
			fprintf (stderr, "Option -%c requires an argument.\n", optopt);
		else if (isprint (optopt))
			fprintf (stderr, "Unknown option `-%c'.\n", optopt);
		else
			fprintf (stderr,
				"Unknown option character `\\x%x'.\n",
				optopt);
		return 1;
	default:
		abort ();
	}
	printf("Build: %s %s\n", __DATE__, __TIME__);

	if (infilename) {
		read_eeprom(infilename);
		return 0;
	}

	printf("Descriptor Version: %d %d\n", 0, 1);

	printf ("gpu = %d, ssd = %d, module SN = %s pcb SN = %s output file = %s\n",
		gpuflag, ssdflag, serialvalue, pcbvalue, outfilename);

	if (gpuflag) {
		if (pcbvalue) {
			strncpy(gpu_cfg.pcba_serial.serial, pcbvalue, GPU_SERIAL_LEN);
		}
		program_eeprom(serialvalue, (void *)&gpu_cfg, sizeof(gpu_cfg), outfilename);
	}

	if (ssdflag) {
		program_eeprom(serialvalue, (void *)&ssd_cfg, sizeof(ssd_cfg), outfilename);
	}

	return 0;
}
