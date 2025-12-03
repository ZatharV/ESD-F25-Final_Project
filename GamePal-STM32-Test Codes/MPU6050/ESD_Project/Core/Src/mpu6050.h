#ifndef MPU6050_H
#define MPU6050_H
#include "stm32f429xx.h"
#include <stdint.h>

#define MPU6050_I2C_ADDR            0x68 // I2C address (AD0 low)

// MPU6050 Register Map
#define MPU6050_REG_ACCEL_XOUT_H    0x3B
#define MPU6050_REG_ACCEL_XOUT_L    0x3C
#define MPU6050_REG_ACCEL_YOUT_H    0x3D
#define MPU6050_REG_ACCEL_YOUT_L    0x3E
#define MPU6050_REG_ACCEL_ZOUT_H    0x3F
#define MPU6050_REG_ACCEL_ZOUT_L    0x40
#define MPU6050_REG_TEMP_OUT_H      0x41
#define MPU6050_REG_TEMP_OUT_L      0x42
#define MPU6050_REG_GYRO_XOUT_H     0x43
#define MPU6050_REG_GYRO_XOUT_L     0x44
#define MPU6050_REG_GYRO_YOUT_H     0x45
#define MPU6050_REG_GYRO_YOUT_L     0x46
#define MPU6050_REG_GYRO_ZOUT_H     0x47
#define MPU6050_REG_GYRO_ZOUT_L     0x48
#define MPU6050_REG_PWR_MGMT_1      0x6B
#define MPU6050_REG_WHO_AM_I        0x75

typedef struct {
    I2C_TypeDef*    i2c;       
    GPIO_TypeDef*   sclPort;  
    uint8_t         sclPin; 
    GPIO_TypeDef*   sdaPort;  
    uint8_t         sdaPin; 
} I2C_Config_t;


typedef struct {
    int16_t Accel_X;
    int16_t Accel_Y;
    int16_t Accel_Z;
    int16_t Temp;
    int16_t Gyro_X;
    int16_t Gyro_Y;
    int16_t Gyro_Z;
} MPU6050_Data_t;

uint8_t MPU6050_Init(I2C_Config_t* config);
void MPU6050_Read_All(I2C_Config_t* config, MPU6050_Data_t* data);
void MPU6050_Write_Byte(I2C_Config_t* config, uint8_t reg, uint8_t data);
uint8_t MPU6050_Read_Byte(I2C_Config_t* config, uint8_t reg);
void MPU6050_Read_Burst(I2C_Config_t* config, uint8_t start_reg, uint8_t* buffer, uint8_t count);


#endif // MPU6050_H