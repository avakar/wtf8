cmake_minimum_required(VERSION 3.15)
project(avakar_wtf8 VERSION 1.0.0)

add_library(avakar_wtf8
	include/avakar/wtf8.h
	src/wtf8.cpp
	)
add_library(avakar::wtf8 ALIAS avakar_wtf8)
target_include_directories(avakar_wtf8 PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
	)
target_compile_features(avakar_wtf8 PUBLIC cxx_std_17)

get_property(cxx_features GLOBAL PROPERTY CMAKE_CXX_KNOWN_FEATURES)
if (cxx_std_20 IN_LIST cxx_features)
	target_compile_features(avakar_wtf8 PUBLIC cxx_std_20)
endif()

install(TARGETS avakar_wtf8 EXPORT "${PROJECT_NAME}-targets")
install(DIRECTORY include DESTINATION include)
install(EXPORT "${PROJECT_NAME}-targets"
	DESTINATION "cmake/${PROJECT_NAME}"
	NAMESPACE avakar::
	FILE "${PROJECT_NAME}-targets.cmake")

include(CTest)
if ("${PROJECT_NAME}" STREQUAL "${CMAKE_PROJECT_NAME}" AND BUILD_TESTING)
	include(FetchContent)
	FetchContent_Declare(
		Catch2
		GIT_REPOSITORY https://github.com/catchorg/Catch2.git
		GIT_TAG d10b9bd02e098476670f5eb0527d2c7281476e8a
		GIT_SHALLOW YES
		)
	FetchContent_MakeAvailable(Catch2)
	include("${catch2_SOURCE_DIR}/contrib/Catch.cmake")

	add_executable(avakar_wtf8_test
		test/test.cpp
		test/test1.cpp
		)
	target_compile_features(avakar_wtf8_test PUBLIC cxx_std_20)
	target_link_libraries(avakar_wtf8_test PUBLIC avakar::wtf8 Catch2::Catch2)

	catch_discover_tests(avakar_wtf8_test)
endif()
