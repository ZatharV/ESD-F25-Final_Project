#include "mpu6050.h"

#define I2C_TIMEOUT_MAX  10000

static int32_t I2C_Wait_Flag(I2C_TypeDef* i2c, uint32_t flag) {
    volatile uint32_t timeout = I2C_TIMEOUT_MAX;
    while (!(i2c->SR1 & flag)) {
        if (timeout-- == 0) return -1;
    }
    return 0;
}

static void Enable_GPIO_Clock(GPIO_TypeDef* port) {
    if (port == GPIOA) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    else if (port == GPIOB) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    else if (port == GPIOC) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    else if (port == GPIOD) RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    else if (port == GPIOE) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    else if (port == GPIOF) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
    else if (port == GPIOG) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    else if (port == GPIOH) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
    else if (port == GPIOI) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
}

uint8_t MPU6050_Init(I2C_Config_t* config) {
    I2C_TypeDef* i2c = config->i2c;

    /* 1. Enable Clocks (I2C and GPIO) */
    if (i2c == I2C1) RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    else if (i2c == I2C2) RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    else if (i2c == I2C3) RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
    else return 0;

    Enable_GPIO_Clock(config->sclPort);
    Enable_GPIO_Clock(config->sdaPort);

    config->sclPort->MODER   &= ~(3U << (config->sclPin * 2));
    config->sclPort->MODER   |=  (2U << (config->sclPin * 2));
    config->sclPort->OTYPER  |=  (1U << config->sclPin);      
    config->sclPort->OSPEEDR |=  (3U << (config->sclPin * 2));
    config->sclPort->PUPDR   &= ~(3U << (config->sclPin * 2));
    config->sclPort->PUPDR   |=  (1U << (config->sclPin * 2));
    config->sclPort->AFR[config->sclPin / 8] &= ~(0xFU << ((config->sclPin % 8) * 4));
    config->sclPort->AFR[config->sclPin / 8] |=  (4U << ((config->sclPin % 8) * 4)); 

    config->sdaPort->MODER   &= ~(3U << (config->sdaPin * 2));
    config->sdaPort->MODER   |=  (2U << (config->sdaPin * 2)); 
    config->sdaPort->OTYPER  |=  (1U << config->sdaPin);       
    config->sdaPort->OSPEEDR |=  (3U << (config->sdaPin * 2)); 
    config->sdaPort->PUPDR   &= ~(3U << (config->sdaPin * 2));
    config->sdaPort->PUPDR   |=  (1U << (config->sdaPin * 2)); 
    config->sdaPort->AFR[config->sdaPin / 8] &= ~(0xFU << ((config->sdaPin % 8) * 4));
    config->sdaPort->AFR[config->sdaPin / 8] |=  (4U << ((config->sdaPin % 8) * 4));

    i2c->CR1 &= ~I2C_CR1_PE;

    i2c->CR1 |= I2C_CR1_SWRST;
    i2c->CR1 &= ~I2C_CR1_SWRST;

    i2c->CR2 = 42;

    i2c->CCR = 210;

    i2c->TRISE = 43;

    i2c->CR1 |= I2C_CR1_PE;

    MPU6050_Write_Byte(config, MPU6050_REG_PWR_MGMT_1, 0x00);
    return MPU6050_Read_Byte(config, MPU6050_REG_WHO_AM_I);
}

void MPU6050_Write_Byte(I2C_Config_t* config, uint8_t reg, uint8_t data) {
    I2C_TypeDef* i2c = config->i2c;

    i2c->CR1 |= I2C_CR1_START;
    if (I2C_Wait_Flag(i2c, I2C_SR1_SB) != 0) return;

    i2c->DR = (MPU6050_I2C_ADDR << 1) | 0;
    if (I2C_Wait_Flag(i2c, I2C_SR1_ADDR) != 0) return;
    (void)i2c->SR1;
    (void)i2c->SR2;

    if (I2C_Wait_Flag(i2c, I2C_SR1_TXE) != 0) return;
    i2c->DR = reg;

    if (I2C_Wait_Flag(i2c, I2C_SR1_TXE) != 0) return;
    i2c->DR = data;

    if (I2C_Wait_Flag(i2c, I2C_SR1_BTF) != 0) return;
    i2c->CR1 |= I2C_CR1_STOP;
}

uint8_t MPU6050_Read_Byte(I2C_Config_t* config, uint8_t reg) {
    I2C_TypeDef* i2c = config->i2c;
    uint8_t data = 0;

    i2c->CR1 |= I2C_CR1_START;
    if (I2C_Wait_Flag(i2c, I2C_SR1_SB) != 0) return 0;

    i2c->DR = (MPU6050_I2C_ADDR << 1) | 0;
    if (I2C_Wait_Flag(i2c, I2C_SR1_ADDR) != 0) return 0;
    (void)i2c->SR1;
    (void)i2c->SR2;

    if (I2C_Wait_Flag(i2c, I2C_SR1_TXE) != 0) return 0;
    i2c->DR = reg;
    if (I2C_Wait_Flag(i2c, I2C_SR1_BTF) != 0) return 0;

    i2c->CR1 |= I2C_CR1_START;
    if (I2C_Wait_Flag(i2c, I2C_SR1_SB) != 0) return 0;

    i2c->DR = (MPU6050_I2C_ADDR << 1) | 1;
    if (I2C_Wait_Flag(i2c, I2C_SR1_ADDR) != 0) return 0;

    i2c->CR1 &= ~I2C_CR1_ACK;
    (void)i2c->SR1;
    (void)i2c->SR2;
    i2c->CR1 |= I2C_CR1_STOP;

    if (I2C_Wait_Flag(i2c, I2C_SR1_RXNE) != 0) return 0;
    data = (uint8_t)i2c->DR;

    i2c->CR1 |= I2C_CR1_ACK;

    return data;
}

void MPU6050_Read_Burst(I2C_Config_t* config, uint8_t start_reg, uint8_t* buffer, uint8_t count) {
    I2C_TypeDef* i2c = config->i2c;

    if (count == 0) return;
    if (count == 1) {
        buffer[0] = MPU6050_Read_Byte(config, start_reg);
        return;
    }

    i2c->CR1 |= I2C_CR1_START;
    if (I2C_Wait_Flag(i2c, I2C_SR1_SB) != 0) return;

    i2c->DR = (MPU6050_I2C_ADDR << 1) | 0;
    if (I2C_Wait_Flag(i2c, I2C_SR1_ADDR) != 0) return;
    (void)i2c->SR1;
    (void)i2c->SR2;
}