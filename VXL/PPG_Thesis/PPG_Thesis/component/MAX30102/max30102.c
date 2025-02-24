/*! @file max30102.c
 * @brief API for MAX30102 Heart rate and SpO2 sensor
 * @author Nguyen Doan Khanh - SPARC Lab
 */
#include <stdio.h>
#include <string.h>
#include <esp_err.h>
#include <esp_timer.h>
#include "max30102.h"

#define CHECK_ARG(ARG) do { if (!(ARG)) return ESP_ERR_INVALID_ARG; } while (0)

esp_err_t max30102_readPartID(i2c_dev_t *dev) {
    uint8_t data;

    I2C_DEV_TAKE_MUTEX(dev);
    I2C_DEV_CHECK(dev, i2c_dev_read_reg(dev, MAX30102_CHIP_ID_ADDR, &data, sizeof(data)));
    I2C_DEV_GIVE_MUTEX(dev);

    if (data == MAX30102_CHIP_ID) {
        return ESP_OK;
    }
    else {
        return ESP_ERR_NOT_FOUND;
    }
}

esp_err_t max30102_initDesc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
    CHECK_ARG(dev);

    dev->port = port;
    dev->addr = MAX30102_SENSOR_ADDR;
    dev->cfg.sda_io_num = sda_gpio;
    dev->cfg.scl_io_num = scl_gpio;
#if defined(CONFIG_IDF_TARGET_ESP32)
    dev->cfg.master.clk_speed = I2C_FREQ_HZ;
#endif
    i2c_dev_create_mutex(dev);

    return ESP_OK;
}

esp_err_t max30102_bitmask(uint8_t reg, uint8_t mask, uint8_t thing, i2c_dev_t *dev) {
    CHECK_ARG(dev);

    uint8_t data;

    I2C_DEV_TAKE_MUTEX(dev);
    I2C_DEV_CHECK(dev, i2c_dev_read_reg(dev, reg, &data, sizeof(data)));

    data = data & mask;
    data = data | thing;

    I2C_DEV_CHECK(dev, i2c_dev_write_reg(dev, reg, &data, sizeof(data)));
    I2C_DEV_GIVE_MUTEX(dev);

    return ESP_OK;
}

esp_err_t max30102_setLEDMode(uint8_t led_mode, i2c_dev_t *dev) {
    if (max30102_bitmask(MAX30102_MODE_CFG_ADDR, MAX30102_MODE_MASK, led_mode, dev) == ESP_OK)
    {
        return ESP_OK;
    }
    else
    {
        return ESP_ERR_NOT_FOUND;
    }
}

esp_err_t max30102_setADCRange(uint8_t adcRange, i2c_dev_t *dev) {
    if (max30102_bitmask(MAX30102_SPO2_CFG_ADDR, MAX30102_ADCRANGE_MASK, adcRange, dev) == ESP_OK)
    {
        return ESP_OK;
    }
    else
    {
        return ESP_ERR_NOT_FOUND;
    }
}

esp_err_t max30102_setSampleRate(uint8_t sampleRate, i2c_dev_t *dev) {
    if (max30102_bitmask(MAX30102_SPO2_CFG_ADDR, MAX30102_SAMPLERATE_MASK, sampleRate, dev) == ESP_OK)
    {
        return ESP_OK;
    }
    else
    {
        return ESP_ERR_NOT_FOUND;
    }
}

esp_err_t max30102_setPulseWidth(uint8_t pulseWidth, i2c_dev_t *dev) {
    if (max30102_bitmask(MAX30102_SPO2_CFG_ADDR, MAX30102_PULSEWIDTH_MASK, pulseWidth, dev) == ESP_OK)
    {
        return ESP_OK;
    }
    else
    {
        return ESP_ERR_NOT_FOUND;
    }
}

