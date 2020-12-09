################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include emlib/subdir.mk
-include Main/subdir.mk
-include Drivers/subdir.mk
-include CMSIS/EFM32GG/subdir.mk
-include BSP/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: HW_EMbeddedSys_Bruna.axf

# Tool invocations
HW_EMbeddedSys_Bruna.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m3 -mthumb -T "HW_EMbeddedSys_Bruna.ld" -Xlinker --gc-sections -Xlinker -Map="HW_EMbeddedSys_Bruna.map" --specs=nano.specs -o HW_EMbeddedSys_Bruna.axf "./BSP/bsp_trace.o" "./CMSIS/EFM32GG/startup_gcc_efm32gg.o" "./CMSIS/EFM32GG/system_efm32gg.o" "./Drivers/segmentlcd.o" "./Main/lightsense.o" "./emlib/em_acmp.o" "./emlib/em_assert.o" "./emlib/em_cmu.o" "./emlib/em_core.o" "./emlib/em_emu.o" "./emlib/em_gpio.o" "./emlib/em_lcd.o" "./emlib/em_lesense.o" "./emlib/em_pcnt.o" "./emlib/em_prs.o" "./emlib/em_rtc.o" "./emlib/em_system.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: HW_EMbeddedSys_Bruna.hex'
	arm-none-eabi-objcopy -O ihex "HW_EMbeddedSys_Bruna.axf" "HW_EMbeddedSys_Bruna.hex"
	@echo ' '

	@echo 'Building bin file: HW_EMbeddedSys_Bruna.bin'
	arm-none-eabi-objcopy -O binary "HW_EMbeddedSys_Bruna.axf" "HW_EMbeddedSys_Bruna.bin"
	@echo ' '

	@echo 'Building s37 file: HW_EMbeddedSys_Bruna.s37'
	arm-none-eabi-objcopy -O srec "HW_EMbeddedSys_Bruna.axf" "HW_EMbeddedSys_Bruna.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "HW_EMbeddedSys_Bruna.axf" -A
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) HW_EMbeddedSys_Bruna.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
