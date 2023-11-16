#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
#include <ctype.h>

#include "crc.h"
#include "gpio_defines.h"
#include "config_definition.h"
#define C_TO_K(temp_c) ((temp_c) + 273)

enum power_state {
	/* Steady states */
	POWER_G3 = 0, /*
		       * System is off (not technically all the way into G3,
		       * which means totally unpowered...)
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
	.custom_temp = {.idx = 2, .temp_fan_off = C_TO_K(48), .temp_fan_max = C_TO_K(69)},

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


void program_eeprom(const char * serial, struct gpu_cfg_descriptor * descriptor, size_t len)
{
	crc_t crc;
	size_t addr = 0;
	int d, i;
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

	printf("writing EEPROM to %s\n", "eeprom.bin");

  fptr = fopen("eeprom.bin","wb");
  fwrite(descriptor, len, 1, fptr); 
  fclose(fptr); 

}

int main(int argc, char *argv[]) {
 int gpuflag = 0;
  int ssdflag = 0;
  char *serialvalue = NULL;
  char *pcbvalue = NULL;

  int index;
  int c;

  opterr = 0;

  while ((c = getopt (argc, argv, "gds:p:")) != -1)
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

  printf ("gpu = %d, ssd = %d, module SN = %s pcb SN = %s\n",
        gpuflag, ssdflag, serialvalue, pcbvalue);

  if (gpuflag) {
    if (pcbvalue) {
      strncpy(gpu_cfg.pcba_serial.serial, pcbvalue, GPU_SERIAL_LEN);
    }
    program_eeprom(serialvalue, (void *)&gpu_cfg, sizeof(gpu_cfg));
  }

  if (ssdflag) {
    program_eeprom(serialvalue, (void *)&ssd_cfg, sizeof(ssd_cfg));
  }
}
