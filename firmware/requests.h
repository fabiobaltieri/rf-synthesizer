/* requests */
#define CUSTOM_RQ_SEND_CONFIG	0x00
#define CUSTOM_RQ_GET_LD	0x01
#define CUSTOM_RQ_WAIT_LD	0x02

#define CUSTOM_RQ_RESET		0xff

/* config format */
enum adf_flags {
	/* R1 */
	ADF_PRESCALER_8_9	= 1 << 0,
	/* R2 */
	ADF_RMULT2_EN		= 1 << 1,
	ADF_RDIV2_EN		= 1 << 2,
	/* R4 */
	ADF_RF_OUT_EN		= 1 << 3,
	ADF_MUTE_TILL_LOCK_EN	= 1 << 4,
};

struct adf_config {
	uint16_t r0_fract;
	uint16_t r0_int;

	uint16_t r1_mod;

	uint16_t r2_r_cnt;
	uint8_t r2_charge_pump_current;
	uint8_t r2_muxout;
	uint8_t r2_noise_mode;

	uint8_t r4_rf_div_sel;
	uint8_t r4_band_sel_div;
	uint8_t r4_output_power;

	uint16_t flags;

} __attribute__((__packed__));
