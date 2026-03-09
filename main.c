#include "stm32h743xx.h"
#include <math.h>
#include <stdint.h>

/* ===== CONFIG ===== */
#define SAMPLE_COUNT   1024        // samples per analysis frame
#define SAMPLE_FREQ    10000.0f    // sampling frequency (Hz)
#define FUND_FREQ      50.0f       // grid frequency (Hz)

/* ADC scaling (adjust to your hardware) */
#define V_ADC_FS       0.1f        // ADC full-scale voltage (100 mV from divider)
#define V_RATIO        2300.0f     // divider scaling (230 V → 0.1 V)
#define SHUNT_R        0.01f       // shunt resistor (ohms)
#define ADC_FS_COUNTS  32768.0f    // 16-bit signed full scale

#define VOLT_SCALE (V_ADC_FS * V_RATIO / ADC_FS_COUNTS)
#define CURR_SCALE (V_ADC_FS / (SHUNT_R * ADC_FS_COUNTS))

/* ===== SAMPLE BUFFERS (filled by ADC/DFSDM ISR or DMA) ===== */
volatile int32_t v_raw[SAMPLE_COUNT];
volatile int32_t i_raw[SAMPLE_COUNT];

/* ===== RESULTS ===== */
volatile float Vrms = 0;
volatile float Irms = 0;
volatile float RealPower = 0;
volatile float ReactivePower = 0;
volatile float PhaseAngle = 0;
volatile float THD = 0;

/* ===== PROCESS FUNCTION ===== */
void ProcessPowerFrame(void)
{
    float v, i;
    float sum_v = 0;
    float sum_i = 0;
    float sum_p = 0;

    float real_sum = 0;
    float imag_sum = 0;

    for(int n = 0; n < SAMPLE_COUNT; n++)
    {
        /* convert ADC counts to real units */
        v = v_raw[n] * VOLT_SCALE;
        i = i_raw[n] * CURR_SCALE;

        sum_v += v * v;
        sum_i += i * i;
        sum_p += v * i;

        /* fundamental component extraction */
        float angle = 2.0f * M_PI * FUND_FREQ * n / SAMPLE_FREQ;

        real_sum += v * cosf(angle);
        imag_sum += v * sinf(angle);
    }

    /* RMS calculations */
    Vrms = sqrtf(sum_v / SAMPLE_COUNT);
    Irms = sqrtf(sum_i / SAMPLE_COUNT);

    /* real power */
    RealPower = sum_p / SAMPLE_COUNT;

    /* apparent power */
    float apparent = Vrms * Irms;

    if(apparent > 0.001f)
        ReactivePower = sqrtf(apparent*apparent - RealPower*RealPower);
    else
        ReactivePower = 0;

    /* phase angle */
    PhaseAngle = atan2f(ReactivePower, RealPower) * 180.0f / M_PI;

    /* THD calculation */
    float fundamental =
        (2.0f / SAMPLE_COUNT) *
        sqrtf(real_sum*real_sum + imag_sum*imag_sum);

    float total_rms = Vrms;

    if(fundamental > 0.001f)
    {
        float harmonic_rms =
            sqrtf(total_rms*total_rms - fundamental*fundamental);

        THD = (harmonic_rms / fundamental) * 100.0f;
    }
    else
    {
        THD = 0;
    }
}
