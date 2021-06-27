#
# Master Opencog CMake file.
#
# General organization:
# -- check for different compilers, OS'es
# -- search for various required & optional libraries/tools
# -- decide what to build based on above results.
# -- configure various config files.
# -- print pretty summary
#

# cogutils already requires 2.8.12.2, so may as well ask for that.
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12.2)
IF (COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF (COMMAND CMAKE_POLICY)

IF(CMAKE_VERSION VERSION_GREATER 3.0.2)
	CMAKE_POLICY(SET CMP0037 OLD)
ENDIF(CMAKE_VERSION VERSION_GREATER 3.0.2)

PROJECT(opencog)

# FIND_PACKAGE(Boost 1.67 REQUIRED date_time program_options thread filesystem system unit_test_framework)
IF(${Boost_FOUND})
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ENDIF()
# ----------------------------------------------------------
# User-modifiable options. Feel free to change these!
#
# uncomment to be in Release mode [default]
# SET(CMAKE_BUILD_TYPE Release)

# uncomment to build in debug mode
# SET(CMAKE_BUILD_TYPE Debug)

# uncomment to be in coverage testing mode
# SET(CMAKE_BUILD_TYPE Coverage)

# uncomment to build in profile mode
# SET(CMAKE_BUILD_TYPE Profile)

# uncomment to build in release mode with debug information
# SET(CMAKE_BUILD_TYPE RelWithDebInfo)

# default build type
IF (CMAKE_BUILD_TYPE STREQUAL "")
	SET(CMAKE_BUILD_TYPE Release)
ENDIF (CMAKE_BUILD_TYPE STREQUAL "")

MESSAGE(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

ADD_DEFINITIONS(-DPROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
                -DPROJECT_BINARY_DIR="${CMAKE_BINARY_DIR}")

# ===============================================================
# Check for existance of various required, optional packages.
# Listed in alphabetical order, more or less.
# CogUtil must come first, because it supplies various FindXXX macros.

# Add the 'lib' dir to cmake's module search path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/lib/")

# Cogutil
FIND_PACKAGE(CogUtil 2.0.1 CONFIG REQUIRED)
IF (COGUTIL_FOUND)
    MESSAGE(STATUS "CogUtil found.")
    ADD_DEFINITIONS(-DHAVE_COGUTIL)
    SET(HAVE_COGUTIL 1)
ELSE (COGUTIL_FOUND)
    MESSAGE(FATAL_ERROR "CogUtil missing: it is needed!")
ENDIF (COGUTIL_FOUND)

# add the 'cmake' directory from cogutil to search path
list(APPEND CMAKE_MODULE_PATH  ${COGUTIL_DATA_DIR}/cmake)

include(${COGUTIL_DATA_DIR}/cmake/OpenCogGccOptions.cmake)
include(${COGUTIL_DATA_DIR}/cmake/OpenCogLibOptions.cmake)
include(${COGUTIL_DATA_DIR}/cmake/OpenCogInstallOptions.cmake)
include(${COGUTIL_DATA_DIR}/cmake/Summary.cmake)

# ===================================================================
# Check for existance of various required, optional packages.

# AtomSpace
FIND_PACKAGE(AtomSpace 5.0.3 CONFIG REQUIRED)
IF (ATOMSPACE_FOUND)
	MESSAGE(STATUS "AtomSpace found.")
	ADD_DEFINITIONS(-DHAVE_ATOMSPACE)
	SET(HAVE_ATOMSPACE 1)
ELSE (ATOMSPACE_FOUND)
	MESSAGE(FATAL_ERROR "AtomSpace missing: it is needed!")
ENDIF (ATOMSPACE_FOUND)

# CogServer
FIND_PACKAGE(CogServer 1.0.0 CONFIG)
IF (COGSERVER_FOUND)
	MESSAGE(STATUS "CogServer found.")
	ADD_DEFINITIONS(-DHAVE_SERVER)
	SET(HAVE_SERVER 1)
ELSE (COGSERVER_FOUND)
	MESSAGE(STATUS "CogServer was not found.")
ENDIF (COGSERVER_FOUND)

# ----------------------------------------------------------
# Needed for unit tests

FIND_PACKAGE(Cxxtest)
IF (NOT CXXTEST_FOUND)
	MESSAGE(STATUS "CxxTest missing: needed for unit tests.")
ENDIF (NOT CXXTEST_FOUND)

# ----------------------------------------------------------
# This is required for Guile Python and Cython

include(OpenCogFindGuile)
include(OpenCogFindPython)

# ----------------------------------------------------------
# jsoncpp
FIND_PACKAGE(PkgConfig)
IF(PKG_CONFIG_FOUND)
  pkg_check_modules(JSONCPP jsoncpp)
  IF(JSONCPP_FOUND)
    ADD_DEFINITIONS(-DHAVE_JSONCPP)
    SET(HAVE_JSONCPP 1)
    SET(JSONCPP_DIR_MESSAGE "JsonCPP was found")
  ELSE(JSONCPP_FOUND)
    SET(JSONCPP_DIR_MESSAGE "JsonCPP was not found; the AtomSpace Publisher Module needs JsonCPP to publish events in JSON.\nIf you need to receive AtomSpace events in JSON, install JsonCPP using your package manager \nsuch as apt or build from source: https://github.com/open-source-parsers/jsoncpp/wiki/Building")
  ENDIF(JSONCPP_FOUND)
ENDIF(PKG_CONFIG_FOUND)

# Threaded Building Blocks (Intel TBB) library
FIND_PACKAGE(TBB)
IF (TBB_FOUND)
   SET(HAVE_TBB 1)
ELSE (TBB_FOUND)
   SET(TBB_DIR_MESSAGE "Intel TBB (Threaded Building Blocks) was not found; the AtomSpace Publisher module will not be built. Installation instructions: https://www.threadingbuildingblocks.org/download")
ENDIF (TBB_FOUND)
MESSAGE(STATUS "${TBB_DIR_MESSAGE}")

# ZeroMQ
FIND_PACKAGE(ZMQ 3.2.4)
IF (ZMQ_FOUND AND ZMQ_LIBRARY)
   ADD_DEFINITIONS(-DHAVE_ZMQ)
   SET(HAVE_ZMQ 1)
   SET(ZMQ_DIR_MESSAGE "ZeroMQ was found.")
ELSE (ZMQ_FOUND AND ZMQ_LIBRARY)
   SET(ZMQ_DIR_MESSAGE "ZeroMQ was not found; the AtomSpace Publisher module, and the message system used by GUI monitor for OAC will not be built.\nTo over-ride, make sure that the environment variable ZMQ_LIBRARY is set.\nInstallation instructions: http://zeromq.org/intro:get-the-software")
ENDIF (ZMQ_FOUND AND ZMQ_LIBRARY)
MESSAGE(STATUS "${ZMQ_DIR_MESSAGE}")

IF (HAVE_JSONCPP AND HAVE_TBB AND HAVE_ZMQ)
	SET(HAVE_EVENT_PUBLISHING_DEPENDENCIES 1)
ENDIF (HAVE_JSONCPP AND HAVE_TBB AND HAVE_ZMQ)

# ===================================================================
# Include configuration.

# AttentionBank
FIND_PACKAGE(AttentionBank 1.0.0 CONFIG)
IF (ATTENTIONBANK_FOUND)
	MESSAGE(STATUS "AttentionBank found.")
	ADD_DEFINITIONS(-DHAVE_BANK)
	SET(HAVE_BANK 1)
ELSE (ATTENTIONBANK_FOUND)
	MESSAGE(STATUS "AttentionBank was not found. OpenPsi and Ghost will not be built.")
ENDIF (ATTENTIONBANK_FOUND)

# Set default include paths.
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
	${COGUTIL_INCLUDE_DIR} ${ATOMSPACE_INCLUDE_DIR})

# Macros that define how atom types get declared.
IF (NOT DEFINED ATOMSPACE_DATA_DIR)
	SET (ATOMSPACE_DATA_DIR "${COGUTIL_DATA_DIR}")
ENDIF (NOT DEFINED ATOMSPACE_DATA_DIR)

INCLUDE("${ATOMSPACE_DATA_DIR}/cmake/OpenCogMacros.cmake")
INCLUDE("${ATOMSPACE_DATA_DIR}/cmake/OpenCogGuile.cmake")
INCLUDE("${ATOMSPACE_DATA_DIR}/cmake/OpenCogCython.cmake")

# ==========================================================
# Decide what to build, based on the packages found.

ADD_SUBDIRECTORY(opencog)

IF (CXXTEST_FOUND)
	ADD_CUSTOM_TARGET(tests)
	ADD_SUBDIRECTORY(tests EXCLUDE_FROM_ALL)
	IF (CMAKE_BUILD_TYPE STREQUAL "Coverage")
		# doing coverage stuff while running tests if this is the Coverage build
		ADD_CUSTOM_TARGET(test
			# TODO lcov should be found by cmake first
			# TODO set it up so that we can pick to run coverage per test, or
			# combined across all tests (the latter is MUCH faster). Use a define?
			# There is coverage specific stuff in AddCxxTest.cmake now...
			# -
			WORKING_DIRECTORY tests
			COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure $(ARGS)

			# This script combines the coverage analysis of each test,
			# then creates html in tests/lcov
			# Note: this should now be run separately...
			#COMMAND ${PROJECT_SOURCE_DIR}/scripts/combine_lcov.sh
			COMMENT "Running tests with coverage..."
		)
	ELSE (CMAKE_BUILD_TYPE STREQUAL "Coverage")
		# If this is a build with coverage enabled then test normally
		ADD_CUSTOM_TARGET(test
			DEPENDS tests
			WORKING_DIRECTORY tests
			COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure $(ARGS)
			COMMENT "Running tests..."
		)
	ENDIF (CMAKE_BUILD_TYPE STREQUAL "Coverage")
ENDIF (CXXTEST_FOUND)

ADD_SUBDIRECTORY(examples EXCLUDE_FROM_ALL)

IF (NOT WIN32)
	ADD_CUSTOM_TARGET (examples
		# using CMAKE_BUILD_TOOL results in teh cryptic error message
		# warning: jobserver unavailable: using -j1.  Add `+' to parent make rule.
		# This is because make doesn't know how to pass jobserver args to
		# the submake.  So, instead, just use $(MAKE) (with round parens)
		# -- that will do the right thing.
		# COMMAND ${CMAKE_BUILD_TOOL}
		COMMAND $(MAKE)
		WORKING_DIRECTORY examples
		COMMENT "Building examples"
	)
ENDIF (NOT WIN32)

ADD_CUSTOM_TARGET(cscope
	COMMAND find opencog examples tests -name '*.cc' -o -name '*.h' -o -name '*.cxxtest' -o -name '*.scm' > ${CMAKE_SOURCE_DIR}/cscope.files
	COMMAND cscope -b
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	COMMENT "Generating CScope database"
)

IF (NOT WIN32)
	ADD_CUSTOM_TARGET (experiments
		# using CMAKE_BUILD_TOOL results in teh cryptic error message
		# warning: jobserver unavailable: using -j1.  Add `+' to parent make rule.
		# This is because make doesn't know how to pass jobserver args to
		# the submake.  So, instead, just use $(MAKE) (with round parens)
		# -- that will do the right thing.
		# COMMAND ${CMAKE_BUILD_TOOL}
		COMMAND $(MAKE)
		WORKING_DIRECTORY experiments
		COMMENT "Building experiments"
	)
ENDIF (NOT WIN32)

# ===================================================================
# Packaging
## Architecture the package is for.
## TODO: Will give error on non debian distros, fix it.
EXECUTE_PROCESS(COMMAND  dpkg --print-architecture
	OUTPUT_VARIABLE PACKAGE_ARCHITECTURE
	OUTPUT_STRIP_TRAILING_WHITESPACE)
STRING(TIMESTAMP UTC_DATE %Y%m%d UTC)
# If 'sudo make install' is run before 'make package', then install_manifest.txt
# will be owned by root. Creating the file during configuration stage ensures
# that it is owned by the builder thus avoiding 'Permission denied' error when
# packaging.
FILE(WRITE "${PROJECT_BINARY_DIR}/install_manifest.txt")
## It doesn't have a header-file declaring the version similar to cogutil and
## atomspace.
SET(SEMANTIC_VERSION 0.1.4)

## Cpack configuration
SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_CONTACT "opencog@googlegroups.com")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/packages")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The AtomSpace RESTful Web API")
SET(CPACK_PACKAGE_NAME "atomspace-restful")
SET(CPACK_PACKAGE_VENDOR "opencog.org")
SET(CPACK_PACKAGE_VERSION "${SEMANTIC_VERSION}-${UTC_DATE}")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
SET(CPACK_PACKAGE_FILE_NAME
	"${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${PACKAGE_ARCHITECTURE}")
