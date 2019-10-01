cmake_minimum_required (VERSION 3.6)
SET (CMAKE_VERBOSE_MAKEFILE off)

set(PROJ "bluenrg1")
PROJECT (bluenrg1)

SET (CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
#set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/Src)
#set(CMSIS_DIR ${CMAKE_SOURCE_DIR}/Drivers/CMSIS)
#set(DEVICE_DIR ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F0xx)
#set(HAL_DIR ${CMAKE_SOURCE_DIR}/Drivers/STM32F0xx_HAL_Driver)

INCLUDE_DIRECTORIES("inc/")
INCLUDE_DIRECTORIES("../../Library/hal/inc/")
INCLUDE_DIRECTORIES("../../Library/BlueNRG1_Periph_Driver/inc/")
INCLUDE_DIRECTORIES("../../Library/Bluetooth_LE/inc/")
INCLUDE_DIRECTORIES("../../Library/CMSIS/Include/")
INCLUDE_DIRECTORIES("../../Library/CMSIS/Device/ST/BlueNRG1/Include")
INCLUDE_DIRECTORIES("../../Drivers/BSP/Components/lsm6dsl/")
INCLUDE_DIRECTORIES("../../Drivers/BSP/Components/Common/")
INCLUDE_DIRECTORIES("../../Drivers/BSP/STEVAL-BLUEMIC1/")
INCLUDE_DIRECTORIES("../../Middlewares/ST/BlueNRG-1_BlueVoice_Library/Inc")

if("${CMAKE_VERSION}" VERSION_GREATER 3.6.0)
	set(CMAKE_C_COMPILER   "arm-none-eabi-gcc")
	set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
	# without these cmake tries to compile/link a test and fails on _exit
	# this _was_ suppressed by the now deprecated FORCE versions
	set(CMAKE_C_COMPILER_WORKS   1)
	set(CMAKE_CXX_COMPILER_WORKS 1)
else()
	# the _force_ macro's are deprecated as of 3.6
	CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc GNU)
	CMAKE_FORCE_CXX_COMPILER(arm-none-eabi-g++ GNU)
endif()

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_CROSSCOMPILING 1)

# set compiler flags to optimize for code size
SET(CMAKE_C_FLAGS_DEBUG "-Og" CACHE STRING "")
SET(CMAKE_C_FLAGS_RELEASE "-Os" CACHE STRING "")

SET(TOOLCHAIN_DIR "" CACHE PATH "The directory containing all the cross compilation tools. (Compilation will fail if this is not set correctly)")

# where is the target environment 
LIST(APPEND CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_DIR}")

MESSAGE(STATUS "Cross-compiling using gcc-arm-embedded toolchain")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_FLAGS "-O0 -g -Wall -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBlueNRG-132 -mcpu=cortex-m0 -mthumb")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mno-thumb-interwork -mfpu=vfp -msoft-float")
set(CMAKE_EXE_LINKER_FLAGS "-T../BlueNRG1.ld -Wl,--gc-section")

# Some trickery to get CMake to deal with our assembler code.
ENABLE_LANGUAGE (ASM)
set_property(SOURCE /Users/maartenweyn/git/teamscheire_tatjana/en.stsw-bluemic-1/Library/hal/src/context_switch2.s PROPERTY LANGUAGE ASM)

add_definitions("-D__weak=__attribute__((weak))")
ADD_DEFINITIONS (-DHS_SPEED_XTAL=HS_SPEED_XTAL_32MHZ -DLS_SOURCE=LS_SOURCE_EXTERNAL_32KHZ -DSMPS_INDUCTOR=SMPS_INDUCTOR_10uH)
add_definitions("-DUSE_FULL_ASSERT")

LINK_DIRECTORIES(/Users/maartenweyn/git/teamscheire_tatjana/en.stsw-bluemic-1/Library/Bluetooth_LE/library/ /Users/maartenweyn/git/teamscheire_tatjana/en.stsw-bluemic-1/Middlewares/ST/BlueNRG-1_BlueVoice_Library/Lib/)


