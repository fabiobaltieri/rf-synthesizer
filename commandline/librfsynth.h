#define RFSYNTH_PRODUCT "rf-synthesizer"
#define RFSYNTH_CLKIN 10000000

struct adf_state {
        unsigned long clkin;
        unsigned      ref_div_factor;
        unsigned      ref_doubler_en;
        unsigned      ref_div2_en;

        unsigned long chspc; /* Channel Spacing */
        unsigned long fpfd; /* Phase Frequency Detector */
        unsigned long min_out_freq;
        unsigned      r0_fract;
        unsigned      r0_int;
        unsigned      r1_mod;
        unsigned      r1_prescaler;
        unsigned      r2_r_cnt;
        unsigned      r4_rf_div_sel;
        unsigned      r4_band_sel_div;
};

void rfsynth_send_reset(usb_dev_handle *handle);
int rfsynth_set_freq(struct adf_state *st, unsigned long long freq);
void rfsynth_set_cfg(struct adf_config *cfg, struct adf_state *st);
void rfsynth_send_config(usb_dev_handle *handle, struct adf_config *cfg);
int rfsynth_get_ld(usb_dev_handle *handle);
