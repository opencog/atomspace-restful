INCLUDE_DIRECTORIES (
	${CMAKE_BINARY_DIR}
	${TBB_INCLUDE_DIR}
)

LINK_DIRECTORIES(
	${TBB_INCLUDE_DIR}
	${TBB_LIBRARY_DIRS}
)

LINK_LIBRARIES(
    ${COGSERVER_LIBRARIES}
    ${ATOMSPACE_LIBRARIES}
    zmq
    tbb
    attention-types
    attention
    atomspacepublishermodule
)

ADD_CXXTEST(AtomSpacePublisherModuleUTest)

TARGET_LINK_LIBRARIES(AtomSpacePublisherModuleUTest
	atomspacepublishermodule
)
