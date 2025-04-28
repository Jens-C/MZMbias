/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define ARM_MATH_CM4
#include "arm_math.h"
#include "math.h"
#define PI 3.1415926
#define TOLERANCE 1e-5
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac_ch2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
#define FFT_BUFFER_SIZE 2048
#define SAMPLE_FREQ 10240
#define FFT_AVRAGE_COUNT 100

uint32_t sine_val[100];
uint16_t hanning_array[FFT_BUFFER_SIZE];

void calcsin ()
{
	for (int i=0; i<100; i++)
	{
		sine_val[i] = ((sin(i*2*PI/100) + 1)*(4096/2))/5+200;
	}
}

void calchanning(){
	for (int i = 0; i < FFT_BUFFER_SIZE; i++) {
	        hanning_array[i] = (int)(0.5 * (1 - cos(2 * M_PI * i / (FFT_BUFFER_SIZE - 1))) * 4096);
	    }
}
//only j1 and j2
float bessel_jn(uint8_t n, float x) {
    float sum = 0.0;
    float term;
    uint16_t k = 0;

    do {
        term = pow(-1, k) * pow(x / 2.0, 2 * k + n) / (tgamma(k + 1) * tgamma(k + n + 1));
        sum += term;
        k++;
    } while (fabs(term) > TOLERANCE);

    return sum;
}

int sgn(float x) {
    if(x>=0){
    	return 1;
    }
    else{
    	return -1;
    }
}
float arccot(float x) {
    return (PI / 2.0) - atan(x);
}


arm_rfft_fast_instance_f32 fftHandler;
uint16_t ADC_val[FFT_BUFFER_SIZE*2] = {0};
float fftBufIn[FFT_BUFFER_SIZE]= {0.0};
float fftBufOut[FFT_BUFFER_SIZE];
uint8_t fftflag = 0;
static int16_t fftIndex = 0;
uint8_t data[50] = {0};
uint16_t fft_count = 0;
//avg used for noise
uint32_t avg =0;
//calculate # sample is 1khz

uint16_t sample_count_1khz = (1000*FFT_BUFFER_SIZE)/SAMPLE_FREQ;
uint32_t freq_mag[3]={0};
float avgPhaseShift = 0.0;



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_DAC1_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t Read_ADC(void) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(&hadc1);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
        for (int i = 0; i < FFT_BUFFER_SIZE ; i++) {
	 	fftBufIn[i] = (float)(ADC_val[i]*hanning_array[i]) ;
        }
        //sprintf(data, "half buffer\r\n\n ");
        //HAL_UART_Transmit(&huart2, data, strlen(data), 100);
        Calc_FFT();
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	 for (int i = FFT_BUFFER_SIZE; i < FFT_BUFFER_SIZE*2; i++) {
	  	fftBufIn[i-FFT_BUFFER_SIZE] = (float)(ADC_val[i]) ;
        }
        //sprintf(data, "full buffer\r\n\n " );
        //HAL_UART_Transmit(&huart2, data, strlen(data), 100);
        Calc_FFT();
}





