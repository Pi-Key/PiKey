cmake_minimum_required(VERSION 3.5)

if (DEFINED ENV{BOARD})
	if ( $ENV{BOARD} STREQUAL "picow" )
		set(PICO_BOARD pico_w)
	else()
		set(PICO_BOARD pico)
	endif()
else()
	set(PICO_BOARD pico_w)
endif()

set(ENV{PICO_SDK_PATH} "${CMAKE_SOURCE_DIR}/pico-sdk")

include(pico_sdk_import.cmake)

file(GLOB PROJECT_SOURCES "src/*.c")
add_executable(pikey ${PROJECT_SOURCES})
project(pikey C CXX ASM)
pico_sdk_init()

# if ( PICO_CYW43_SUPPORTED )
# 	target_link_libraries(pikey
# 		pico_stdlib
# 		pico_multicore
# 		pico_cyw43_arch_none
# 		pico_unique_id
#
# 		tinyusb_device
# 		tinyusb_board
# 		)
# else()
# 	target_link_libraries(pikey
# 		pico_stdlib
# 		pico_multicore
#
# 		tinyusb_device
# 		tinyusb_board
# 		)
# endif()
#
# target_include_directories(pikey PRIVATE
# 	$ENV{PICO_SDK_PATH}/lib/tinyusb/hw
# 	$ENV{PICO_SDK_PATH}/lib/tinyusb/src
# 	)

target_link_libraries(pikey
	pico_stdlib
	pico_multicore
	pico_cyw43_arch_none
	
	tinyusb_device
	tinyusb_board
	)

target_include_directories(pikey PRIVATE
	src
	$ENV{PICO_SDK_PATH}/lib/tinyusb/hw
	$ENV{PICO_SDK_PATH}/lib/tinyusb/src
	)

pico_add_extra_outputs(pikey)

pico_enable_stdio_usb(pikey 1)
pico_enable_stdio_uart(pikey 0)