esp_err_t max30102_setLedAmplitude(uint8_t led, uint8_t amplitude, i2c_dev_t *dev) {
    uint8_t reg;
    if (led == 1) {
        reg = MAX30102_LED1_PA_ADDR;
    }
    else if (led == 2) {
        reg = MAX30102_LED2_PA_ADDR;
    }
    else
    {
        return ESP_ERR_NOT_FOUND;
    }
    

    I2C_DEV_TAKE_MUTEX(dev);

    I2C_DEV_CHECK(dev, i2c_dev_write_reg(dev, reg, &amplitude, 1));

    I2C_DEV_GIVE_MUTEX(dev);

    return ESP_OK;
}

esp_err_t max30102_enableSlot(uint8_t slotNumber, uint8_t device, i2c_dev_t *dev) {
    esp_err_t ret = ESP_OK;

    switch (slotNumber) {
        case (1):
            ret = max30102_bitmask(MAX30102_SLOT_1_2_ADDR, MAX30102_SLOT1_MASK, device, dev);
            break;
        case (2):
            ret = max30102_bitmask(MAX30102_SLOT_1_2_ADDR, MAX30102_SLOT2_MASK, device << 4, dev);
            break;
        case (3):
            ret = max30102_bitmask(MAX30102_SLOT_3_4_ADDR, MAX30102_SLOT3_MASK, device, dev);
            break;
        case (4):
            ret = max30102_bitmask(MAX30102_SLOT_3_4_ADDR, MAX30102_SLOT4_MASK, device << 4, dev);
            break;
        default:
        //Shouldn't be here!
        break;
    }
    return ret;
}

esp_err_t max30102_disableSlots(i2c_dev_t *dev) {
    I2C_DEV_TAKE_MUTEX(dev);

    I2C_DEV_CHECK(dev, i2c_dev_write_reg(dev, MAX30102_SLOT_1_2_ADDR, 0, 1));
    I2C_DEV_CHECK(dev, i2c_dev_write_reg(dev, MAX30102_SLOT_3_4_ADDR, 0, 1));

    I2C_DEV_GIVE_MUTEX(dev);

    return ESP_OK;
}

esp_err_t max30102_clearFIFO(i2c_dev_t *dev) {
    uint8_t data = 0x00;
    I2C_DEV_TAKE_MUTEX(dev);

    I2C_DEV_CHECK(dev, i2c_dev_write_reg(dev, MAX30102_FIFO_WR_PTR_ADDR, &data, 1));
    I2C_DEV_CHECK(dev, i2c_dev_write_reg(dev, MAX30102_FIFO_RD_PTR_ADDR, &data, 1));
    I2C_DEV_CHECK(dev, i2c_dev_write_reg(dev, MAX30102_FIFO_DATA_ADDR, &data, 1));

    I2C_DEV_GIVE_MUTEX(dev);

    return ESP_OK;
}

esp_err_t max30102_enableFIFORollover(i2c_dev_t *dev) {
    if (max30102_bitmask(MAX30102_FIFO_CFG_ADDR, MAX30102_ROLLOVER_MASK, MAX30102_ROLLOVER_ENABLE, dev) == ESP_OK)
    {
        return ESP_OK;
    }
    else
    {
        return ESP_ERR_NOT_FOUND;
    }
}

esp_err_t max30102_disableFIFORollover(i2c_dev_t *dev) {
    if (max30102_bitmask(MAX30102_FIFO_CFG_ADDR, MAX30102_ROLLOVER_MASK, MAX30102_ROLLOVER_DISABLE, dev) == ESP_OK)
    {
        return ESP_OK;
    }
    else
    {
        return ESP_ERR_NOT_FOUND;
    }
}

esp_err_t max30102_setFIFOAvarage(uint8_t numberOfSamples, i2c_dev_t *dev) {
    if (max30102_bitmask(MAX30102_FIFO_CFG_ADDR, MAX30102_SAMPLEAVG_MASK, numberOfSamples, dev) == ESP_OK)
    {
        return ESP_OK;
    }
    else
    {
        return ESP_ERR_NOT_FOUND;
    }
}


uint8_t max30102_getWritePointer(i2c_dev_t *dev) {
    uint8_t data;

    I2C_DEV_TAKE_MUTEX(dev);

    I2C_DEV_CHECK(dev, i2c_dev_read_reg(dev, MAX30102_FIFO_WR_PTR_ADDR, &data, 1));

    I2C_DEV_GIVE_MUTEX(dev);

    return data;
}