void Calc_FFT(){

	arm_rfft_fast_f32(&fftHandler, &fftBufIn, &fftBufOut,0);
		  float phase_1khz = 0.0f, phase_2khz = 0.0f;
		  //calc freqmagnitude for 1,2,3khz
		  float phase_shift_diff = 0.0;
		  for (int i = sample_count_1khz; i <= sample_count_1khz*3; i +=sample_count_1khz){
		        float real = fftBufOut[i * 2];
		        float imag = fftBufOut[(i * 2) + 1];
		        float curVal = sqrtf((real * real) + (imag * imag));
		        float phase = atan2f(imag, real);
		        if (i == sample_count_1khz) {
		            phase_1khz = phase;
		        } else if (i == sample_count_1khz * 2) {
		            phase_2khz = phase;
		            phase_shift_diff = phase_2khz - phase_1khz;
		        }

			  freq_mag[i/sample_count_1khz-1]+=(uint16_t)curVal;

		  }
		  sprintf(data, "%d ", (int16_t)(phase_shift_diff*1000));
		  HAL_UART_Transmit(&huart2, data, strlen(data), 100);
		  if (phase_shift_diff < 0) {
		      phase_shift_diff += 2 * PI;
		  } else if (phase_shift_diff >= 2 * PI) {
		      phase_shift_diff -= 2 * PI;
		  }


			  avgPhaseShift += phase_shift_diff;
			  fft_count++;


			  float avgTemp = 0.0;

			  for (int i = 4; i < FFT_BUFFER_SIZE; i +=2){
			  	float curVal = sqrtf((fftBufOut[i]*fftBufOut[i])+ (fftBufOut[i+1]*fftBufOut[i+1]));
			  	avgTemp += curVal;  // Convert float to
				  //sprintf(data, "%d ", (uint16_t)(curVal));
				  //HAL_UART_Transmit(&huart2, data, strlen(data), 100);
			  }
			  avg += (uint32_t)avgTemp/((FFT_BUFFER_SIZE / 2)-2);

		  //old code, peakHz detector
/*
		  uint16_t freqIndex = 0;
		  float peakVal =0.0;
		  uint16_t peakHz = 0;
		  for (int i = 0; i < FFT_BUFFER_SIZE; i +=2){
			 float curVal = sqrtf((fftBufOut[i]*fftBufOut[i])+ (fftBufOut[i+1]*fftBufOut[i+1]));
			 uint16_t fft_value = (uint16_t)curVal;  // Convert float to

			 if(curVal> peakVal){
				 if(i>2){
					peakVal = curVal;
				 }
				 peakHz = (uint16_t)(((freqIndex)*SAMPLE_FREQ)/FFT_BUFFER_SIZE);
			 }
			freqIndex++;


		 }


		  sprintf(data, "peakhz: %d ",peakHz);
		  			 HAL_UART_Transmit(&huart2, data, strlen(data), 100);

	  		freqIndex = 0;

	  		 */





}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_DAC1_Init();
  MX_TIM6_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  //start sweep for mzm bias and calculate bias on midpoint between min and max output current
#define STEP_SIZE_BIAS_SWEEP 16
  uint32_t adc_val[128]={0};
  uint16_t dac_val[128]={0};
  uint16_t index_adc_val_smallest=0;
  uint16_t index_adc_val_highest=0;
  HAL_DAC_Start(&hdac1,DAC_CHANNEL_1);
  Read_ADC();
  for(int i = 0; i< 2048; i+=STEP_SIZE_BIAS_SWEEP){
	  //take avrage of 10values
	  sprintf(data, "set to:%d\r\n\n ",i);
	  HAL_UART_Transmit(&huart2, data, strlen(data), 100);
	  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1,DAC_ALIGN_12B_R, i);
	  for(int j = 0; j<20;j++){
		  HAL_Delay(2);
		  adc_val[i/STEP_SIZE_BIAS_SWEEP] += Read_ADC();
	  }
	  //keep highest and lowest adc values
	  if(adc_val[i/STEP_SIZE_BIAS_SWEEP]>=adc_val[index_adc_val_highest]){
		  index_adc_val_highest = i/STEP_SIZE_BIAS_SWEEP;
	  }else if(adc_val[i/STEP_SIZE_BIAS_SWEEP]<=adc_val[index_adc_val_smallest]){
		  index_adc_val_smallest = i/STEP_SIZE_BIAS_SWEEP;
	  }
	  dac_val[i/STEP_SIZE_BIAS_SWEEP] = i;
	  HAL_Delay(10);
  }
  //set dac to midpoint
  uint16_t midpoint_dac_val = dac_val[(index_adc_val_highest + index_adc_val_smallest) / 2];
  midpoint_dac_val = 1500;
  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, midpoint_dac_val  );
  sprintf(data, "bias set as:%d\r\n\n ",midpoint_dac_val);
  HAL_UART_Transmit(&huart2, data, strlen(data), 100);


  //reset the adc
  HAL_ADC_Stop(&hadc1);
  //ADC_Disable(&hadc1);
  MX_ADC1_Init2();

  HAL_TIM_Base_Start(&htim6);
  calcsin();
  calchanning();
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_DAC_Start_DMA(&hdac1, DAC1_CHANNEL_2, sine_val, 100, DAC_ALIGN_12B_R);
  HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_val,FFT_BUFFER_SIZE*2);
  arm_rfft_fast_init_f32(&fftHandler,FFT_BUFFER_SIZE);
  HAL_TIM_Base_Start(&htim1);



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint16_t dac_val_tone_bias[3]={0};
  uint32_t adc_val_tone_bias[3]={0};
  int16_t sweep_count = -1;
  uint16_t prev_bias = 0;
  while (1)
  {

	  //for(int sweep = midpoint_dac_val-STEP_SIZE_BIAS_SWEEP; i<midpoint_dac_val+STEP_SIZE_BIAS_SWEEP; i+=STEP_SIZE_BIAS_SWEEP)
	  //sweep through best known value of bias, one below and one above
	  if(fft_count>=FFT_AVRAGE_COUNT){
		  float phaseShift = avgPhaseShift/ fft_count;
		  sprintf(data, "phase shift: %d\r\n\n ",(int16_t)(phaseShift*1000));
		  HAL_UART_Transmit(&huart2, data, strlen(data), 100);

		  fft_count=0;
		  sprintf(data, "%dfor  %d 1khz:%d 2khz:%d 3khz:%d\r\n\n ",sweep_count,prev_bias,freq_mag[0], freq_mag[1] ,freq_mag[2]);
		  HAL_UART_Transmit(&huart2, data, strlen(data), 100);
		    if (phaseShift> PI) {
		    	phaseShift -= 2 * PI;
		    } else if (phaseShift < PI) {
		    	//phaseShift+= 2 * PI;
		    }

			  sprintf(data, "phase shift: %d\r\n\n ",(int16_t)(phaseShift*1000));
			  HAL_UART_Transmit(&huart2, data, strlen(data), 100);
		  // divided by 4096# points dac multiplied by non inverting amp 14.124 = 3.3V*82k/25k+1

			  //code from paper
		  //float Vpi = ((midpoint_dac_val*13.724)/4096.0)*2;
		  //float Vac = 0.66;
		  //float correction =(Vpi/PI*arccot(((float)freq_mag[1]*bessel_jn(1,((Vac*PI)/Vpi)))/((float)freq_mag[0]*bessel_jn(2,((Vac*PI)/Vpi))))*sgn(phaseShift));//-(Vpi/2);
		  //midpoint_dac_val = midpoint_dac_val - (int16_t)((correction/13.724)*4096);
		  //fixed offset
			  sprintf(data, "avg: %d\r\n\n ",avg);
			  HAL_UART_Transmit(&huart2, data, strlen(data), 100);
		  if(freq_mag[1]> 150000){
			  float correction2 =50*sgn(phaseShift);
			  midpoint_dac_val = midpoint_dac_val - (int16_t)(correction2);
			  sprintf(data, "correction: %d\r\n\n ",(int16_t)(correction2));
			  HAL_UART_Transmit(&huart2, data, strlen(data), 100);
		  }
		  avg = 0;
		  sprintf(data, "new dac val: %d\r\n\n ",midpoint_dac_val);
		  HAL_UART_Transmit(&huart2, data, strlen(data), 100);
		  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, midpoint_dac_val);

	  		fft_count=0;
	  		avgPhaseShift = 0;
			  for(int i =0;i<=2;i++){
				  freq_mag[i]=0;
			  }
			  fft_count=0;
	  }

