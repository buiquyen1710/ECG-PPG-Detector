/**
* @file       MAX30102.h
* @date       2023-09-25
* @version    v0.0.1
* @author     Nguyen Doan Khanh - SPARC Lab
*
*/

/*! @file MAX30102.h
 * @brief Sensor driver for MAX30102 sensor
 */

#ifndef MAX30102_H_
#define MAX30102_H_

/* Header includes */
#include <esp_log.h>
#include <time.h>
#include <stdbool.h>
#include <i2cdev.h>
#include <esp_err.h>
#include <stdint.h>
#include <stddef.h>

/********************************************************/
/*! @name       Common macros               */
/********************************************************/

#if !defined(UINT8_C) && !defined(INT8_C)
#define INT8_C(x)    S8_C(x)
#define UINT8_C(x)   U8_C(x)
#endif

#if !defined(UINT16_C) && !defined(INT16_C)
#define INT16_C(x)   S16_C(x)
#define UINT16_C(x)  U16_C(x)
#endif

#if !defined(INT32_C) && !defined(UINT32_C)
#define INT32_C(x)   S32_C(x)
#define UINT32_C(x)  U32_C(x)
#endif

#if !defined(INT64_C) && !defined(UINT64_C)
#define INT64_C(x)   S64_C(x)
#define UINT64_C(x)  U64_C(x)
#endif

/**@}*/
/**\name C standard macros */
#ifndef NULL
#ifdef __cplusplus
#define NULL         0
#else
#define NULL         ((void *) 0)
#endif
#endif

/********************************************************/
#ifndef TRUE
#define TRUE                                      UINT8_C(1)
#endif
#ifndef FALSE
#define FALSE                                     UINT8_C(0)
#endif

/**\I2C frequency */
#define I2C_FREQ_HZ 400000

/**
 * MAX30102_INTF_RET_TYPE is the read/write interface return type which can be overwritten by the build system.
 */
#ifndef MAX30102_INTF_RET_TYPE
#define MAX30102_INTF_RET_TYPE                      int8_t
#endif

/**
 * The last error code from read/write interface is stored in the device structure as intf_rslt.
 */
#ifndef MAX30102_INTF_RET_SUCCESS
#define MAX30102_INTF_RET_SUCCESS                   INT8_C(0)
#endif

/**\name MAX30102 chip identifier */
#define MAX30102_CHIP_ID                            UINT8_C(0x15)

/**\MAX30102 I2C address */
#define MAX30102_SENSOR_ADDR 0x57

/**\name Register Address */
#define MAX30102_INT_STATUS1_ADDR               UINT8_C(0x00)
#define MAX30102_INT_STATUS2_ADDR               UINT8_C(0x01)
#define MAX30102_INT_ENABLE1_ADDR               UINT8_C(0x02)
#define MAX30102_INT_ENABLE2_ADDR               UINT8_C(0x03)
#define MAX30102_FIFO_WR_PTR_ADDR               UINT8_C(0x04)
#define MAX30102_FIFO_OVF_CTR_ADDR              UINT8_C(0x05)
#define MAX30102_FIFO_RD_PTR_ADDR               UINT8_C(0x06)
#define MAX30102_FIFO_DATA_ADDR                 UINT8_C(0x07)
#define MAX30102_FIFO_CFG_ADDR                  UINT8_C(0x08)
#define MAX30102_MODE_CFG_ADDR                  UINT8_C(0x09)
#define MAX30102_SPO2_CFG_ADDR                  UINT8_C(0x0A)
/* 0x0B is reserved */
#define MAX30102_LED1_PA_ADDR                   UINT8_C(0x0C)
#define MAX30102_LED2_PA_ADDR                   UINT8_C(0x0D)
/* 0x0E and 0x0F are reserved */
#define MAX30102_SLOT_1_2_ADDR                  UINT8_C(0x11)
#define MAX30102_SLOT_3_4_ADDR                  UINT8_C(0x12)
/* 0x13 trough 0x1E are reserved */
#define MAX30102_TEMP_INT_ADDR                  UINT8_C(0x1F)
#define MAX30102_TEMP_FRAC_ADDR                 UINT8_C(0x20)
#define MAX30102_TEMP_CFG_ADDR                  UINT8_C(0x21)

#define MAX30102_REV_ID_ADDR                    UINT8_C(0xFE)
#define MAX30102_CHIP_ID_ADDR                   UINT8_C(0xFF)

