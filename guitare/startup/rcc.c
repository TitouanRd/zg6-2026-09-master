/** 
  ==============================================================================
                      ##### RCC specific features #####
  ==============================================================================
    [..]  
      After reset the device is running from Internal High Speed oscillator 
      (HSI 16MHz) with Flash 0 wait state, Flash prefetch buffer, D-Cache 
      and I-Cache are disabled, and all peripherals are off except internal
      SRAM, Flash and JTAG.
      (+) There is no prescaler on High speed (AHB) and Low speed (APB) busses;
          all peripherals mapped on these busses are running at HSI speed.
      (+) The clock for all peripherals is switched off, except the SRAM and FLASH.
      (+) All GPIOs are in input floating state, except the JTAG pins which
          are assigned to be used for debug purpose.
    
    [..]          
      Once the device started from reset, the user application has to:        
      (+) Configure the clock source to be used to drive the System clock
          (if the application needs higher frequency/performance)
      (+) Configure the System clock frequency and Flash settings  
      (+) Configure the AHB and APB busses prescalers
      (+) Enable the clock for the peripheral(s) to be used
      (+) Configure the clock source(s) for peripherals which clocks are not
          derived from the System clock (I2S, RTC, ADC, USB OTG FS/SDIO/RNG)

                      ##### RCC Limitations #####
  ==============================================================================
    [..]  
      A delay between an RCC peripheral clock enable and the effective peripheral 
      enabling should be taken into account in order to manage the peripheral read/write 
      from/to registers.
      (+) This delay depends on the peripheral mapping.
      (+) If peripheral is mapped on AHB: the delay is 2 AHB clock cycle 
          after the clock enable bit is set on the hardware register
      (+) If peripheral is mapped on APB: the delay is 2 APB clock cycle 
          after the clock enable bit is set on the hardware register

    [..]  
      Possible Workarounds:
      (#) Enable the peripheral clock sometimes before the peripheral read/write 
          register is required.
      (#) For AHB peripheral, insert two dummy read to the peripheral register.
      (#) For APB peripheral, insert a dummy read to the peripheral register.
  */

#include "include/board.h"
#include "rcc.h"

/* Clock information */
Clock_t sysclks = {
	.main_osc_is_hse=0,
	.ahb_freq=0,
	.apb1_freq=0,
	.apb1_timer_freq=0,
	.apb2_freq=0,
	.apb2_timer_freq=0,
	.i2s_freq=0
	
};

enum rcc_osc {
	RCC_HSI,
	RCC_HSE,
	RCC_PLL,
	RCC_PLLI2S,
	RCC_LSI,
	RCC_LSE
};

/* HPRE: AHB high-speed prescaler */
#define RCC_CFGR_HPRE_DIV_NONE			0x0
#define RCC_CFGR_HPRE_DIV_2				(0x8 + 0)
#define RCC_CFGR_HPRE_DIV_4				(0x8 + 1)
#define RCC_CFGR_HPRE_DIV_8				(0x8 + 2)
#define RCC_CFGR_HPRE_DIV_16			(0x8 + 3)
#define RCC_CFGR_HPRE_DIV_64			(0x8 + 4)
#define RCC_CFGR_HPRE_DIV_128			(0x8 + 5)
#define RCC_CFGR_HPRE_DIV_256			(0x8 + 6)
#define RCC_CFGR_HPRE_DIV_512			(0x8 + 7)

/* PPRE1/2: APB high-speed prescalers */
#define RCC_CFGR_PPRE_DIV_NONE			0x0U
#define RCC_CFGR_PPRE_DIV_2				0x4U
#define RCC_CFGR_PPRE_DIV_4				0x5U
#define RCC_CFGR_PPRE_DIV_8				0x6U
#define RCC_CFGR_PPRE_DIV_16			0x7U

/* FLASH latency definitions */
#define FLASH_LATENCY_0WS				0x00000000U
#define FLASH_LATENCY_1WS				0x00000001U
#define FLASH_LATENCY_2WS				0x00000002U
#define FLASH_LATENCY_3WS				0x00000003U
#define FLASH_LATENCY_4WS				0x00000004U
#define FLASH_LATENCY_5WS				0x00000005U
#define FLASH_LATENCY_6WS				0x00000006U
#define FLASH_LATENCY_7WS				0x00000007U

