static constexpr uint8_t GPRs_size = 32;
static constexpr uint8_t io_size = 64;
static constexpr sizemcu_t io_start = 0x20;
static constexpr uint8_t ext_io_size = 160;
static constexpr sizemcu_t ext_io_start = 0x60;
static constexpr sizemcu_t ISRAM_size = 2560;
static constexpr at_addr_t ISRAM_start = 0x100;

static constexpr sizemcu_t total_io_size = io_size + ext_io_size;
static constexpr sizemcu_t data_size = GPRs_size + io_size + ext_io_size + ISRAM_size;
static constexpr sizemcu_t SP_initaddr = ISRAM_start + ISRAM_size - 1;

static constexpr sizemcu_t eeprom_size = 1024;

static constexpr at_addr_t SPH = 0x5e, SPL = 0x5d;

static constexpr at_addr_t SREG = 0x5f;
static constexpr uint8_t SREG_I = 7, SREG_T = 6, SREG_H = 5, SREG_S = 4, SREG_V = 3, SREG_N = 2, SREG_Z = 1, SREG_C = 0; //should probably be an enum, but hooooow do you make it count down :c

static constexpr at_addr_t RAMPZ = 0x5b;

static constexpr at_addr_t MCUSR = 0x54;
static constexpr uint8_t MCUSR_USBRF = 5, MCUSR_JTRF = 4, MCUSR_WDRF = 3, MCUSR_BORF = 2, MCUSR_EXTRF = 1, MCUSR_PORF = 0;

static constexpr at_addr_t PORTB = 0x25, PORTC = 0x28, PORTD = 0x2B, PORTE = 0x2E;
static constexpr at_addr_t PINB = 0x23, PINC = 0x26, PIND = 0x29, PINE = 0x2C, PINF = 0x2F;

static constexpr at_addr_t EEARH = 0x42, EEARL = 0x41, EEDR = 0x40, EECR = 0x3F;
static constexpr uint8_t EECR_EEPM1 = 5, EECR_EEPM0 = 4, EECR_EERIE = 3, EECR_EEMPE = 2, EECR_EEPE = 1, EECR_EERE = 0;

static constexpr at_addr_t PLLCSR = 0x49;
static constexpr uint8_t PLLCSR_PINDIV = 4, PLLCSR_PLLE = 1, PLLCSR_PLOCK = 0;

static constexpr at_addr_t SPDR = 0x4E, SPSR = 0x4D, SPCR = 0x4C;
static constexpr uint8_t SPSR_SPIF = 7;

static constexpr at_addr_t PRR0 = 0x64; //Power Reduction Register
static constexpr at_addr_t TCCR0A = 0x44, TCCR0B = 0x45, TCNT0 = 0x46, TIFR0 = 0x35, TIMSK0 = 0x6E;
static constexpr uint8_t TIFR0_TOV0 = 0;
static constexpr uint8_t TIMSK0_TOIE0 = 0;

static constexpr at_addr_t SMCR = 0x53;