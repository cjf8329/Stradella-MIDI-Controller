cmake_minimum_required(VERSION 3.20)

# Set target system and processor BEFORE project()
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Explicitly clear any Apple host architecture flags
set(CMAKE_OSX_ARCHITECTURES "")

# Specify toolchain
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

# Project definition must come AFTER setting CMAKE_SYSTEM_NAME
project(cmsis C ASM)

# 3. Target executable
add_executable(${PROJECT_NAME}
    Src/main.c
    Src/clock.c
    Src/led.c
    Src/systick.c
    Src/lpuart.c
    Drivers/CMSIS/Device/ST/STM32G4xx/Source/system_stm32g4xx.c
    Drivers/CMSIS/Device/ST/STM32G4xx/Source/startup_stm32g474xx.s
)

# 4. Include paths
target_include_directories(${PROJECT_NAME} PRIVATE
    Inc
    Drivers/CMSIS/Include
    Drivers/CMSIS/Device/ST/STM32G4xx/Include
)

# 5. Device definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    STM32G474xx
)

# 6. Target MCU flags (Cortex-M4 + Hardware Single-Precision FPU)
set(MCU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

target_compile_options(${PROJECT_NAME} PRIVATE
    ${MCU_FLAGS}
    -Wall
    -Wextra
    -fdata-sections
    -ffunction-sections
    $<$<CONFIG:Debug>:-g3 -O0>
    $<$<CONFIG:Release>:-O2>
)

# 7. Linker settings
target_link_options(${PROJECT_NAME} PRIVATE
    ${MCU_FLAGS}
    -T${CMAKE_CURRENT_SOURCE_DIR}/stm32g474xe_flash.ld
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    --specs=nano.specs
    --specs=nosys.specs
)

# 8. Post-build HEX and BIN generation
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND arm-none-eabi-objcopy -O binary $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMENT "Generating HEX and BIN"
)