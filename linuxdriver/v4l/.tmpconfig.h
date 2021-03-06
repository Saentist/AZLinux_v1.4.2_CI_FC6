/*
 * Automatically generated C config: don't edit
 * Linux kernel version: 
 * Thu Mar  8 17:41:46 2007
 */
#define AUTOCONF_INCLUDED
#define CONFIG_SONYPI_MODULE 1
#define CONFIG_USB 1
#define CONFIG_PARPORT_MODULE 1
#define CONFIG_MODULES 1
#undef CONFIG_BROKEN
#undef CONFIG_PPC_PMAC
#define CONFIG_PROC_FS 1
#define CONFIG_FW_LOADER_MODULE 1
#define CONFIG_I2C_ALGOBIT_MODULE 1
#undef CONFIG_PLAT_M32700UT
#define CONFIG_NET 1
#undef CONFIG_SOUND_PRIME
#define CONFIG_INET 1
#define CONFIG_CRC32_MODULE 1
#undef CONFIG_SGI_IP22
#define CONFIG_SYSFS 1
#define CONFIG_I2C_MODULE 1
#define CONFIG_ISA 1
#define CONFIG_PCI 1
#undef CONFIG_SOUND_ACI_MIXER
#define CONFIG_SND_PCM_MODULE 1
#define CONFIG_PARPORT_1284 1
#define CONFIG_SND_MODULE 1
#define CONFIG_EXPERIMENTAL 1
#undef CONFIG_M32R
#undef CONFIG_I2C_ALGO_SGI
#define CONFIG_VIDEO_KERNEL_VERSION 1

/*
 * Multimedia devices
 */
#define CONFIG_VIDEO_DEV_MODULE 1
#define CONFIG_VIDEO_V4L1 1
#define CONFIG_VIDEO_V4L1_COMPAT 1
#define CONFIG_VIDEO_V4L2 1

/*
 * Video Capture Adapters
 */

/*
 * Video Capture Adapters
 */
#undef CONFIG_VIDEO_ADV_DEBUG
#undef CONFIG_VIDEO_VIVI
#undef CONFIG_VIDEO_BT848
#undef CONFIG_VIDEO_PMS
#undef CONFIG_VIDEO_BWQCAM
#undef CONFIG_VIDEO_CQCAM
#undef CONFIG_VIDEO_W9966
#undef CONFIG_VIDEO_CPIA
#undef CONFIG_VIDEO_CPIA2
#undef CONFIG_VIDEO_SAA5246A
#undef CONFIG_VIDEO_SAA5249
#undef CONFIG_TUNER_3036
#undef CONFIG_VIDEO_STRADIS
#undef CONFIG_VIDEO_ZORAN
#undef CONFIG_VIDEO_MEYE
#undef CONFIG_VIDEO_SAA7134
#undef CONFIG_VIDEO_MXB
#undef CONFIG_VIDEO_DPC
#undef CONFIG_VIDEO_HEXIUM_ORION
#undef CONFIG_VIDEO_HEXIUM_GEMINI
#define CONFIG_VIDEO_CX88_MODULE 1
#undef CONFIG_VIDEO_CX88_ALSA
#undef CONFIG_VIDEO_CX88_BLACKBIRD
#define CONFIG_VIDEO_CX88_DVB_MODULE 1
#undef CONFIG_VIDEO_CX88_VP3054

/*
 * Encoders and Decoders
 */
#define CONFIG_VIDEO_MSP3400_MODULE 1
#undef CONFIG_VIDEO_CS53L32A
#undef CONFIG_VIDEO_TLV320AIC23B
#undef CONFIG_VIDEO_WM8775
#undef CONFIG_VIDEO_WM8739
#undef CONFIG_VIDEO_CX2341X
#undef CONFIG_VIDEO_CX25840
#undef CONFIG_VIDEO_SAA711X
#undef CONFIG_VIDEO_SAA7127
#undef CONFIG_VIDEO_UPD64031A
#undef CONFIG_VIDEO_UPD64083

/*
 * V4L USB devices
 */
#undef CONFIG_VIDEO_PVRUSB2
#undef CONFIG_VIDEO_EM28XX
#undef CONFIG_USB_VICAM
#undef CONFIG_USB_IBMCAM
#undef CONFIG_USB_KONICAWC
#undef CONFIG_USB_QUICKCAM_MESSENGER
#undef CONFIG_USB_ET61X251
#undef CONFIG_VIDEO_OVCAMCHIP
#undef CONFIG_USB_W9968CF
#undef CONFIG_USB_OV511
#undef CONFIG_USB_SE401
#undef CONFIG_USB_SN9C102
#undef CONFIG_USB_STV680
#undef CONFIG_USB_ZC0301
#undef CONFIG_USB_PWC

/*
 * Radio Adapters
 */
