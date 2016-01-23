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

/* SPI */

#define SPI_DDR  DDRB
#define SPI_PORT PORTB
#define SPI_SS   PB2
#define SPI_SCK  PB5
#define SPI_MOSI PB3
#define SPI_MISO PB4

#define can_cs_h() (SPI_PORT |=  _BV(SPI_SS))
#define can_cs_l() (SPI_PORT &= ~_BV(SPI_SS))