# +-----------------+
# | My code         |
# +-----------------+
ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}.elf
        "src/BlueNRG1_it.c"
        "src/bluevoice_app.c"
        "src/peripheral_mngr_app.c"
        "src/inertial_app.c"
        "src/main.c"
        "inc/BlueNRG1_it.h"
        "inc/BlueNRG1_conf.h"
)


# +--------------+
# | STM Cube     |
# +--------------+
add_library ("stm" STATIC
        # Periph
        "../../Library/BlueNRG1_Periph_Driver/inc/BlueNRG1_adc.h"
        "../../Library/BlueNRG1_Periph_Driver/inc/BlueNRG1_dma.h"
        "../../Library/BlueNRG1_Periph_Driver/inc/BlueNRG1_flash.h"
        "../../Library/BlueNRG1_Periph_Driver/inc/BlueNRG1_gpio.h"
        "../../Library/BlueNRG1_Periph_Driver/inc/BlueNRG1_i2c.h"
        "../../Library/BlueNRG1_Periph_Driver/inc/BlueNRG1_mft.h"
        "../../Library/BlueNRG1_Periph_Driver/inc/BlueNRG1_rng.h"
        "../../Library/BlueNRG1_Periph_Driver/inc/BlueNRG1_rtc.h"
        "../../Library/BlueNRG1_Periph_Driver/inc/BlueNRG1_spi.h"
        "../../Library/BlueNRG1_Periph_Driver/inc/BlueNRG1_sysCtrl.h"
        "../../Library/BlueNRG1_Periph_Driver/inc/BlueNRG1_uart.h"
        "../../Library/BlueNRG1_Periph_Driver/inc/BlueNRG1_wdg.h"
        "../../Library/BlueNRG1_Periph_Driver/inc/misc.h"
        "../../Library/BlueNRG1_Periph_Driver/src/BlueNRG1_adc.c"
        "../../Library/BlueNRG1_Periph_Driver/src/BlueNRG1_dma.c"
        "../../Library/BlueNRG1_Periph_Driver/src/BlueNRG1_flash.c"
        "../../Library/BlueNRG1_Periph_Driver/src/BlueNRG1_gpio.c"
        "../../Library/BlueNRG1_Periph_Driver/src/BlueNRG1_i2c.c"
        "../../Library/BlueNRG1_Periph_Driver/src/BlueNRG1_mft.c"
        "../../Library/BlueNRG1_Periph_Driver/src/BlueNRG1_rng.c"
        "../../Library/BlueNRG1_Periph_Driver/src/BlueNRG1_rtc.c"
        "../../Library/BlueNRG1_Periph_Driver/src/BlueNRG1_spi.c"
        "../../Library/BlueNRG1_Periph_Driver/src/BlueNRG1_sysCtrl.c"
        "../../Library/BlueNRG1_Periph_Driver/src/BlueNRG1_uart.c"
        "../../Library/BlueNRG1_Periph_Driver/src/BlueNRG1_wdg.c"
        "../../Library/BlueNRG1_Periph_Driver/src/misc.c"

        # BSP stuff
        "../../Drivers/BSP/STEVAL-BLUEMIC1/steval_bluemic1_accelero.c"
        "../../Drivers/BSP/STEVAL-BLUEMIC1/steval_bluemic1_accelero.h"
        "../../Drivers/BSP/STEVAL-BLUEMIC1/steval_bluemic1_audio_in.c"
        "../../Drivers/BSP/STEVAL-BLUEMIC1/steval_bluemic1_audio_in.h"
        "../../Drivers/BSP/STEVAL-BLUEMIC1/steval_bluemic1_gyro.c"
        "../../Drivers/BSP/STEVAL-BLUEMIC1/steval_bluemic1_gyro.h"
        "../../Drivers/BSP/STEVAL-BLUEMIC1/steval_bluemic1.c"
        "../../Drivers/BSP/STEVAL-BLUEMIC1/steval_bluemic1.h"

        #HAL high level stuff
        "../../Library/hal/src/osal.c"
        "../../Library/hal/src/sleep.c"
        "../../Library/hal/src/context_switch2.s"
        "../../Library/hal/src/clock.c"
        "../../Library/hal/inc/osal.h"
        "../../Library/hal/inc/sleep.h"
        "../../Library/hal/inc/clock.h"

        # Startup code is writen in C for this µC
        "../../Library/CMSIS/Device/ST/BlueNRG1/Source/system_bluenrg1.c"


        # BLE Stack stuff
        "../../Library/Bluetooth_LE/inc/ble_const.h"
        "../../Library/Bluetooth_LE/inc/ble_status.h"
        "../../Library/Bluetooth_LE/inc/bluenrg1_api.h"
        "../../Library/Bluetooth_LE/inc/bluenrg1_events.h"
        "../../Library/Bluetooth_LE/inc/bluenrg1_gap.h"
        "../../Library/Bluetooth_LE/inc/bluenrg1_gatt_server.h"
        "../../Library/Bluetooth_LE/inc/bluenrg1_hal.h"
        "../../Library/Bluetooth_LE/inc/bluenrg1_stack.h"
        "../../Library/Bluetooth_LE/inc/hci_const.h"
        "../../Library/Bluetooth_LE/inc/link_layer.h"
        "../../Library/Bluetooth_LE/inc/sm.h"

        # CMSIS device specific headers
        "../../Library/CMSIS/Device/ST/BlueNRG1/Include/BlueNRG1.h"
#        "../../Library/CMSIS/Device/ST/BlueNRG1/Include/BlueNRG_x_device.h"
        "../../Library/CMSIS/Device/ST/BlueNRG1/Include/system_bluenrg1.h"

        # CMSIS core headers
        "../../Library/CMSIS/Include/arm_common_tables.h"
        "../../Library/CMSIS/Include/arm_math.h"
        "../../Library/CMSIS/Include/core_cm0.h"
        "../../Library/CMSIS/Include/core_cm0plus.h"
        "../../Library/CMSIS/Include/core_cm3.h"
        "../../Library/CMSIS/Include/core_cm4.h"
        "../../Library/CMSIS/Include/core_cm4_simd.h"
        "../../Library/CMSIS/Include/core_cmFunc.h"
        "../../Library/CMSIS/Include/core_cmInstr.h"
        "../../Library/CMSIS/Include/core_sc000.h"
        "../../Library/CMSIS/Include/core_sc300.h"

        #lsm6dsl
        "../../Drivers/BSP/Components/lsm6dsl/LSM6DSL_ACC_GYRO_driver_HL.c"
        "../../Drivers/BSP/Components/lsm6dsl/LSM6DSL_ACC_GYRO_driver_HL.h"
        "../../Drivers/BSP/Components/lsm6dsl/LSM6DSL_ACC_GYRO_driver.c"
        "../../Drivers/BSP/Components/lsm6dsl/LSM6DSL_ACC_GYRO_driver.h"
)

#set_target_properties (${CMAKE_PROJECT_NAME}.elf PROPERTIES COMPILE_FLAGS "-save-temps")



TARGET_LINK_LIBRARIES (${CMAKE_PROJECT_NAME}.elf libbluenrg1_stack_lib_atollic.a)
TARGET_LINK_LIBRARIES (${CMAKE_PROJECT_NAME}.elf libbluevoiceADPCM_BNRG1_100_Atollic.a)
TARGET_LINK_LIBRARIES (${CMAKE_PROJECT_NAME}.elf -Wl,--whole-archive stm -Wl,--no-whole-archive)



add_custom_command(
  TARGET ${PROJ}.elf
  COMMAND "arm-none-eabi-objdump"
  ARGS "-S" "${PROJ}.elf" ">>" "${PROJ}.lst")

  ADD_CUSTOM_TARGET(${CMAKE_PROJECT_NAME}.bin ALL DEPENDS ${CMAKE_PROJECT_NAME}.elf COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin)