/* RCC flags */
#define RCC_HSI_ON						(1U<<0)
#define RCC_HSI_RDY						(1U<<1)
#define RCC_HSE_ON						(1U<<16)
#define RCC_HSE_RDY						(1U<<17)
#define RCC_HSE_BYP						(1U<<18)
#define RCC_CSS_ON						(1U<<19)
#define RCC_PLL_ON						(1U<<24)
#define RCC_PLL_RDY						(1U<<25)
#define RCC_PLLI2S_ON					(1U<<26)
#define RCC_PLLI2S_RDY					(1U<<27)
#define RCC_LSI_ON						(1U<<0)
#define RCC_LSI_RDY						(1U<<1)
#define RCC_LSE_ON						(1U<<0)
#define RCC_LSE_RDY						(1U<<1)

struct ClockConfig_t {
	uint8_t		type;
	uint8_t		pll_src;
	uint16_t	plln;
	uint8_t		pllm;
	uint8_t		pllp;
	uint8_t		pllq;
	uint8_t		pllr;
	uint8_t		hpre;
	uint8_t		ppre1;
	uint8_t		ppre2;
	uint8_t		power_save;
	uint32_t	flash_cfg;
	uint32_t	ahb_freq;
	uint32_t	apb1_freq;
	uint32_t	apb2_freq;
};

static struct ClockConfig_t _clock_config[] = {
	{ /* HSE 8MHz */
		.type = RCC_HSE,
		.hpre = RCC_CFGR_HPRE_DIV_NONE,
		.ppre1 = RCC_CFGR_PPRE_DIV_NONE,
		.ppre2 = RCC_CFGR_PPRE_DIV_NONE,
	},
	{ /* HSE 8MHz --> 48MHz */
		.type = RCC_PLL,
		.pll_src = RCC_HSE,
		.pllm = 8,
		.plln = 96,
		.pllp = 2,
		.pllq = 2,
		.hpre = RCC_CFGR_HPRE_DIV_NONE,
		.ppre1 = RCC_CFGR_PPRE_DIV_4,
		.ppre2 = RCC_CFGR_PPRE_DIV_2,
		.power_save = 1,							/* HCLK <= 64MHz */
		.flash_cfg = FLASH_LATENCY_1WS,
		.ahb_freq  = 48000000,
		.apb1_freq = 12000000,
		.apb2_freq = 24000000,
	},
	{ /* HSE 8MHz --> 84MHz */
		.type = RCC_PLL,
		.pll_src = RCC_HSE,
		.pllm = 8,
		.plln = 336,
		.pllp = 4,
		.pllq = 7,
		.power_save = 2,							/* HCLK <= 84MHz */
		.hpre = RCC_CFGR_HPRE_DIV_NONE,
		.ppre1 = RCC_CFGR_PPRE_DIV_2,
		.ppre2 = RCC_CFGR_PPRE_DIV_NONE,
		.flash_cfg = FLASH_LATENCY_2WS,
		.ahb_freq  = 84000000,
		.apb1_freq = 42000000,
		.apb2_freq = 84000000
	},
	{ /* HSE 8MHz --> 96MHz */
		.type = RCC_PLL,
		.pll_src = RCC_HSE,
		.pllm = 8,
		.plln = 192,
		.pllp = 2,
		.pllq = 2,
		.power_save = 3,							/* HCLK <= 100MHz */
		.hpre = RCC_CFGR_HPRE_DIV_NONE,
		.ppre1 = RCC_CFGR_PPRE_DIV_2,
		.ppre2 = RCC_CFGR_PPRE_DIV_NONE,
		.flash_cfg = FLASH_LATENCY_3WS,
		.ahb_freq  = 96000000,
		.apb1_freq = 48000000,
		.apb2_freq = 96000000
	
	},
	{ /* HSI 16MHz */
		.type = RCC_HSI,
		.hpre = RCC_CFGR_HPRE_DIV_NONE,
		.ppre1 = RCC_CFGR_PPRE_DIV_NONE,
		.ppre2 = RCC_CFGR_PPRE_DIV_NONE,
	},
	{ /* HSI 16MHz --> 84MHz */
		.type = RCC_PLL,
		.pll_src = RCC_HSI,
		.pllm = 8,
		.plln = 336,
		.pllp = 4,
		.pllq = 7,
		.power_save = 2,							/* HCLK <= 84MHz */
		.hpre = RCC_CFGR_HPRE_DIV_NONE,
		.ppre1 = RCC_CFGR_PPRE_DIV_2,
		.ppre2 = RCC_CFGR_PPRE_DIV_NONE,
		.flash_cfg = FLASH_LATENCY_2WS,
		.ahb_freq  = 84000000,
		.apb1_freq = 42000000,
		.apb2_freq = 84000000
	}
};

