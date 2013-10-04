################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

EXECUTABLE=dcd
RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
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
all: dcd

# Tool invocations
dcd: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -Wall -L/usr/lib64/mysql -L/usr/lib -o $(EXECUTABLE) $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) $(EXECUTABLE)
	-@echo ' '

install:
	install $(EXECUTABLE) /usr/bin/$(EXECUTABLE)
	install $(EXECUTABLE).service /usr/lib/systemd/system/$(EXECUTABLE).service
	install $(EXECUTABLE).timer /etc/systemd/system/$(EXECUTABLE).timer
	install $(EXECUTABLE).conf /etc/$(EXECUTABLE).conf
	systemctl daemon-reload

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