SET(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")

## Debian specific configurations
SET(DEPENDENCY_LIST
	"guile-2.2-dev (>= 2.2.2)"
	"python3-dev (>= 3.6.7)"
	"libstdc++6 (>= 4.7)"
	"libcogutil-dev (>= 2.0.2)"
	"atomspace-dev (>= 5.0.3)"
)

STRING(REPLACE ";" ", " MAIN_DEPENDENCIES "${DEPENDENCY_LIST}")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${MAIN_DEPENDENCIES}")
SET(CPACK_DEBIAN_PACKAGE_SECTION "libdevel")
SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://opencog.org")
INCLUDE(CPack)

# ===================================================================
# documentation
FIND_PACKAGE(Doxygen)
# ADD_SUBDIRECTORY(doc EXCLUDE_FROM_ALL)

# ===================================================================
# Show a summary of what we found, what we will do.

SUMMARY_ADD("Doxygen" "Code documentation" DOXYGEN_FOUND)
SUMMARY_ADD("Python tests" "Python bindings nose tests" HAVE_NOSETESTS)
SUMMARY_ADD("Unit tests" "Unit tests" CXXTEST_FOUND)
SUMMARY_ADD("REST Events" "REST Atomspace Event Publisher module"
	HAVE_EVENT_PUBLISHING_DEPENDENCIES)

SUMMARY_SHOW()