uint8_t max30102_getReadPointer(i2c_dev_t *dev) {
    uint8_t data;

    I2C_DEV_TAKE_MUTEX(dev);

    I2C_DEV_CHECK(dev, i2c_dev_read_reg(dev, MAX30102_FIFO_RD_PTR_ADDR, &data, 1));

    I2C_DEV_GIVE_MUTEX(dev);

    return data;
}

esp_err_t max30102_softReset(i2c_dev_t *dev) {
    if (max30102_bitmask(MAX30102_MODE_CFG_ADDR, MAX30102_RESET_MASK, MAX30102_RESET, dev) == ESP_OK)
    {
        return ESP_OK;
    }
    else
    {
        return ESP_ERR_NOT_FOUND;
    }
    
    
}

esp_err_t max30102_init(uint8_t powerLevel, uint8_t sampleAverage, uint8_t ledMode, int sampleRate, int pulseWidth,
                    int adcRange,struct max30102_record *record, i2c_dev_t *dev) 
{
    esp_err_t ret = ESP_OK;
    ret = max30102_softReset(dev);
    vTaskDelay(pdMS_TO_TICKS(100));

    //FIFO Configuration
    //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //The chip will average multiple samples of same type together if you wish
    if (sampleAverage == 1) ret |= max30102_setFIFOAvarage(MAX30102_SAMPLEAVG_1, dev); //No averaging per FIFO record
    else if (sampleAverage == 2) ret |= max30102_setFIFOAvarage(MAX30102_SAMPLEAVG_2, dev);
    else if (sampleAverage == 4) ret |= max30102_setFIFOAvarage(MAX30102_SAMPLEAVG_4, dev);
    else if (sampleAverage == 8) ret |= max30102_setFIFOAvarage(MAX30102_SAMPLEAVG_8, dev);
    else if (sampleAverage == 16) ret |= max30102_setFIFOAvarage(MAX30102_SAMPLEAVG_16, dev);
    else if (sampleAverage == 32) ret |= max30102_setFIFOAvarage(MAX30102_SAMPLEAVG_32, dev);
    else ret |= max30102_setFIFOAvarage(MAX30102_SAMPLEAVG_4, dev);
    
    //Allow FIFO to wrap/roll over
    ret |= max30102_enableFIFORollover(dev);

    //Mode Configuration
    //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    if (ledMode == 3) ret |= max30102_setLEDMode(MAX30102_MODE_MULTILED, dev); //Watch all three LED channels
    else if (ledMode == 2) ret |= max30102_setLEDMode(MAX30102_MODE_REDIRONLY, dev);//Red and IR
    else ret |= max30102_setLEDMode(MAX30102_MODE_REDONLY, dev); //Red only
    record->activeLEDs = ledMode; //Used to control how many bytes to read from FIFO buffer

    //Particle Sensing Configuration
    //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    if(adcRange < 4096) ret |= max30102_setADCRange(MAX30102_ADCRANGE_2048, dev); //7.81pA per LSB
    else if(adcRange < 8192) ret |= max30102_setADCRange(MAX30102_ADCRANGE_4096, dev); //15.63pA per LSB
    else if(adcRange < 16384) ret |= max30102_setADCRange(MAX30102_ADCRANGE_8192, dev);//31.25pA per LSB
    else if(adcRange == 16384) ret |= max30102_setADCRange(MAX30102_ADCRANGE_16384, dev); //62.5pA per LSB
    else ret |= max30102_setADCRange(MAX30102_ADCRANGE_2048, dev);

    if (sampleRate < 100) ret |= max30102_setSampleRate(MAX30102_SAMPLERATE_50, dev);//Take 50 samples per second
    else if (sampleRate < 200) ret |= max30102_setSampleRate(MAX30102_SAMPLERATE_100, dev);
    else if (sampleRate < 400) ret |= max30102_setSampleRate(MAX30102_SAMPLERATE_200, dev);
    else if (sampleRate < 800) ret |= max30102_setSampleRate(MAX30102_SAMPLERATE_400, dev);
    else if (sampleRate < 1000) ret |= max30102_setSampleRate(MAX30102_SAMPLERATE_800, dev);
    else if (sampleRate < 1600) ret |= max30102_setSampleRate(MAX30102_SAMPLERATE_1000, dev);
    else if (sampleRate < 3200) ret |= max30102_setSampleRate(MAX30102_SAMPLERATE_1600, dev);
    else if (sampleRate == 3200) ret |= max30102_setSampleRate(MAX30102_SAMPLERATE_3200, dev);
    else ret |= max30102_setSampleRate(MAX30102_SAMPLERATE_50, dev);

    //The longer the pulse width the longer range of detection you'll have
    //At 69us and 0.4mA it's about 2 inches
    //At 411us and 0.4mA it's about 6 inches
    if (pulseWidth < 118) ret |= max30102_setPulseWidth(MAX30102_PULSEWIDTH_69, dev); //Page 26, Gets us 15 bit resolution
    else if (pulseWidth < 215) ret |= max30102_setPulseWidth(MAX30102_PULSEWIDTH_118, dev); //16 bit resolution
    else if (pulseWidth < 411) ret |= max30102_setPulseWidth(MAX30102_PULSEWIDTH_215, dev); //17 bit resolution
    else if (pulseWidth == 411) ret |= max30102_setPulseWidth(MAX30102_PULSEWIDTH_411, dev); //18 bit resolution
    else ret |= max30102_setPulseWidth(MAX30102_PULSEWIDTH_69, dev);

    //LED Pulse Amplitude Configuration
    //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //Default is 0x1F which gets us 6.4mA
    //powerLevel = 0x02, 0.4mA - Presence detection of ~4 inch
    //powerLevel = 0x1F, 6.4mA - Presence detection of ~8 inch
    //powerLevel = 0x7F, 25.4mA - Presence detection of ~8 inch
    //powerLevel = 0xFF, 50.0mA - Presence detection of ~12 inch

    ret |= max30102_setLedAmplitude(1, powerLevel, dev); 
    ret |= max30102_setLedAmplitude(2, powerLevel, dev); 

    //Multi-LED Mode Configuration, Enable the reading of the three LEDs
    //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    ret |= max30102_enableSlot(1, SLOT_RED_LED, dev); 
    if (ledMode > 1) ret |= max30102_enableSlot(2, SLOT_IR_LED, dev); 

    //Reset the FIFO before we begin checking the sensor
    ret |= max30102_clearFIFO(dev); 

    return ret;
}

