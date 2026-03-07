#include "stm32h7xx_hal.h"
#include <math.h>

/* ===== VARIABLES TO WATCH ===== */
volatile uint32_t alive_counter = 0;
volatile float Vrms = 0.0f;
volatile float Irms = 0.0f;
volatile float RealPower = 0.0f;
volatile float ReactivePower = 0.0f;
volatile float PhaseAngle = 0.0f;

/* ===== PROTOTYPES ===== */
void SystemClock_Config(void);
/* ===== FAKE POWER CALCULATION ===== */
void Fake_Process(void) {
	static float theta = 0.0f;
	Vrms = 230.0f;
	Irms = 1.5f;
	PhaseAngle = theta * 180.0f / 3.14159f;
	RealPower = Vrms * Irms * cosf(theta);
	ReactivePower = Vrms * Irms * sinf(theta);
	theta += 0.1f;
	if (theta > 2 * 3.14159f)
		theta = 0.0f;
}

/* ===== MAIN ===== */
int main(void) {
	HAL_Init();
	SystemClock_Config();

	while (1) {
		Fake_Process();
		alive_counter++;      // <- THIS PROVES MCU IS RUNNING
		HAL_Delay(500);
	}
}

/* ===== CLOCK (SAFE DEFAULT) ===== */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.PLL.PLLState = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&osc);

    clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_0);
}