static void rcc_osc_on(enum rcc_osc osc)
{
	switch (osc) {
	case RCC_HSI:
		if (!(_RCC->CR & RCC_HSI_ON)) {
			_RCC->CR |= RCC_HSI_ON;
			while ((_RCC->CR & RCC_HSI_RDY)==0);
		}
		break;
	case RCC_HSE:
		if (!(_RCC->CR & RCC_HSE_ON)) {
			_RCC->CR |= (RCC_HSE_ON | RCC_HSE_BYP | RCC_CSS_ON);
			while ((_RCC->CR & RCC_HSE_RDY)==0);
		}
		break;
	case RCC_PLL:
		if (!(_RCC->CR & RCC_PLL_ON)) {
			_RCC->CR |= RCC_PLL_ON;
			while ((_RCC->CR & RCC_PLL_RDY)==0);
		}
		break;
	case RCC_PLLI2S:
		if (!(_RCC->CR & RCC_PLLI2S_ON)) {
			_RCC->CR |= RCC_PLLI2S_ON;
			while ((_RCC->CR & RCC_PLLI2S_RDY)==0);
		}
		break;
	case RCC_LSI:
		if (!(_RCC->CSR & RCC_LSI_ON)) {
			_RCC->CSR |= RCC_LSI_ON;
			while ((_RCC->CSR & RCC_LSI_RDY)==0);
		}
		break;
	case RCC_LSE:
		if (!(_RCC->BDCR & RCC_LSE_ON)) {
			_RCC->BDCR |= RCC_LSE_ON;
			while ((_RCC->BDCR & RCC_LSE_RDY)==0);
		}
		break;
	}
}

static void rcc_osc_off(enum rcc_osc osc)
{
	switch (osc) {
	case RCC_HSI:
		_RCC->CR &= ~RCC_HSI_ON;
		break;
	case RCC_HSE:
		_RCC->CR &= ~(RCC_HSE_ON | RCC_HSE_BYP | RCC_CSS_ON);
		break;
	case RCC_PLL:
		_RCC->CR &= ~RCC_PLL_ON;
		break;
	case RCC_PLLI2S:
		_RCC->CR &= ~RCC_PLLI2S_ON;
		break;
	case RCC_LSI:
		_RCC->CSR &= ~RCC_LSI_ON;
		break;
	case RCC_LSE:
		_RCC->BDCR &= ~RCC_LSE_ON;
		break;
	}
}

static void rcc_set_sysclk(enum rcc_osc osc)
{
	_RCC->CFGR = (_RCC->CFGR & ~0x3) | (osc & 3);
	while (((_RCC->CFGR & 0xC)>>2) != osc);
}