int max30102_available(struct max30102_record *record) {
    int numberOfSamples = record->head - record->tail;
    if (numberOfSamples < 0) {
        numberOfSamples += STORAGE_SIZE;
    }
    return (numberOfSamples);
}

int16_t max30102_check(struct max30102_record *record, i2c_dev_t *dev) {
    //Read register FIDO_DATA in (3-byte * number of active LED) chunks
    //Until FIFO_RD_PTR = FIFO_WR_PTR

    uint8_t readPointer = max30102_getReadPointer(dev);
    uint8_t writePointer = max30102_getWritePointer(dev);

    int16_t numberOfSamples = 0;

    //Do we have new data?
    if (readPointer != writePointer)
    {
        //Calculate the number of readings we need to get from sensor
        numberOfSamples = (int16_t)(writePointer - readPointer);
        if (numberOfSamples < 0) numberOfSamples += 32; //Wrap condition

        //We now have the number of readings, now calc bytes to read
        //For this example we are just doing Red and IR (3 bytes each)
        int bytesLeftToRead = (int)(numberOfSamples * record->activeLEDs * 3);
        
        //We may need to read as many as 288 bytes so we read in blocks no larger than I2C_BUFFER_LENGTH
        //I2C_BUFFER_LENGTH changes based on the platform. 64 bytes for SAMD21, 32 bytes for Uno.
        //Wire.requestFrom() is limited to BUFFER_LENGTH which is 32 on the Uno
        while (bytesLeftToRead > 0)
        {
            int toGet = bytesLeftToRead;
            if (toGet > 32)
            {
                //If toGet is 32 this is bad because we read 6 bytes (Red+IR * 3 = 6) at a time
                //32 % 6 = 2 left over. We don't want to request 32 bytes, we want to request 30.
                //32 % 9 (Red+IR+GREEN) = 5 left over. We want to request 27.

                toGet = 32 - (32 % (record->activeLEDs * 3)); //Trim toGet to be a multiple of the samples we need to read
            }

            bytesLeftToRead -= toGet;
            uint8_t data_temp[toGet]; //Array of 6 bytes that we will convert into long: Red and IR

            // read bytes: for Red, for IR
            I2C_DEV_TAKE_MUTEX(dev);
            I2C_DEV_CHECK(dev, i2c_dev_read_reg(dev, MAX30102_FIFO_DATA_ADDR, &data_temp, toGet));
            I2C_DEV_GIVE_MUTEX(dev);

            // i2c_dev_read_reg(dev, MAX30102_FIFO_DATA_ADDR, &data_temp, toGet);

            uint8_t read_count = 0;
            
            while (toGet > 0)
            {
                record->head++; //Advance the head of the storage struct
                record->head %= STORAGE_SIZE; //Wrap condition

                uint8_t temp[sizeof(uint32_t)]; //Array of 4 bytes that we will convert into long
                uint32_t tempLong;

                temp[3] = 0;
                temp[2] = data_temp[read_count++];
                temp[1] = data_temp[read_count++];
                temp[0] = data_temp[read_count++];

                //Convert array to long
                memcpy(&tempLong, temp, sizeof(tempLong));
                
                tempLong &= 0x3FFFF; //Zero out all but 18 bits

                record->red[record->head] = tempLong; //Store this reading into the sense array

                // If not just read only Red 
                if (record->activeLEDs > 1)
                {
                    temp[3] = 0;
                    temp[2] = data_temp[read_count++];
                    temp[1] = data_temp[read_count++];
                    temp[0] = data_temp[read_count++];

                    //Convert array to long
                    memcpy(&tempLong, temp, sizeof(tempLong));

                    tempLong &= 0x3FFFF; //Zero out all but 18 bits
                    
                    record->IR[record->head] = tempLong;
                }

                toGet -= record->activeLEDs * 3;
            }

        } //End while (bytesLeftToRead > 0)

    } //End readPtr != writePtr

    return (numberOfSamples); //Let the world know how much new data we found
}

