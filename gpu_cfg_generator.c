#include <gpu_cfg_generator.h>
#include "crc.h"

#ifndef GIT_HASH
#define GIT_HASH "unknown"
#endif


static bool verbose = false;


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

void print_custom_temp(uint8_t block_length, struct gpu_cfg_custom_temp *block_body) {
	uint8_t blocks = block_length / sizeof(struct gpu_cfg_custom_temp);
	struct gpu_cfg_custom_temp *block;
	for (int i = 0; i < blocks; i++) {
		block = &block_body[i];

		printf("    ID:          %d\n", block->idx);
		printf("    Temp Fan Off:%d (%dC)\n", block->temp_fan_off, K_TO_C(block->temp_fan_off));
		printf("    Temp Fan Max:%d (%dC)\n", block->temp_fan_max, K_TO_C(block->temp_fan_max));
	}
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
			case GPIO_FUNC_UNUSED:
				printf("Unused\n");
				break;
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
			case GPIO_FUNC_IS_THROTTLING:
				printf ("Is Throttling\n");
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
		case PD_TYPE_CCG8S:
			printf("CCG8S\n");
			break;
		default:
			printf("Invalid (%d)\n", pd->gpu_pd_type);
			break;
	}
	printf("    Address:     0x%x\n", pd->address);
	printf("    Flags:       0x%x\n", pd->flags);
	printf("    PDO:         0x%x\n", pd->pdo);
	printf("    RDO:         0x%x\n", pd->rdo);
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
		case GPU_NV_GN22:
			printf("Nvidia GN22\n");
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
			printf("--- Offset 0x%lX \tLength %d\n",\
				offset + sizeof(descriptor),\
				block_header->block_length);

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
					printf("Custom Temp\n");
					print_custom_temp(block_header->block_length, (struct gpu_cfg_custom_temp *)block_body);
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

	if (offset > descriptor.descriptor_length) {
		printf("Error: Block length exceeds descriptor length\n");
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


	assert((descriptor->length + descriptor->descriptor_length) == len);

	printf("writing EEPROM to %s\n", outpath);

	fptr = fopen(outpath,"wb");
	fwrite(descriptor, len, 1, fptr);
	fclose(fptr);

}

struct eeprom_target {
	const char * name;
	struct gpu_cfg_descriptor * descriptor;
	size_t length;
	void (*pcbwritefn)(char * serial);
};

struct eeprom_target programming_targets [] = {
	{"AMDR23", (void*)&r23m_gpu_cfg, sizeof(r23m_gpu_cfg), r23m_gpu_cfg_write_pcb},
	{"NVGN22", (void*)&gn22_gpu_cfg, sizeof(gn22_gpu_cfg), gn22_gpu_cfg_write_pcb},
	{"NVGN22_NEWTHERMAL", (void*)&gn22_gpu_cfg_newthermal, sizeof(gn22_gpu_cfg_newthermal), gn22_newthermal_gpu_cfg_write_pcb},
	{"SSD", (void*)&ssd_cfg, sizeof(ssd_cfg)},
	{"PCIE", (void*)&pcie_accessory_cfg, sizeof(pcie_accessory_cfg)},
	{"PCIE_THERMAL_3RDPARTY", (void*)&pcie_thermal_cfg, sizeof(pcie_thermal_cfg)},
};

int main(int argc, char *argv[]) {
	int i;

	char *targetvalue = "";

	char *serialvalue = "";
	char *pcbvalue = NULL;
	char *outfilename = "eeprom.bin";
	char *infilename = NULL;
	int c;

	opterr = 0;

	while ((c = getopt (argc, argv, "hvs:p:o:i:t:")) != -1)
	switch (c)
	{
	case 't':
		targetvalue = optarg;
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
	case 'h':
		printf("Build: %s %s (%s)\n", __DATE__, __TIME__, GIT_HASH);
		printf("    -t: Allowed targets:\n");
		for(i = 0; i < (sizeof(programming_targets) / sizeof(struct eeprom_target)); i++)
		{
			printf("       %s\n", programming_targets[i].name);
		}
		printf("    -s <18 digit serial>\n");
		printf("    -p <18 digit PCB serial if defined>\n");
		printf("    -o <output file name>\n");
		printf("    -i <input file name> - this will parse the input file and print information\n");
		printf("    -v - Enable verbose mode, useful with -i\n");
		return 0;
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
	printf("Build: %s %s (%s)\n", __DATE__, __TIME__, GIT_HASH);

	if (infilename) {
		read_eeprom(infilename);
		return 0;
	}

	printf("Descriptor Version: %d %d\n", 0, 1);

	for(i = 0; i < (sizeof(programming_targets) / sizeof(struct eeprom_target)); i++)
	{
		if (strcmp(programming_targets[i].name, targetvalue) == 0)
		{
			printf("Will generate based on %s\n", programming_targets[i].name);
			if (pcbvalue) {
				if (programming_targets[i].pcbwritefn) {
					programming_targets[i].pcbwritefn(pcbvalue);
				} else {
					printf("This config does not support PCB Serial\n");
				}
			}
			program_eeprom(serialvalue, (void *)programming_targets[i].descriptor, programming_targets[i].length, outfilename);
			return 0;
		}
	}
	printf("Could not find a valid programming target, try -h for valid targets\n");

	return 1;
}