/**\name Interrupt status flags */
/*! @todo Define int status flags */

/**\name Interrupt enable flags */
/*! @todo Define int enable flags */

/**\name FIFO configuration values */
/* Samples averaging */
#define MAX30102_INT_A_FULL_MASK        	    (byte)~0b10000000;
#define MAX30102_SMP_AVE_NO                     UINT8_C(0x00)
#define MAX30102_SMP_AVE_2                      UINT8_C(0x20)
#define MAX30102_SMP_AVE_4                      UINT8_C(0x40)
#define MAX30102_SMP_AVE_8                      UINT8_C(0x60)
#define MAX30102_SMP_AVE_16                     UINT8_C(0x80)
#define MAX30102_SMP_AVE_32                     UINT8_C(0xE0)

/* Roll over enable */
#define MAX30102_FIFO_ROLL_OVER                 UINT8_C(0x10)

/**\name MODE configuration */
#define MAX30102_SHDWN                          UINT8_C(0x80)
#define MAX30102_HR_MODE                        UINT8_C(0x02)
#define MAX30102_SPO2_MODE                      UINT8_C(0x03)
#define MAX30102_MULTILED_MODE                  UINT8_C(0x07)

/**\name SPO2 configuration */
#define MAX30102_SPO2_RANGE_2048                UINT8_C(0x00)
#define MAX30102_SPO2_RANGE_4096                UINT8_C(0x20)
#define MAX30102_SPO2_RANGE_8192                UINT8_C(0x40)
#define MAX30102_SPO2_RANGE_16384               UINT8_C(0x60)
#define MAX30102_SPO2_50_SPS                    UINT8_C(0x00)
#define MAX30102_SPO2_100_SPS                   UINT8_C(0x04)
#define MAX30102_SPO2_200_SPS                   UINT8_C(0x08)
#define MAX30102_SPO2_400_SPS                   UINT8_C(0x0C)
#define MAX30102_SPO2_800_SPS                   UINT8_C(0x10)
#define MAX30102_SPO2_1000_SPS                  UINT8_C(0x14)
#define MAX30102_SPO2_1600_SPS                  UINT8_C(0x18)
#define MAX30102_SPO2_3200_SPS                  UINT8_C(0x1C)
#define MAX30102_SPO2_LED_PW_69                 UINT8_C(0x00)
#define MAX30102_SPO2_LED_PW_118                UINT8_C(0x01)
#define MAX30102_SPO2_LED_PW_215                UINT8_C(0x02)
#define MAX30102_SPO2_LED_PW_411                UINT8_C(0x03)

// MAX30102 Commands
// Interrupt configuration (pg 13, 14)
#define MAX30102_INT_A_FULL_ENABLE  	0x80
#define MAX30102_INT_A_FULL_DISABLE  	0x00

#define MAX30102_INT_DATA_RDY_MASK      (byte)~0b01000000
#define MAX30102_INT_DATA_RDY_ENABLE 	0x40
#define MAX30102_INT_DATA_RDY_DISABLE   0x00

#define MAX30102_INT_ALC_OVF_MASK       (byte)~0b00100000
#define MAX30102_INT_ALC_OVF_ENABLE  	0x20
#define MAX30102_INT_ALC_OVF_DISABLE    0x00

#define MAX30102_INT_PROX_INT_MASK      0xEF
#define MAX30102_INT_PROX_INT_ENABLE    0x10
#define MAX30102_INT_PROX_INT_DISABLE   0x00

#define MAX30102_INT_DIE_TEMP_RDY_MASK      0xFD
#define MAX30102_INT_DIE_TEMP_RDY_ENABLE    0x02
#define MAX30102_INT_DIE_TEMP_RDY_DISABLE   0x00

#define MAX30102_SAMPLEAVG_MASK 	0x1F
#define MAX30102_SAMPLEAVG_1 	    0x00
#define MAX30102_SAMPLEAVG_2  	    0x20
#define MAX30102_SAMPLEAVG_4  	    0x40
#define MAX30102_SAMPLEAVG_8  	    0x60
#define MAX30102_SAMPLEAVG_16  	    0x80
#define MAX30102_SAMPLEAVG_32 	    0xA0

#define MAX30102_ROLLOVER_MASK  	0xEF
#define MAX30102_ROLLOVER_ENABLE    0x10
#define MAX30102_ROLLOVER_DISABLE   0x00

