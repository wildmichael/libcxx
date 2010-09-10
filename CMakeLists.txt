cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(libcxx C CXX)
set(LIBCXX_VERSION 1)

# sources
set(SRCS
	src/algorithm.cpp src/bind.cpp src/chrono.cpp src/condition_variable.cpp
	src/exception.cpp src/future.cpp src/hash.cpp src/ios.cpp src/iostream.cpp
	src/locale.cpp src/memory.cpp src/mutex.cpp src/new.cpp src/random.cpp
	src/regex.cpp src/stdexcept.cpp src/string.cpp src/strstream.cpp
	src/system_error.cpp src/thread.cpp src/typeinfo.cpp src/utility.cpp
	src/valarray.cpp
	)

# headers
set(HDRS
	include/__bit_reference include/__config include/__functional_03
	include/__functional_base include/__functional_base_03 include/__hash_table
	include/__locale include/__mutex_base include/__split_buffer
	include/__sso_allocator include/__std_stream include/__tree include/__tuple
	include/__tuple_03 include/algorithm include/array include/bitset
	include/cassert include/ccomplex include/cctype include/cerrno include/cfenv
	include/cfloat include/chrono include/cinttypes include/ciso646
	include/climits include/clocale include/cmath include/codecvt include/complex
	include/complex.h include/condition_variable include/csetjmp include/csignal
	include/cstdarg include/cstdbool include/cstddef include/cstdint
	include/cstdio include/cstdlib include/cstring include/ctgmath include/ctime
	include/cwchar include/cwctype include/deque include/exception
	include/ext/hash_map include/ext/hash_set include/forward_list
	include/fstream include/functional include/future include/initializer_list
	include/iomanip include/ios include/iosfwd include/iostream include/istream
	include/iterator include/limits include/list include/locale include/map
	include/memory include/mutex include/new include/numeric include/ostream
	include/queue include/random include/ratio include/regex
	include/scoped_allocator include/set include/sstream include/stack
	include/stdexcept include/streambuf include/string include/strstream
	include/system_error include/tgmath.h include/thread include/tuple
	include/type_traits include/typeindex include/typeinfo include/unordered_map
	include/unordered_set include/utility include/valarray include/vector
	)
set_source_files_properties(${HDRS} PROPERTIES
	HEADER_FILE_ONLY TRUE
	LANGUAGE CXX
	)

# installation directories
set(CMAKE_INSTALL_LIBDIR lib CACHE STRING
	"Installation directory for libraries")
set(CMAKE_INSTALL_INCLUDEDIR include/c++/v${LIBCXX_VERSION} CACHE STRING
	"Installation directory for headers")
if(WIN32)
	set(default_cmakedir cmake)
else()
	set(default_cmakedir share/cmake)
endif()
set(CMAKE_INSTALL_CMAKEDIR ${default_cmakedir} CACHE STRING
	"Installation directory for CMake files")
mark_as_advanced(CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_INCLUDEDIR
	CMAKE_INSTALL_CMAKEDIR)
foreach(d LIB INCLUDE CMAKE)
	if(NOT IS_ABSOLUTE ${CMAKE_INSTALL_${d}DIR})
		set(CMAKE_INSTALL_${d}DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_${d}DIR})
	endif()
endforeach()

