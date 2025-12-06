################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Games/Snake_game.c \
../Core/Src/Games/T_rex.c 

OBJS += \
./Core/Src/Games/Snake_game.o \
./Core/Src/Games/T_rex.o 

C_DEPS += \
./Core/Src/Games/Snake_game.d \
./Core/Src/Games/T_rex.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Games/%.o Core/Src/Games/%.su Core/Src/Games/%.cyclo: ../Core/Src/Games/%.c Core/Src/Games/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Games

clean-Core-2f-Src-2f-Games:
	-$(RM) ./Core/Src/Games/Snake_game.cyclo ./Core/Src/Games/Snake_game.d ./Core/Src/Games/Snake_game.o ./Core/Src/Games/Snake_game.su ./Core/Src/Games/T_rex.cyclo ./Core/Src/Games/T_rex.d ./Core/Src/Games/T_rex.o ./Core/Src/Games/T_rex.su

.PHONY: clean-Core-2f-Src-2f-Games

