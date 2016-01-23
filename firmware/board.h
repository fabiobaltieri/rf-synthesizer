/* LEDs */

#define LED_A_PORT    PORTC
#define LED_A_DDR     DDRC
#define LED_A         PC3

#define LED_B_PORT    PORTC
#define LED_B_DDR     DDRC
#define LED_B         PC4

#define led_a_on()     LED_A_PORT |=  _BV(LED_A)
#define led_a_off()    LED_A_PORT &= ~_BV(LED_A)
#define led_a_toggle() LED_A_PORT ^=  _BV(LED_A)

#define led_b_on()     LED_B_PORT |=  _BV(LED_B)
#define led_b_off()    LED_B_PORT &= ~_BV(LED_B)
#define led_b_toggle() LED_B_PORT ^=  _BV(LED_B)

#define led_init()				\
	do {					\
		LED_A_DDR |= _BV(LED_A);	\
		LED_B_DDR |= _BV(LED_B);	\
	} while (0);

/* ADF4351 */

#define ADF_PDBRF_PORT PORTC
#define ADF_PDBRF_DDR  DDRC
#define ADF_PDBRF      PC1

#define adf_pdbrf_h() (ADF_PDBRF_PORT |=  _BV(ADF_PDBRF))
#define adf_pdbrf_l() (ADF_PDBRF_PORT &= ~_BV(ADF_PDBRF))

#define ADF_CE_PORT PORTB
#define ADF_CE_DDR  DDRB
#define ADF_CE      PB1

#define adf_ce_h() (ADF_CE_PORT |=  _BV(ADF_CE))
#define adf_ce_l() (ADF_CE_PORT &= ~_BV(ADF_CE))

#define ADF_MUXOUT_PIN PINC
#define ADF_MUXOUT     PC0

#define adf_muxout() (ADF_MUXOUT_PIN & _BV(ADF_MUXOUT))

#define ADF_LD_PIN PINC
#define ADF_LD     PC2

#define adf_ld() (ADF_LD_PIN & _BV(ADF_LD))

/* SPI */

#define SPI_DDR  DDRB
#define SPI_PORT PORTB
#define SPI_SS   PB2
#define SPI_SCK  PB5
#define SPI_MOSI PB3
#define SPI_MISO PB4

#define adf_cs_h() (SPI_PORT |=  _BV(SPI_SS))
#define adf_cs_l() (SPI_PORT &= ~_BV(SPI_SS))
