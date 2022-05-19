
# The atom_types.h file is written to the build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

# DECLARE_GUILE_CONFIG_TARGET(SCM_CONFIG "opencog restul-config" "OPENCOG_TEST")

ADD_SUBDIRECTORY (python)

#IF (HAVE_EVENT_PUBLISHING_DEPENDENCIES)
#	ADD_SUBDIRECTORY (events)
#ENDIF (HAVE_EVENT_PUBLISHING_DEPENDENCIES)

# WRITE_GUILE_CONFIG(${GUILE_BIN_DIR}/opencog/restul-config.scm SCM_CONFIG TRUE)
#
# WRITE_GUILE_CONFIG(${GUILE_BIN_DIR}/opencog/restul-config-installable.scm SCM_CONFIG FALSE)
# INSTALL(FILES ${GUILE_BIN_DIR}/opencog/restul-config-installable.scm DESTINATION ${GUILE_SITE_DIR}/opencog RENAME restul-config.scm)