#define MAX30102_A_FULL_MASK  	    0xF0

// Mode configuration commands (page 19)
#define MAX30102_SHUTDOWN_MASK 	    0x7F
#define MAX30102_SHUTDOWN 		    0x80
#define MAX30102_WAKEUP  			0x00

#define MAX30102_RESET_MASK 		0xBF
#define MAX30102_RESET 			    0x40

#define MAX30102_MODE_MASK  		0xF8
#define MAX30102_MODE_REDONLY  	    0x02
#define MAX30102_MODE_REDIRONLY  	0x03
#define MAX30102_MODE_MULTILED  	0x07

// Particle sensing configuration commands (pgs 19-20)
#define MAX30102_ADCRANGE_MASK 	    0x9F
#define MAX30102_ADCRANGE_2048  	0x00
#define MAX30102_ADCRANGE_4096  	0x20
#define MAX30102_ADCRANGE_8192  	0x40
#define MAX30102_ADCRANGE_16384  	0x60

#define MAX30102_SAMPLERATE_MASK    0xE3
#define MAX30102_SAMPLERATE_50  	0x00
#define MAX30102_SAMPLERATE_100  	0x04
#define MAX30102_SAMPLERATE_200  	0x08
#define MAX30102_SAMPLERATE_400  	0x0C
#define MAX30102_SAMPLERATE_800  	0x10
#define MAX30102_SAMPLERATE_1000    0x14
#define MAX30102_SAMPLERATE_1600    0x18
#define MAX30102_SAMPLERATE_3200    0x1C

#define MAX30102_PULSEWIDTH_MASK    0xFC
#define MAX30102_PULSEWIDTH_69  	0x00
#define MAX30102_PULSEWIDTH_118  	0x01
#define MAX30102_PULSEWIDTH_215  	0x02
#define MAX30102_PULSEWIDTH_411  	0x03

//Multi-LED Mode configuration (pg 22)
#define MAX30102_SLOT1_MASK 		0xF8
#define MAX30102_SLOT2_MASK  		0x8F
#define MAX30102_SLOT3_MASK  		0xF8
#define MAX30102_SLOT4_MASK  		0x8F

#define SLOT_NONE 				0x00
#define SLOT_RED_LED 			0x01
#define SLOT_IR_LED 			0x02
#define SLOT_GREEN_LED 			0x03
#define SLOT_NONE_PILOT  		0x04
#define SLOT_RED_PILOT 			0x05
#define SLOT_IR_PILOT		    0x06
#define SLOT_GREEN_PILOT  		0x07

#define STORAGE_SIZE 4 //Each sample long is 4 bytes so limit this to fit on your micro

/**\name Multiled mode configuration */
/*! @todo Define configuration values for multiled mode. */

/**\name API success code */
#define MAX30102_OK                                 INT8_C(0)

/**\name API error codes */
#define MAX30102_E_NULL_PTR                         INT8_C(-1)
#define MAX30102_E_DEV_NOT_FOUND                    INT8_C(-2)
#define MAX30102_E_INVALID_LEN                      INT8_C(-3)
#define MAX30102_E_COMM_FAIL                        INT8_C(-4)
#define MAX30102_E_SLEEP_MODE_FAIL                  INT8_C(-5)
#define MAX30102_E_NVM_COPY_FAILED                  INT8_C(-6)

// /**\name Macros related to size */
#define MAX30102_DATA_LEN                           UINT8_C(6)

/**\name Macro to combine two 8 bit data's to form a 16 bit data */
#define BME280_CONCAT_BYTES(msb, lsb)             (((uint16_t)msb << 8) | (uint16_t)lsb)