/*
	  //for(int sweep = midpoint_dac_val-STEP_SIZE_BIAS_SWEEP; i<midpoint_dac_val+STEP_SIZE_BIAS_SWEEP; i+=STEP_SIZE_BIAS_SWEEP)
	  //sweep through best known value of bias, one below and one above

	  if(fft_count>=FFT_AVRAGE_COUNT){
		  sprintf(data, "main");
		  HAL_UART_Transmit(&huart2, data, strlen(data), 100);
		  float phaseShift = avgPhaseShift/= fft_count;

		  fft_count=0;
		    if (phaseShift> PI) {
		    	phaseShift -= 2 * PI;
		    } else if (phaseShift < -PI) {
		    	phaseShift+= 2 * PI;
		    }
		  // divided by 4096# points dac multiplied by non inverting amp 14.124 = 3.3V*82k/25k+1
		  float Vpi = ((midpoint_dac_val*14.124)/4096.0)*2;
		  float Vac = 0.66;
		  float correction =(Vpi/PI*arccot((freq_mag[0]*bessel_jn(1,((Vac*PI)/Vpi)))/(freq_mag[1]*bessel_jn(2,((Vac*PI)/Vpi))))*sgn(avgPhaseShift))-(Vpi/2);
		  sprintf(data, "correction: %d\r\n\n ",(uint16_t)correction);
		  HAL_UART_Transmit(&huart2, data, strlen(data), 100);

		  midpoint_dac_val = midpoint_dac_val + (uint16_t)((correction/14.124)*4096);
		  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, midpoint_dac_val);

		  //process data here

	  		fft_count=0;
	  		avgPhaseShift = 0;
			  for(int i =0;i<=2;i++){
				  freq_mag[i]=0;
			  }
			  fft_count=0;
	  }
	  */

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 32;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT2 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 624;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 9;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 63;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 9;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void MX_ADC1_Init2(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T1_TRGO;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
