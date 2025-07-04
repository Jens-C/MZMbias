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
//amplitude of the dither voltage
#define DITHER_VOLTAGE 0.025
// sampling frequency must be multiple of the FFT_BUFFER_SIZE
#define FFT_BUFFER_SIZE 2048
// sampling frequency must be multiple of the FFT_BUFFER_SIZE
#define SAMPLE_FREQ 10240
// amount of times the fft is averaged before correcting the MZM dc bias
#define FFT_AVRAGE_COUNT 100
//Set 0 to disable polarization control
#define EnablePolControl 0
// the size of the steps the initial MZM bias sweep takes in dac values 0 = 0V, 4096 = 12V
#define STEP_SIZE_BIAS_SWEEP 16
// the size of the MZM bias correction when the algorithm runs 0 = 0V, 4096 = 12V
#define BIAS_CORRECTION_FACTOR 2
//the ratio of the dither signal being on/off, minium of 4
#define DISABLE_BIAS_COUNT 4

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
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */


// calculate the sinewave of the dithersignal
uint32_t sine_val[100];
void calcsin ()
{
	for (int i=0; i<100; i++)
	{
    sine_val[i] = ((sin(i * 2 * PI / 100) + 1) / 2) * (DITHER_VOLTAGE * 4096 / 3.3) + 200;
	}
}

//reverse of the calcsin function, used in testing to get the phase angle by reading the state of the DAC
float getPhaseAngle(int val) {
    float SCALE = (DITHER_VOLTAGE * 4096.0f / 3.3f);
    // Reverse transform
    float sine_val = (2.0f * (val - 200) / SCALE) - 1.0f;
    // Clamp to valid range [-1, 1]
    if (sine_val < -1.0f || sine_val > 1.0f) {
        return NAN;
    }
    // Return phase angle in radians
    return asinf(sine_val);
}

// hanning window array creation
uint16_t hanning_array[FFT_BUFFER_SIZE];
void calchanning(){
	for (int i = 0; i < FFT_BUFFER_SIZE; i++) {
	        hanning_array[i] = (int)(0.5 * (1 - cos(2 * M_PI * i / (FFT_BUFFER_SIZE - 1))) * 4096);
	    }
}

//function to get sign of a value
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

//FFT buffers for the double half buffer structure
arm_rfft_fast_instance_f32 fftHandler;
uint16_t ADC_val[FFT_BUFFER_SIZE*2] = {0};
float fftBufIn1[FFT_BUFFER_SIZE]= {0.0};
float fftBufIn2[FFT_BUFFER_SIZE]= {0.0};
float fftBufOut[FFT_BUFFER_SIZE];
//index of the current fft count
static int16_t fftIndex = 0;
//data buffer for serial debug
uint8_t data[50] = {0};
uint16_t fft_count = 0;
//avg used for noise
uint32_t avg =0;
//calculate which sample is 1khz
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
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void Sweep_PWM(TIM_HandleTypeDef *htim, uint32_t channel);
uint32_t Read_ADC(void);
//define pointers of the 3 timers used for pwm
TIM_HandleTypeDef *htim_array[3] = {&htim3, &htim2, &htim2};
//define pointers of the 3 channels used for pwm
uint32_t pwm_channels[3] = {TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3};
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//read 1 ADC value
uint32_t Read_ADC(void) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(&hadc1);
}

//sweep pwm values of the MPC
void Sweep_PWM(TIM_HandleTypeDef *htim, uint32_t channel) {
  //delay to wait for mpc to go to start
	HAL_Delay(1000);
	uint32_t best_pwm_value = 0;
	uint32_t min_adc_value = 0xFFFFFFFF;
	uint32_t adc_val = Read_ADC();
    //loop over all posible duty cycles
    for (uint16_t duty = 900; duty <= 2100; duty += 10) {
    	__HAL_TIM_SET_COMPARE(htim, channel, duty);
      HAL_Delay(5);
      //average 5 adc values
    	uint32_t adc_val = 0;
      for(uint16_t i= 0; i<5;i++){
        adc_val += Read_ADC();
        HAL_Delay(2);
      }
      //sprintf(data, "adc:%d, pwm:%d \r\n",adc_val, duty);
      //HAL_UART_Transmit(&huart2, data, strlen(data), 100);
      //save best value
      if (adc_val < min_adc_value) {
        	min_adc_value = adc_val;
        	best_pwm_value = duty;
      }

    }
    //sprintf(data, "best adc:%d, pwm:%d \r\n",min_adc_value, best_pwm_value );
    //HAL_UART_Transmit(&huart2, data, strlen(data), 100);
    //set pwm of paddle to best
    __HAL_TIM_SET_COMPARE(htim, channel, best_pwm_value);
    HAL_Delay(1000);
}