#undef CONFIG_RADIO_CADET
#undef CONFIG_RADIO_RTRACK
#undef CONFIG_RADIO_RTRACK2
#undef CONFIG_RADIO_AZTECH
#undef CONFIG_RADIO_GEMTEK
#undef CONFIG_RADIO_GEMTEK_PCI
#undef CONFIG_RADIO_MAXIRADIO
#undef CONFIG_RADIO_MAESTRO
#undef CONFIG_RADIO_SF16FMI
#undef CONFIG_RADIO_SF16FMR2
#undef CONFIG_RADIO_TERRATEC
#undef CONFIG_RADIO_TRUST
#undef CONFIG_RADIO_TYPHOON
#undef CONFIG_RADIO_ZOLTRIX
#undef CONFIG_USB_DSBR

/*
 * Digital Video Broadcasting Devices
 */
#define CONFIG_DVB 1
#define CONFIG_DVB_CORE_MODULE 1
#undef CONFIG_DVB_CORE_ATTACH

/*
 * Supported SAA7146 based PCI Adapters
 */
#undef CONFIG_DVB_AV7110
#undef CONFIG_DVB_BUDGET
#undef CONFIG_DVB_BUDGET_CI
#undef CONFIG_DVB_BUDGET_AV

/*
 * Supported USB Adapters
 */
#undef CONFIG_DVB_USB
#undef CONFIG_DVB_TTUSB_BUDGET
#undef CONFIG_DVB_TTUSB_DEC
#undef CONFIG_DVB_CINERGYT2

/*
 * Supported FlexCopII (B2C2) Adapters
 */
#undef CONFIG_DVB_B2C2_FLEXCOP

/*
 * Supported BT878 Adapters
 */

/*
 * Supported Pluto2 Adapters
 */
#undef CONFIG_DVB_PLUTO2

/*
 * Supported Mantis Adapters
 */
#define CONFIG_DVB_MANTIS_MODULE 1

/*
 * Supported Mantis CI
 */
#define CONFIG_DVB_MANTIS_CI_MODULE 1

/*
 * Supported DVB Frontends
 */

/*
 * Customise DVB Frontends
 */
#define CONFIG_DVB_FE_CUSTOMISE 1

/*
 * DVB-S2 frontends
 */
#define CONFIG_DVB_STB0899_MODULE 1

/*
 * DVB-S (satellite) frontends
 */
#define CONFIG_DVB_STV0299_MODULE 1
#define CONFIG_DVB_MB86A16_MODULE 1
#define CONFIG_DVB_CX24110_MODULE 1
#define CONFIG_DVB_CX24123_MODULE 1
#undef CONFIG_DVB_TDA8083
#undef CONFIG_DVB_MT312
#undef CONFIG_DVB_VES1X93
#undef CONFIG_DVB_S5H1420
#undef CONFIG_DVB_TDA10086

/*
 * DVB-T (terrestrial) frontends
 */
#define CONFIG_DVB_SP8870_MODULE 1
#define CONFIG_DVB_SP887X_MODULE 1
#define CONFIG_DVB_CX22700_MODULE 1
#define CONFIG_DVB_CX22702_MODULE 1
#undef CONFIG_DVB_L64781
#undef CONFIG_DVB_TDA1004X
#define CONFIG_DVB_NXT6000_MODULE 1
#define CONFIG_DVB_MT352_MODULE 1
#define CONFIG_DVB_ZL10353_MODULE 1
#undef CONFIG_DVB_DIB3000MB
#undef CONFIG_DVB_DIB3000MC

/*
 * DVB-C (cable) frontends
 */
#undef CONFIG_DVB_VES1820
#undef CONFIG_DVB_TDA10021
#define CONFIG_DVB_CU1216_MODULE 1
#undef CONFIG_DVB_STV0297

/*
 * ATSC (North American/Korean Terrestrial/Cable DTV) frontends
 */
#undef CONFIG_DVB_NXT200X
#define CONFIG_DVB_OR51211_MODULE 1
#define CONFIG_DVB_OR51132_MODULE 1
#undef CONFIG_DVB_BCM3510
#undef CONFIG_DVB_LGDT330X

/*
 * Tuners/PLL support
 */
#define CONFIG_DVB_PLL_MODULE 1
#undef CONFIG_DVB_TDA826X
#undef CONFIG_DVB_TUNER_MT2060

/*
 * Miscellaneous devices
 */
#undef CONFIG_DVB_LNBP21
#define CONFIG_DVB_ISL6421_MODULE 1
#define CONFIG_VIDEO_TUNER_MODULE 1
#define CONFIG_VIDEO_BUF_MODULE 1
#define CONFIG_VIDEO_BUF_DVB_MODULE 1
#define CONFIG_VIDEO_BTCX_MODULE 1
#define CONFIG_VIDEO_IR_MODULE 1
#define CONFIG_VIDEO_TVEEPROM_MODULE 1
#undef CONFIG_USB_DABUSB