if(APPLE)

	# build or find libc++-abi
	option(BUILD_CPPABI "Build private copy of libppabi" FALSE)
	if(BUILD_CPPABI)
		include(ExternalProject)
		if(EXISTS ${CMAKE_BINARY_DIR}/libcppabi.zip)
			set(cppabi_url ${CMAKE_BINARY_DIR}/libcppabi.zip)
		elseif(EXISTS ${CMAKE_SOURCE_DIR}/libcppabi.zip)
			set(cppabi_url ${CMAKE_SOURCE_DIR}/libcppabi.zip)
		else()
			set(cppabi_url http://home.roadrunner.com/~hinnant/libcppabi.zip)
		endif()
		set(CPPABI_INCLUDE_DIR
			${CMAKE_BINARY_DIR}/external/Install/cppabi/usr/include)
		set(CPPABI_LIBRARY
			${CMAKE_BINARY_DIR}/external/Install/cppabi/usr/lib/libc++abi.dylib)
		set_directory_properties(PROPERTIES EP_BASE ${CMAKE_BINARY_DIR}/external)
		ExternalProject_Add(cppabi
			URL ${cppabi_url}
			URL_MD5 d214d7798484ec611ae5ece5bab79de5
			# the project tries to install files that don't exist
			PATCH_COMMAND /usr/bin/patch -d libcppabi -p1 <
				${CMAKE_SOURCE_DIR}/libcppabi.patch
			CONFIGURE_COMMAND ""
			BUILD_COMMAND cd libcppabi && xcodebuild -configuration Release
			BUILD_IN_SOURCE 1
			INSTALL_COMMAND cd libcppabi &&
				xcodebuild -configuration Release install DSTROOT=<INSTALL_DIR> &&
				# fixup the install_name
				install_name_tool -id ${CPPABI_LIBRARY} ${CPPABI_LIBRARY}
			)
		unset(CPPABI_INCLUDE_DIR CACHE)
		unset(CPPABI_LIBRARY CACHE)
	else()
		find_path(CPPABI_INCLUDE_DIR cxxabi.h)
		find_library(CPPABI_LIBRARY c++abi)
		mark_as_advanced(CPPABI_INCLUDE_DIR CPPABI_LIBRARY)
		if(NOT CPPABI_INCLUDE_DIR OR NOT CPPABI_LIBRARY)
			message(FATAL_ERROR
				"Failed to find libcppabi. "
				"If you want to build libcppabi, set BUILD_CPPABI to TRUE")
		endif()
	endif()
	include_directories(${CPPABI_INCLUDE_DIR})

	set(CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_LIBDIR})
	set(outname
		${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/libc++.${LIBCXX_VERSION}.dylib)
	# compile flags
	set(compile_flags -nostdinc++)
	# link libraries
	set(libs /usr/lib/libSystem.dylib)
	# link flags
	set(link_flags -nodefaultlibs)
	set(extra_link_flags "-Wl,-reexport_library,${CPPABI_LIBRARY}")
	# post-processing commands
	set(postproc_commands
		COMMAND /usr/bin/dsymutil -o ${outname}.dSYM ${outname}
		COMMAND ${CMAKE_STRIP} -S ${outname}
		)
	if(CMAKE_GENERATOR STREQUAL Xcode)
		# Xcode generator doesn't libraries with a so-name namelinks
		list(APPEND postproc_commands
			COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CFG_INTDIR}/libc++.dylib
			COMMAND ${CMAKE_COMMAND} -E create_symlink
				libc++.${LIBCXX_VERSION}.dylib
				${CMAKE_CFG_INTDIR}/libc++.dylib
			)
	endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)

	message(WARNING
		"libc++ is useless on Linux (locales not implemented)")

	set(CMAKE_THREAD_PREFER_PTHREADS TRUE)
	find_package(Threads REQUIRED)
	if(NOT CMAKE_USE_PTHREADS_INIT AND NOT CMAKE_HP_PTHREADS_INIT)
		message(FATAL_ERROR "Require pthreads")
	endif()

	set(outname
		${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/libc++.so.${LIBCXX_VERSION})
	# compile flags
	set(compile_flags -nostdinc++)
	# link libraries
	set(libs stdc++ m c ${CMAKE_THREAD_LIBS_INIT})
	# link flags
	set(link_flags -nodefaultlibs)
	set(extra_link_flags)

	# check whether clock_settime needs linking against librt
	include(CheckFunctionExists)
	set(CMAKE_REQUIRED_FLAGS "${compile_flags} ${link_flags}")
	set(CMAKE_REQUIRED_LIBRARIES ${libs})
	check_function_exists(clock_gettime HAVE_CLOCK_GETTIME_WITHOUT_LIBRT)
	if(NOT HAVE_CLOCK_GETTIME_WITHOUT_LIBRT)
		list(APPEND CMAKE_REQUIRED_LIBRARIES rt)
		check_function_exists(clock_gettime HAVE_CLOCK_GETTIME_WITH_LIBRT)
		if(HAVE_CLOCK_GETTIME_WITH_LIBRT)
			list(APPEND libs rt)
		else()
			message(SEND_ERROR
				"Required function clock_gettime not available")
		endif()
	endif()

	# post-processing commands
	set(postproc_commands
		COMMAND ${CMAKE_OBJCOPY} --only-keep-debug ${outname} ${outname}.debug
		COMMAND ${CMAKE_STRIP} -S ${outname}
		COMMAND ${CMAKE_OBJCOPY}
		  --add-gnu-debuglink=libc++.so.${LIBCXX_VERSION}.debug ${outname}
		)

else()
	message(FATAL_ERROR "Ooops. Not implemented")
endif()

# the library
add_library(c++ SHARED ${SRCS} ${HDRS})
target_link_libraries(c++ ${libs})
set_target_properties(c++ PROPERTIES
	COMPILE_FLAGS "${compile_flags}"
	LINK_FLAGS "${link_flags} ${extra_link_flags}"
	VERSION ${LIBCXX_VERSION}
	)
if(APPLE AND BUILD_CPPABI)
	add_dependencies(c++ cppabi)
endif()

include_directories(BEFORE SYSTEM include)

# post-process
add_custom_command(TARGET c++ POST_BUILD
	${postproc_commands}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Extracting debugging symbols"
	VERBATIM
	)

# install the stuff
install(TARGETS c++ EXPORT libcxx-exports
	RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

install(DIRECTORY include/  # the trailing / is important!
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	PATTERN .svn EXCLUDE
	)

if(CMAKE_GENERATOR STREQUAL Xcode)
	# Xcode generator doesn't libraries with a so-name namelinks
	set_target_properties(c++ PROPERTIES
		OUTPUT_NAME c++.${LIBCXX_VERSION}
		)
	install(CODE
"execute_process(
	COMMAND ${CMAKE_COMMAND} -E create_symlink
		libc++.${LIBCXX_VERSION}.dylib
		${CMAKE_INSTALL_LIBDIR}/libc++.dylib
		)
")
endif()

if(APPLE AND BUILD_CPPABI)
	# install cxxabi.h and libc++abi.dylib
	install(FILES ${CPPABI_INCLUDE_DIR}/cxxabi.h
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
	install(PROGRAMS ${CPPABI_LIBRARY}
		DESTINATION ${CMAKE_INSTALL_LIBDIR})
	# fixup install names
	install(CODE
"execute_process(
	COMMAND install_name_tool -id
		${CMAKE_INSTALL_LIBDIR}/libc++abi.dylib
		${CMAKE_INSTALL_LIBDIR}/libc++abi.dylib
		)
execute_process(
	COMMAND install_name_tool -change
		${CPPABI_LIBRARY}
		${CMAKE_INSTALL_LIBDIR}/libc++abi.dylib
		${CMAKE_INSTALL_LIBDIR}/libc++.dylib
		)
")
endif()

# export targets to build tree
export(TARGETS c++ NAMESPACE libcxx_
	FILE ${CMAKE_BINARY_DIR}/libcxx-exports.cmake)
export(PACKAGE libcxx)
set(config_includedir ${CMAKE_SOURCE_DIR}/include)
set(config_cmakedir ${CMAKE_BINARY_DIR})
configure_file(libcxx-config.cmake.in
	${CMAKE_BINARY_DIR}/libcxx-config.cmake @ONLY)
configure_file(libcxx-config-version.cmake.in
	${CMAKE_BINARY_DIR}/libcxx-config-version.cmake @ONLY)

# export targets for install tree
set(config_includedir ${CMAKE_INSTALL_INCLUDEDIR})
set(config_cmakedir ${CMAKE_INSTALL_CMAKEDIR})
configure_file(libcxx-config.cmake.in
	${CMAKE_BINARY_DIR}/install_files/libcxx-config.cmake @ONLY)
configure_file(libcxx-config-version.cmake.in
	${CMAKE_BINARY_DIR}/install_files/libcxx-config-version.cmake @ONLY)
install(EXPORT libcxx-exports NAMESPACE libcxx_
	DESTINATION ${CMAKE_INSTALL_CMAKEDIR})
install(FILES
	${CMAKE_BINARY_DIR}/install_files/libcxx-config.cmake
	${CMAKE_BINARY_DIR}/install_files/libcxx-config-version.cmake
	DESTINATION ${CMAKE_INSTALL_CMAKEDIR})

# add tests (disble by default)
option(BUILD_TESTING "Build the testing tree" OFF)
include(CTest)
if(BUILD_TESTING)
	add_subdirectory(test)
endif()