//First half buffer interupt handler
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
        for (int i = 0; i < FFT_BUFFER_SIZE ; i++) {
	 	fftBufIn1[i] = (float)(ADC_val[i]*hanning_array[i]);
        }
        //sprintf(data, "half buffer\r\n\n ");
        //HAL_UART_Transmit(&huart2, data, strlen(data), 100);
        Calc_FFT(fftBufIn1);
}

//Second half buffer interupt handler
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	 for (int i = 0; i < FFT_BUFFER_SIZE; i++) {
	  	fftBufIn2[i] = (float)(ADC_val[i]*hanning_array[i]);
        }
        //sprintf(data, "full buffer\r\n\n " );
        //HAL_UART_Transmit(&huart2, data, strlen(data), 100);
        Calc_FFT(fftBufIn2);
}

//function to calculate the fft of a buffer
void Calc_FFT(float *fftBufIn ){

	    arm_rfft_fast_f32(&fftHandler, fftBufIn, &fftBufOut,0);
		  float phase_1khz = 0.0f, phase_2khz = 0.0f;
		  //calc freqmagnitude for 1,2,3khz
		  float phase_shift_diff = 0.0;
		  float dacval = 0.0f;
		  for (int i = sample_count_1khz; i <= sample_count_1khz*3; i +=sample_count_1khz){
		        float real = fftBufOut[2*i ];
		        float imag = fftBufOut[(2*i) + 1];
		        float curVal = sqrtf((real * real) + (imag * imag));
		        float phase = atan2f(imag, real);
		        if (i == sample_count_1khz) {
		            phase_1khz = phase;
		        } else if (i == sample_count_1khz * 2) {
		            phase_2khz = phase;
                //because the sinewave generation and the fft speed are synchronized instead of the phase diff between generated sinewave and 2nd harmonic the phase of only 2nd harmonic is used
		            phase_shift_diff = phase_2khz;
		            //dacval= getPhaseAngle(HAL_DAC_GetValue(&hdac1, DAC1_CHANNEL_2));
		        }
			  freq_mag[i/sample_count_1khz-1]+=((uint32_t)curVal)/1000;
	  		  //sprintf(data, "%d \r\n", (uint16_t)(curVal));
	  		  //HAL_UART_Transmit(&huart2, data, strlen(data), 100);

		  }
		  //sprintf(data, "%d ", (int16_t)(phase_2khz*1000));
		  //HAL_UART_Transmit(&huart2, data, strlen(data), 100);


			  avgPhaseShift += phase_shift_diff;
			  fft_count++;
        //calculate average of the intire FFT
			  uint32_t avgTemp = 0;
			  for (int i = 4; i < FFT_BUFFER_SIZE; i +=2){
			  	float curVal = sqrtf((fftBufOut[i]*fftBufOut[i])+ (fftBufOut[i+1]*fftBufOut[i+1]));
			  	avgTemp += ((uint32_t)curVal)/1000;  // Convert float to
			  }
			  avg += ((uint32_t)avgTemp/((FFT_BUFFER_SIZE / 2)-2));

		  //code usefull for debugging, peaHz detector
/*
		  uint16_t freqIndex = 0;
		  float peakVal =0.0;
		  uint16_t peakHz = 0;
		  for (int i = 0; i < FFT_BUFFER_SIZE; i +=2){
			 float curVal = sqrtf((fftBufOut[i]*fftBufOut[i])+ (fftBufOut[i+1]*fftBufOut[i+1]));
			 uint16_t fft_value = (uint32_t)curVal;  // Convert float to

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
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  //polarization control
  if(EnablePolControl){
	  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
	  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	  for (uint8_t i = 0; i < 3; i++) {
	  		__HAL_TIM_SET_COMPARE(htim_array[i],pwm_channels[i],900);
	  		uint32_t adc_val = Read_ADC();
	  	  }
	  	HAL_Delay(1000);
	  	// intial sweep
	  	for (uint8_t i = 0; i < 3; i++) {
	  	     Sweep_PWM(htim_array[i],pwm_channels[i]);
	  	  }
	  	// sweep 3 extra times padel 2 and 3 for extra accuracy
	  	for (uint8_t i = 0; i < 2; i++) {
	  		__HAL_TIM_SET_COMPARE(htim_array[1],pwm_channels[1],900);
	  		HAL_Delay(500);
	  		Sweep_PWM(htim_array[1],pwm_channels[1]);
	  		__HAL_TIM_SET_COMPARE(htim_array[2],pwm_channels[2],900);
	  		HAL_Delay(500);
	  		Sweep_PWM(htim_array[2],pwm_channels[2]);
	  	}
  }

  //start sweep for mzm bias and calculate bias on midpoint between min and max output current
  uint32_t adc_val[256]={0};
  uint16_t dac_val[256]={0};
  uint16_t index_adc_val_smallest=0;
  uint16_t index_adc_val_highest=0;
  HAL_DAC_Start(&hdac1,DAC_CHANNEL_1);
  Read_ADC();
  for(int i = 0; i< 3000; i+=STEP_SIZE_BIAS_SWEEP){
	  //take avrage of 20 values
	  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1,DAC_ALIGN_12B_R, i);
	  for(int j = 0; j<20;j++){
		  HAL_Delay(2);
		  adc_val[i/STEP_SIZE_BIAS_SWEEP] += Read_ADC();
	  }
	  //sprintf(data,"set to:%d, adc: %d\r\n\n ",i, adc_val[i/STEP_SIZE_BIAS_SWEEP]);
	  //HAL_UART_Transmit(&huart2, data, strlen(data), 100);
	  //keep highest and lowest adc values
	  if(adc_val[i/STEP_SIZE_BIAS_SWEEP]>=adc_val[index_adc_val_highest]){
		  index_adc_val_highest = i/STEP_SIZE_BIAS_SWEEP;
	  }else if(adc_val[i/STEP_SIZE_BIAS_SWEEP]<=adc_val[index_adc_val_smallest]){
		  index_adc_val_smallest = i/STEP_SIZE_BIAS_SWEEP;
	  }
	  dac_val[i/STEP_SIZE_BIAS_SWEEP] = i;
	  HAL_Delay(10);
  }
   //set dac to quad bias point in middle of max/min
  uint16_t midpoint_dac_val = dac_val[(index_adc_val_highest + index_adc_val_smallest) / 2];
 //DC bias can be set to static value if sweep is unwanted
  //midpoint_dac_val = 1870;
  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, midpoint_dac_val  );
  sprintf(data, "bias set as:%d\r\n\n ",midpoint_dac_val);
  //HAL_UART_Transmit(&huart2, data, strlen(data), 100);

  //reset the adc
  //initialize adc for running in continous mode, this is normaly auto generated but because 2 different modes are used extra adc_init is added
  MX_ADC1_Init2();
  HAL_TIM_Base_Start(&htim6);
  calcsin();
  calchanning();
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  //start dither generation
  HAL_DAC_Start_DMA(&hdac1, DAC1_CHANNEL_2, sine_val, 100, DAC_ALIGN_12B_R);
  //start double buffer 
  HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_val,FFT_BUFFER_SIZE*2);
    //init fft
  arm_rfft_fast_init_f32(&fftHandler,FFT_BUFFER_SIZE);
  HAL_TIM_Base_Start(&htim1);



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint16_t dac_val_tone_bias[3]={0};
  uint32_t adc_val_tone_bias[3]={0};
  int16_t sweep_count = -1;
  uint16_t prev_bias = 0;
  int16_t bias_count = 0;
  while (1)
  {

	  if(fft_count>=FFT_AVRAGE_COUNT){
      //loop to pause the dither signal, delay is used because this way the sync between dithe and FFT is kept
		  if(bias_count ==0){
			  HAL_TIM_Base_Stop(&htim6);
			  HAL_TIM_Base_Stop(&htim1);
			  HAL_Delay(1000);
			  HAL_TIM_Base_Start(&htim6);
			  HAL_TIM_Base_Start(&htim1);
		  }
		  else if(DISABLE_BIAS_COUNT -2 ==bias_count){

		  }else if(DISABLE_BIAS_COUNT ==bias_count){
			  float phaseShift = avgPhaseShift/ FFT_AVRAGE_COUNT;
			  fft_count=0;
        //avg can be used to look at the noise level of the TIA
        //sprintf(data, "avg: %d\r\n ",avg);
			  //HAL_UART_Transmit(&huart2, data, strlen(data), 100);
			  avg = 0;
        //correct bias according to the phase shift
				float correction2 =-BIAS_CORRECTION_FACTOR * sgn(phaseShift);
				midpoint_dac_val = midpoint_dac_val - (int16_t)(correction2);

        // print value of the phase shift and new DAC value 0 ~= 0V, 4096 ~12V
			  sprintf(data, "phase: %d new dac val: %d\r\n\n ",(int16_t)(phaseShift*1000), midpoint_dac_val);
			  HAL_UART_Transmit(&huart2, data, strlen(data), 100);
			  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, midpoint_dac_val);
			  bias_count = -1;
		  }
       //reset fft
		  bias_count++;
	  		fft_count=0;
	  		avgPhaseShift = 0;
			  for(int i =0;i<=2;i++){
				  freq_mag[i]=0;
			  }
			  fft_count=0;
	  }

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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 63;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 63;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 4999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

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
//this is added because both a DMA config and a normal config are needed for this adc
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