void rcc_sys_clk_cfg(uint32_t config)
{
	struct ClockConfig_t *clk;
	
	if (config < CLOCK_CONFIG_END) {
		clk=&(_clock_config[config]);
	} else {
		clk=&(_clock_config[CLOCK_CONFIG_HSI_16MHz]);
	}
		
	if (clk->type == RCC_HSE) {			// HSE Clock
		rcc_osc_on(RCC_HSE);
		rcc_set_sysclk(RCC_HSE);
		rcc_osc_off(RCC_PLL);
		rcc_osc_off(RCC_HSI);
		sysclks.main_osc_is_hse=1;
	
	} else if (clk->type == RCC_PLL) {
		// enable PWR module clocking
		_RCC->APB1ENR |= 1<<28;
	
		if (clk->pll_src == RCC_HSE) {	// HSE Clock src
			rcc_osc_on(RCC_HSE);
			sysclks.main_osc_is_hse=1;
			
			// set HSE Clock as PLL src
			_RCC->PLLCFGR |= 1<<22;
		} else {						// Default: HSI Clock src
			rcc_osc_on(RCC_HSI);
			sysclks.main_osc_is_hse=0;
			
			// set HSI Clock as PLL src
			_RCC->PLLCFGR &= ~(1<<22);
		}
		
		// regulator voltage scaling : VOS[1:0], active when PLL is on
		//  0x1 -> Scale 3 mode: HCLK <= 64MHz
		//  0x2 -> Scale 2 mode: HCLK <= 84MHz
		//  0x3 -> Scale 1 mode: HCLK <= 100MHz
		_PWR->CR = (_PWR->CR & (~(3<<28))) | (clk->power_save << 28);
		
		// configure prescalers for 
		//  AHB: AHBCLK > 25MHz
		//  APB1: APB1CLK <= 42MHz
		//  APB2: APB2CLK <= 84MHz
		_RCC->CFGR = ( _RCC->CFGR & ~((0x3F<<10) | (0xF<<4)) ) |
					((clk->hpre & 0xF) << 4) |
					((clk->ppre1 & 0x7) << 10) |
					((clk->ppre2 & 0x7) << 13);
					
		// configure PLL
		_RCC->PLLCFGR = ( _RCC->PLLCFGR & ~((0xF<<24) | (3<<16) | (0x7FFF)) ) |
					((clk->pllm & 0x3F)) |
					((clk->plln & 0x1FF) << 6) |
					((((clk->pllp>>1) - 1) & 0x3) << 16) |
					((clk->pllq & 0x3F) << 24);
					
		// enable PLL oscillator
		rcc_osc_on(RCC_PLL);
		
		// set Flash timings and enable PREFETCH, Instruction and Data cahes
		_FLASH->ACR = (_FLASH->ACR & ~(0xF)) | 
					((1U<<8) | (1U<<9) | (1U<<10) | (clk->flash_cfg & 0xF));
		
		// connect to PLL
		rcc_set_sysclk(RCC_PLL);
		
		// stop unused clock
		if ((clk->pll_src == RCC_HSE) && (_RCC->CR & RCC_HSI_ON))
			rcc_osc_off(RCC_HSI);
		else
			rcc_osc_off(RCC_HSE);
			
	} else {							// Default: HSI Clock
		rcc_osc_on(RCC_HSI);
		rcc_set_sysclk(RCC_HSI);
		rcc_osc_off(RCC_PLL);
		rcc_osc_off(RCC_HSE);
	}
	
	sysclks.ahb_freq = clk->ahb_freq;
	sysclks.apb1_freq = clk->apb1_freq;
	sysclks.apb1_timer_freq = clk->ppre1==RCC_CFGR_PPRE_DIV_NONE ? clk->apb1_freq : 2*clk->apb1_freq;
	sysclks.apb2_freq = clk->apb2_freq;
	sysclks.apb2_timer_freq = clk->ppre2==RCC_CFGR_PPRE_DIV_NONE ? clk->apb2_freq : 2*clk->apb2_freq;
}

void rcc_i2s_clk_cfg(const I2SClkCfg_t *cfg)
{
	rcc_osc_off(RCC_PLLI2S);
	_RCC->PLLI2SCFGR = (cfg->R & 0x7)<<28 | (cfg->N & 0x1FF)<<6 | (cfg->M & 0x3F);
	rcc_osc_on(RCC_PLLI2S);
	sysclks.i2s_freq=(sysclks.main_osc_is_hse ? HSE_VALUE : HSI_VALUE) / cfg->M * cfg->N / cfg->R;
}