#define BME280_SET_BITS(reg_data, bitname, data) \
    ((reg_data & ~(bitname##_MSK)) | \
     ((data << bitname##_POS) & bitname##_MSK))
#define BME280_SET_BITS_POS_0(reg_data, bitname, data) \
    ((reg_data & ~(bitname##_MSK)) | \
     (data & bitname##_MSK))

#define BME280_GET_BITS(reg_data, bitname)        ((reg_data & (bitname##_MSK)) >> \
                                                   (bitname##_POS))
#define BME280_GET_BITS_POS_0(reg_data, bitname)  (reg_data & (bitname##_MSK))

/**\name Sensor component selection macros
 * These values are internal for API implementation.
 */
#define MAX30102_BPM                              UINT8_C(1)
#define MAX30102_ALL                                UINT8_C(0x07)

// /**\name Settings selection macros */
#define MAX30102_BPM_SAMPLES_SIZE 50 /*!< The size of the buffer for the samples from sensor */
#define MAX30102_BPM_PERIOD_SAMPLE_SIZE 4 /*!< The size of the buffer for average period */
#define MAX30102_BPM_NO_SAMPLES MAX30102_BPM_SAMPLES_SIZE + 1 /*!< Dummy value for zero-crossing detection indexes */

/*!
 * @brief Interface selection Enums
 */
enum max30102_intf {
    /*< SPI interface */
    MAX30102_SPI_INTF,
    /*< I2C interface */
    MAX30102_I2C_INTF
};

/**
 * @brief Init I2C device description 
 * 
 * @param dev Device descriptor
 * @param port I2C port
 * @param sda_gpio SDA pin
 * @param scl_gpio SCLpin
 * @return Result of API execution status 
 */
esp_err_t max30102_initDesc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio);

/*!
 * @brief Type definitions
 */

/*!
 * @brief Bus communication function pointer which should be mapped to
 * the platform specific read functions of the user
 *
 * @param[in] reg_addr       : Register address from which data is read.
 * @param[out] reg_data     : Pointer to data buffer where read data is stored.
 * @param[in] len            : Number of bytes of data to be read.
 * @param[in, out] intf_ptr  : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs.
 *
 * @retval   0 -> Success.
 * @retval Non zero value -> Fail.
 *
 */
typedef MAX30102_INTF_RET_TYPE (*max30102_read_fptr_t)(uint8_t reg_addr, uint8_t *reg_data, uint32_t len);

/*!
 * @brief Bus communication function pointer which should be mapped to
 * the platform specific write functions of the user
 *
 * @param[in] reg_addr      : Register address to which the data is written.
 * @param[in] reg_data     : Pointer to data buffer in which data to be written
 *                            is stored.
 * @param[in] len           : Number of bytes of data to be written.
 * @param[in, out] intf_ptr : Void pointer that can enable the linking of descriptors
 *                            for interface related call backs
 *
 * @retval   0   -> Success.
 * @retval Non zero value -> Fail.
 *
 */
typedef MAX30102_INTF_RET_TYPE (*max30102_write_fptr_t)(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len);

/*!
 * @brief Delay function pointer which should be mapped to
 * delay function of the user
 *
 * @param[in] period              : Delay in microseconds.
 * @param[in, out] intf_ptr       : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs
 *
 */
typedef void (*max30102_delay_us_fptr_t)(uint32_t period);

struct max30102_data
{
    uint32_t red;
    uint32_t ir;
};

/*!
 * @brief MAX30102 sensor settings structure.
 */
struct max30102_settings
{
    uint8_t fifo;
    uint8_t spo2;
    uint8_t multiled;
};

/*!
 * @brief max30102 device structure
 */
struct max30102_record
{
    /*< Chip Id */
    uint8_t chip_id;

    /*< Delay function pointer */
    max30102_delay_us_fptr_t delay_us;

    /*< Sensor settings */
    struct max30102_settings settings;

    /*< Variable to store result of read/write function */
    MAX30102_INTF_RET_TYPE intf_rslt;

    /*< Head of sample */
    uint8_t head;

    /*< Tail of sample */
    uint8_t tail;

    /*< Array store red LED data read from data register */
    uint32_t red[STORAGE_SIZE]; 

    /*< Array store IR LED data read from data register */
    uint32_t IR[STORAGE_SIZE];

    /*< Number of active LEDs */
    uint8_t activeLEDs;
};

/**
 * \ingroup max30102
 * \defgroup max30102ApiInit Initialization
 * @brief Initialize the sensor and device structure
 */

/**
 * @brief Read Part ID of chip
 * 
 * @param dev  Device descriptor
 * @return Result of API execution status.
 */
esp_err_t max30102_readPartID(i2c_dev_t *dev);

/**
 * @brief Setup the sensor. The MAX30102 has many settings. By default we select:
 *                                                          Sample Average = 4,
 *                                                          Mode = MultiLED,
 *                                                          ADC Range = 16384 (62.5pA per LSB),
 *                                                          Sample rate = 50.
 * Use the default setup if you are just getting started with the MAX30102 sensor
 * 
 * @return Result of API execution status.
 */
esp_err_t max30102_init(uint8_t powerLevel, uint8_t sampleAverage, uint8_t ledMode, int sampleRate, int pulseWidth, int adcRange,struct max30102_record *record, i2c_dev_t *dev);

/**
 * @brief Reset all configuration, threshold, and data registers to POR values
 * 
 * @param record Structure instance of max30102_record.
 * @param dev  Device descriptor
 * @return Result of API execution status.
 */
esp_err_t max30102_softReset(i2c_dev_t *dev);

/*!
 * @details This API sets FIFO parameters
 * @param[in] dev : Structure instance of MAX30102.
 * @param[in] setup : Variable which contains setup for the FIFO.
 * 
 * @return Result of API execution status.
 */
esp_err_t max30102_setFIFO(uint8_t setup,struct max30102_record *record, i2c_dev_t *dev);

/*!
 * @details This API sets LED current amplitude
 * @param led led that be set
 * @param[in] dev : Structure instance of MAX30102.
 * @param[in] amplitude : Variable which contains discrete value mapped to current value (see datasheet).
 * 
 * @return Result of API execution status.
 */
esp_err_t max30102_setLedAmplitude(uint8_t led, uint8_t amplitude, i2c_dev_t *dev);

/*!
 * @details This API sets sensor mode
 * @param[in] dev : Structure instance of MAX30102.
 * @param[in] sensor_mode : Variable which contains setup for sensor adquisition mode.
 * 
 * @return Result of API execution status.
 */
esp_err_t max30102_setSpo2(uint8_t sensor_mode,struct max30102_record *record, i2c_dev_t *dev);

/*!
 * \ingroup max30102ApiSensorMode
 * \page max30102_api_max30102_set_sensor_mode max30102_set_sensor_mode
 * \code
 * int8_t max30102_set_sensor_mode(uint8_t sensor_mode, conststruct max30102_record *record, i2c_dev_t *dev);
 * \endcode
 * @details This API sets the adquisition mode of the sensor.
 *
 * @param[in] dev : Structure instance of max30102_record.
 * @param[in] sensor_mode : Variable which contains the mode to be set.
 *
 * - MAX30102_SHDWN: Shutdown sensor
 * - MAX30102_RESET: Reset sensor
 * - MAX30102_HR_MODE: Heart rate mode
 * - MAX30102_SPO2_MODE: SpO2 mode
 * - MAX30102_MULTILED_MODE: Heart rate and SpO2 mode.
 *
 * @return Result of API execution status
 *
 * @retval   0 -> Success.
 * @retval > 0 -> Warning.
 * @retval < 0 -> Fail.
 *
 */
esp_err_t max30102_setSensorMode(uint8_t sensor_mode,struct max30102_record *record, i2c_dev_t *dev);

/**
 * \ingroup max30102
 * \defgroup max30102ApiSensorData Sensor Data
 * @brief Data processing of sensor
 */
/*!
 * \ingroup max30102ApiSensorData
 * \page max30102_api_max30102_get_sensor_data max30102_get_sensor_data
 * \code
 * int8_t max30102_get_sensor_data(uint8_t sensor_comp, struct max30102_data *comp_data,struct max30102_record *record, i2c_dev_t *dev);
 * \endcode
 * @details This API reads the sample stored in the FIFO
 *
 * @param[in] led : Variable which selects which data to be read from
 * the sensor.
 *
 * MAX30102_BPM
 * MAX30102_ALL
 *
 * @param[out] comp_data : Structure instance of max30102_data.
 * @param[in] dev : Structure instance of max30102_record.
 *
 * @return Result of API execution status
 *
 * @retval   0 -> Success.
 * @retval > 0 -> Warning.
 * @retval < 0 -> Fail.
 *
 * @todo Implement read for SpO2 and multiled mode.
 */
esp_err_t max30102_getSensorData(uint8_t led, struct max30102_data *comp_data,struct max30102_record *record, i2c_dev_t *dev);

/*!
 * \ingroup max30102ApiSensorData
 * \page max30102_api_max30102_parse_sensor_data max30102_parse_sensor_data
 * \code
 * void max30102_parse_sensor_data(const uint8_t *reg_data, struct max30102_data *data);
 * \endcode
 *  @details This API is used to parse the sample read (see the datasheet for
 * more information about the samples format vs. ADC resolution)
 *
 *  @param[in] reg_data     : Contains register data which needs to be parsed
 *  @param[out] uncomp_data : Contains the uncompensated pressure, temperature
 *  and humidity data.
 *
 * @todo Implement parse for several ADC ranges.
 */
void max30102_parseSensorData(const uint8_t *reg_data, struct max30102_data *data);

/*!
 * \ingroup max30102ApiSensorData
 * \page max30102_api_max30102_get_bpm max30102_get_bpm
 * \code
 * void max30102_get_bpm(int32_t *data);
 * \endcode
 *  @details This API is used to process the buffer data in order to get the
 * BPM value through edge detection.
 *
 *  @param[in] data : The buffer with samples to process
 *
 */
esp_err_t max30102_get_bpm(int32_t *data);


/**
 * @brief set LED mode for max30102.
 * 
 * @param led_mode LED mode. It can be only RED or both RED and IR.
 * @param record Structure instance of max30102_record.
 * @param dev  Device descriptor
 * @return Result of API execution status
 *
 * @retval   0 -> Success.
 * @retval > 0 -> Warning.
 * @retval < 0 -> Fail.
 */
esp_err_t max30102_setLEDMode(uint8_t led_mode, i2c_dev_t *dev);

/**
 * @brief set ADC range for max30102
 * 
 * @param adcRange one of MAX30102_ADCRANGE_2048, _4096, _8192, _16384
 * @param record Structure instance of max30102_record.
 * @param dev  Device descriptor
 * @return Result of API execution statusa
 *
 * @retval   0 -> Success.
 * @retval > 0 -> Warning.
 * @retval < 0 -> Fail.
 */
esp_err_t max30102_setADCRange(uint8_t adcRange, i2c_dev_t *dev);

/**
 * @brief set the effective sampling rate with one sample consisting of one IR pulse/conversion and one Red pulse/conversion.
 * 
 * @param sampleRate one of MAX30102_ADCRANGE_2048, _4096, _8192, _16384
 * @param dev dev Structure instance of max30102_record.
 * @return Result of API execution status
 *
 * @retval   0 -> Success.
 * @retval > 0 -> Warning.
 * @retval < 0 -> Fail.
 */
esp_err_t max30102_setSampleRate(uint8_t sampleRate, i2c_dev_t *dev);

/**
 * @brief  set the LED pulse width (the IR and Red have the same pulse width), and therefore, indirectly sets the integration
 *          time of the ADC in each sample. The ADC resolution is directly related to the integration time.
 * 
 * @param pulseWidth one of MAX30102_PULSEWIDTH_69, _188, _215, _411
 * @param record Structure instance of max30102_record.
 * @param dev  Device descriptor
 * @return Result of API execution statusa
 *
 * @retval   0 -> Success.
 * @retval > 0 -> Warning.
 * @retval < 0 -> Fail.
 */
esp_err_t max30102_setPulseWidth(uint8_t pulseWidth, i2c_dev_t *dev);

/**
 * @brief In multi-LED mode, each sample is split into up to four time slots, SLOT1 through SLOT4. These control registers determine
 *        which LED is active in each time slot, making for a very flexible configuration.
 * 
 * @param slotNumber number of slots for LED
 * @param record Structure instance of max30102_record.
 * @param dev  Device descriptor
 * @return Result of API execution statusa
 *
 * @retval   0 -> Success.
 * @retval > 0 -> Warning.
 * @retval < 0 -> Fail.
 */
esp_err_t max30102_enableSlot(uint8_t slotNumber, uint8_t device, i2c_dev_t *dev);

/**
 * @brief Clears all slot assignments.
 * 
 * @param record Structure instance of max30102_record.
 * @param dev  Device descriptor
 * @return Result of API execution statusa
 *
 * @retval   0 -> Success.
 * @retval > 0 -> Warning.
 * @retval < 0 -> Fail.
 */
esp_err_t max30102_disableSlots(i2c_dev_t *dev);

/**
 * @brief Clear FIFO.
 * 
 * @param record Structure instance of max30102_record.
 * @param dev  Device descriptor
 * @return Result of API execution statusa
 *
 * @retval   0 -> Success.
 * @retval > 0 -> Warning.
 * @retval < 0 -> Fail.
 */
esp_err_t max30102_clearFIFO(i2c_dev_t *dev);

/**
 * @brief Enable roll over if FIFO over flows.
 * 
 * @param record Structure instance of max30102_record.
 * @param dev  Device descriptor
 * @return Result of API execution statusa
 *
 * @retval   0 -> Success.
 * @retval > 0 -> Warning.
 * @retval < 0 -> Fail.
 */
esp_err_t max30102_enableFIFORollover(i2c_dev_t *dev);

/**
 * @brief Disable roll over if FIFO over flows.
 * 
 * @param record Structure instance of max30102_record.
 * @param dev  Device descriptor
 * @return Result of API execution statusa
 *
 * @retval   0 -> Success.
 * @retval > 0 -> Warning.
 * @retval < 0 -> Fail.
 */
esp_err_t max30102_disableFIFORollover(i2c_dev_t *dev);

/**
 * @brief To reduce the amount of data throughput, adjacent samples (in each individual channel) can be averaged and decimated
 *        on the chip by setting this register.
 * 
 * @param numberOfSamples No. of sameples avaraged by FIFO sample
 * @param record Structure instance of max30102_record.
 * @param dev  Device descriptor
 * @return Result of API execution statusa
 */
esp_err_t max30102_setFIFOAvarage(uint8_t numberOfSamples, i2c_dev_t *dev);

/**
 * @brief Read the FIFO Write Pointer.
 * 
 * @param record Structure instance of max30102_record.
 * @param dev  Device descriptor
 * @return Result of API execution statusa
 *
 * @retval   0 -> Success.
 * @retval > 0 -> Warning.
 * @retval < 0 -> Fail.
 */
uint8_t max30102_getWritePointer(i2c_dev_t *dev);

/**
 * @brief Read the FIFO Read Pointer.
 * 
 * @param record Structure instance of max30102_record.
 * @param dev  Device descriptor
 * @return Result of API execution statusa
 *
 * @retval   0 -> Success.
 * @retval > 0 -> Warning.
 * @retval < 0 -> Fail.
 */
uint8_t max30102_getReadPointer(i2c_dev_t *dev);

/**
 * @brief Tell caller how many samples are available.
 * 
 * @param dev Structure instance of max30102_record.
 * @return Number of samples that are available.
 */
int max30102_available(struct max30102_record *record);

/**
 * @brief Advance the tail
 * 
 * @param record Structure instance of max30102_record.
 * @return esp_err_t Status of function, ESP_OK is succeed.
 */
esp_err_t max30102_nextSample(struct max30102_record *record);


/**
 * @brief Polls the sensor for new data. Call regularly. If new data is available, 
 *          it updates the head and tail in the main struct. Returns number of new samples obtained
 * 
 * @param dev Structure instance of max30102_record.
 * @return Returns number of new samples obtained
 */
int16_t max30102_check(struct max30102_record *record, i2c_dev_t *dev);

/**
 * @brief Check for new data but give up after a certain amount of time. 
 * 
 * @param maxTimeToCheck Certain amount of time to check
 * @param dev Structure instance of max30102_record.
 * @return Returns true if new data was found. Returns false if new data was not found
 */
esp_err_t max30102_safeCheck(uint8_t maxTimeToCheck, struct max30102_record *record, i2c_dev_t *dev);

/**
 * @brief Given a register, read it, mask it, and then set the thing.
 * 
 * @param reg Register to set.
 * @param mask Mask of bit that be set.
 * @param thing Data of mask bit that you want to set.
 * @param record Structure instance of max30102_record.
 * @param dev  Device descriptor
 * @return Result of API execution statusa
 */
esp_err_t max30102_bitmask(uint8_t reg, uint8_t mask, uint8_t thing, i2c_dev_t *dev);

#endif /* MAX30102_H_ */
/** @}*/

/**
 * @brief Get the next Red value in the FIFO
 * 
 * @param record Structure instance of max30102_record.
 * @param dev Device descriptor
 * @return uint32_t The next Red value in the FIFO
 */
uint32_t max30102_getFIFORed(struct max30102_record *record);

/**
 * @brief Get the next IR value in the FIFO
 * 
 * @param record Structure instance of max30102_record.
 * @param dev Device descriptor
 * @return uint32_t The next IR value in the FIFO
 */
uint32_t max30102_getFIFOIR(struct max30102_record *record);
