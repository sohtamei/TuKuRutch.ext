#if defined(DEVICE_M5CAMERA)
// M5camera modelB
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     22
  #define SIOC_GPIO_NUM     23

  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

  #define P_LED				14
  #define P_LED_NEG			1

#elif defined(DEVICE_M5TIMERCAM)
// M5TimerCam
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23

  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

  #define P_LED				2
  #define P_LED_NEG			0

  #define BAT_OUTPUT_HOLD_PIN	33	// 0-bat power disable
  #define BAT_ADC_PIN			38

#elif defined(DEVICE_ESP32_CAM)
// ESP32-CAM
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM     0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27

  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM       5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22

  #define P_LED				33
  #define P_LED_NEG			1

#elif defined(DEVICE_ESP32CAM)
// ESP32cam
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23

  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       17
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

  #define P_LED				16
  #define P_LED_NEG			0

#elif defined(DEVICE_UNITCAM)
// UnitCam
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23

  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

  #define P_LED				4
  #define P_LED_NEG			1

#elif defined(QCAI)
// QCAI
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM     32
  #define SIOD_GPIO_NUM     -1
  #define SIOC_GPIO_NUM     -1
  #define SCCB_I2C_PORT      0

  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM       5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22

  #define P_LED				-1
  #define P_LED_NEG			0

#elif defined(DEVICE_CAMERATCH32)
// CAMERATCH32
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM     32
  #define SIOD_GPIO_NUM     -1
  #define SIOC_GPIO_NUM     -1
  #define SCCB_I2C_PORT      0

  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM       5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22

  #define P_LED				15
  #define P_LED_NEG			0

#elif defined(CAMERA_MODEL_ESP32S3_EYE)
// ESP32S3 camera Freenove
  #define PWDN_GPIO_NUM -1
  #define RESET_GPIO_NUM -1
  #define XCLK_GPIO_NUM 15
  #define SIOD_GPIO_NUM 4
  #define SIOC_GPIO_NUM 5

  #define Y2_GPIO_NUM 11
  #define Y3_GPIO_NUM 9
  #define Y4_GPIO_NUM 8
  #define Y5_GPIO_NUM 10
  #define Y6_GPIO_NUM 12
  #define Y7_GPIO_NUM 18
  #define Y8_GPIO_NUM 17
  #define Y9_GPIO_NUM 16

  #define VSYNC_GPIO_NUM 6
  #define HREF_GPIO_NUM 7
  #define PCLK_GPIO_NUM 13

  #define P_LED				2
  #define P_LED_NEG			0

#elif defined(CAMERA_MODEL_SQUARE_CAM)
// SQUARE CAM
  #define PWDN_GPIO_NUM -1
  #define RESET_GPIO_NUM -1
  #define XCLK_GPIO_NUM 15
  #define SIOD_GPIO_NUM 4
  #define SIOC_GPIO_NUM 5

  #define Y2_GPIO_NUM 11
  #define Y3_GPIO_NUM 9
  #define Y4_GPIO_NUM 8
  #define Y5_GPIO_NUM 10
  #define Y6_GPIO_NUM 12
  #define Y7_GPIO_NUM 18
  #define Y8_GPIO_NUM 17
  #define Y9_GPIO_NUM 16

  #define VSYNC_GPIO_NUM 6
  #define HREF_GPIO_NUM 7
  #define PCLK_GPIO_NUM 13

  #define P_LED				-1
  #define P_LED_NEG			0

#elif defined(CAMERA_MODEL_XIAO32S3SENSE)
// xiao32S3sense
  #define PWDN_GPIO_NUM -1
  #define RESET_GPIO_NUM -1
  #define XCLK_GPIO_NUM 10
  #define SIOD_GPIO_NUM 40
  #define SIOC_GPIO_NUM 39

  #define Y2_GPIO_NUM 15
  #define Y3_GPIO_NUM 17
  #define Y4_GPIO_NUM 18
  #define Y5_GPIO_NUM 16
  #define Y6_GPIO_NUM 14
  #define Y7_GPIO_NUM 12
  #define Y8_GPIO_NUM 11
  #define Y9_GPIO_NUM 48

  #define VSYNC_GPIO_NUM 38
  #define HREF_GPIO_NUM  47
  #define PCLK_GPIO_NUM  13

  #define P_LED				21
  #define P_LED_NEG			1

#elif defined(CAMERA_MODEL_CORES3)
// M5Stack CoreS3
  #define PWDN_GPIO_NUM -1
  #define RESET_GPIO_NUM -1		// AW9523B_P1_0
  #define XCLK_GPIO_NUM -1
  #define SIOD_GPIO_NUM -1
  #define SIOC_GPIO_NUM -1
  #define SCCB_I2C_PORT 1

  #define Y2_GPIO_NUM 39
  #define Y3_GPIO_NUM 40
  #define Y4_GPIO_NUM 41
  #define Y5_GPIO_NUM 42
  #define Y6_GPIO_NUM 15
  #define Y7_GPIO_NUM 16
  #define Y8_GPIO_NUM 48
  #define Y9_GPIO_NUM 47

  #define VSYNC_GPIO_NUM 46
  #define HREF_GPIO_NUM 38
  #define PCLK_GPIO_NUM 45

  #define P_LED				-1
  #define P_LED_NEG			0

#else
  #error
#endif

