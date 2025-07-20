cmake_minimum_required(VERSION 3.15)

project(Skeleton)

set(CMAKE_CXX_STANDARD 14)

# Set the path to the Adobe After Effects SDK
set(AE_SDK_PATH "./ThirdParty/AdobeSdk/")

# Gather all .cpp files
file(GLOB_RECURSE SOURCE_FILES 
  ${AE_SDK_PATH}/Util/*.cpp
  ./Skeleton_Strings.cpp
  ./Skeleton.cpp
)

# Gather all include directories
set(AESDK_INCLUDE
	${AE_SDK_PATH}/Headers
	${AE_SDK_PATH}/Headers/SP
	${AE_SDK_PATH}/Util
	${AE_SDK_PATH}/Resources
)

include_directories(
  ${AESDK_INCLUDE}
  ./
)

# Specify the plugin target
add_library(${PROJECT_NAME} MODULE ${SOURCE_FILES})

# For Visual Studio project generation
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_definitions(
		-D_CRT_SECURE_NO_WARNINGS
	)

	# Specify target extension
	set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".aex")

	# Get absolute path to PiPLtool
	get_filename_component(
		AFX_REZ
		${AE_SDK_PATH}/Resources/PiPLtool.exe
		ABSOLUTE
		CACHE
	)

	# .r >> .rr
	add_custom_command(
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rr
			COMMAND
			cl 
			   /I \"../${AE_SDK_PATH}/Headers\" 
			   /I \"../${AE_SDK_PATH}/Headers/SP\" 
			   /I \"../${AE_SDK_PATH}/Util\" 
			   /I \"../${AE_SDK_PATH}/Resources\" 
			   /EP \"${CMAKE_CURRENT_SOURCE_DIR}/SkeletonPiPL.r\" > ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rr
	)

	# .rr -> .rrc
	add_custom_command(
		DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rr
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rrc
		COMMAND ${AFX_REZ} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rr ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rrc
	)

	# .rrc -> .rc (for Windows)
	add_custom_command(
		DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rrc
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc
		COMMAND cl /D "MSWindows" /EP ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rrc > ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc
	)

	# include .rc file in sources
	target_sources(
		${PROJECT_NAME} PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc
	)

	target_link_libraries(Skeleton PRIVATE D3DCompiler)

endif()