
OUT_DIR += $(SRC_DIR) \
$(SRC_DIR)/patch_z_sdk \
$(SRC_DIR)/custom_zcl

OBJS += \
$(OUT_PATH)$(SRC_DIR)/patch_z_sdk/cstartup_8258.o \
$(OUT_PATH)$(SRC_DIR)/div_mod.o \
$(OUT_PATH)$(SRC_DIR)/main.o \
$(OUT_PATH)$(SRC_DIR)/battery.o \
$(OUT_PATH)$(SRC_DIR)/custom_zcl/zcl_reporting.o \
$(OUT_PATH)$(SRC_DIR)/custom_zcl/zcl_onoffSwitchCfg.o \
$(OUT_PATH)$(SRC_DIR)/custom_zcl/zcl_se_metering.o \
$(OUT_PATH)$(SRC_DIR)/zb_appCb.o \
$(OUT_PATH)$(SRC_DIR)/zcl_appCb.o \
$(OUT_PATH)$(SRC_DIR)/app_endpoint_cfg.o \
$(OUT_PATH)$(SRC_DIR)/app_reporting.o \
$(OUT_PATH)$(SRC_DIR)/app_utility.o \
$(OUT_PATH)$(SRC_DIR)/app_onoff.o \
$(OUT_PATH)$(SRC_DIR)/app_led.o \
$(OUT_PATH)$(SRC_DIR)/app_button.o \
$(OUT_PATH)$(SRC_DIR)/app_switch.o \
$(OUT_PATH)$(SRC_DIR)/app_arith64.o \
$(OUT_PATH)$(SRC_DIR)/app_relay.o \
$(OUT_PATH)$(SRC_DIR)/app_bl0942.o \
$(OUT_PATH)$(SRC_DIR)/app_bl0937.o \
$(OUT_PATH)$(SRC_DIR)/my18b20.o \
$(OUT_PATH)$(SRC_DIR)/energy_save.o \
$(OUT_PATH)$(SRC_DIR)/app_main.o \
$(OUT_PATH)$(SRC_DIR)/patch_z_sdk/flash.o \
$(OUT_PATH)$(SRC_DIR)/patch_z_sdk/flash_drv.o \
$(OUT_PATH)$(SRC_DIR)/patch_z_sdk/adc_drv.o \
$(OUT_PATH)$(SRC_DIR)/patch_z_sdk/random.o \
$(OUT_PATH)$(SRC_DIR)/patch_z_sdk/i2c_drv.o \
$(OUT_PATH)$(SRC_DIR)/patch_z_sdk/hw_drv.o \
$(OUT_PATH)$(SRC_DIR)/patch_z_sdk/drv_nv.o


# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)$(SRC_DIR)/%.o: $(PROJECT_PATH)$(SRC_DIR)/%.c
	@echo 'Building file: $<'
	@$(TC32_PATH)tc32-elf-gcc $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"
	
$(OUT_PATH)$(SRC_DIR)/%.o: $(PROJECT_PATH)$(SRC_DIR)/%.S
	@echo 'Building file: $<'
	@$(TC32_PATH)tc32-elf-gcc $(GCC_FLAGS) $(ASM_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"