esp_err_t max30102_safeCheck(uint8_t maxTimeToCheck, struct max30102_record *record, i2c_dev_t *dev) {
    uint64_t markTime = esp_timer_get_time();
  
    while(1)
    {
        if (esp_timer_get_time() - markTime > maxTimeToCheck * 1000) {
            printf("Safe check time out\n");
            return false;
        }

        if (max30102_check(record, dev) != 0) //We found new data!
            return(true);
    }
}

esp_err_t max30102_getSensorData(uint8_t led, struct max30102_data *comp_data, struct max30102_record *record, i2c_dev_t *dev) {
    //Check the sensor for new data for 250ms
    if (max30102_safeCheck(250, record, dev)) {
        if (led == 1)
        {
            comp_data->red = record->red[record->head];
        }
        else if (led == 2) 
        {
            comp_data->ir = record->IR[record->head];
        }
        else {
            return ESP_ERR_NOT_FINISHED;
        }
        return ESP_OK;
    }
    else {
        return ESP_ERR_NOT_FINISHED; //Sensor failed to find new data
    }
}

uint32_t max30102_getFIFORed(struct max30102_record *record)
{
  return (record->red[record->tail]);
}

uint32_t max30102_getFIFOIR(struct max30102_record *record)
{
  return (record->IR[record->tail]);
}

esp_err_t max30102_nextSample(struct max30102_record *record)
{
  if (max30102_available(record)) //Only advance the tail if new data is available
  {
    record->tail++;
    record->tail %= STORAGE_SIZE; //Wrap condition
    return ESP_OK;
  }
  
  return ESP_ERR_INVALID_STATE;
}