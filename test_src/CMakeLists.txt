CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(tapitest-legacy C)

SET(tapitest-legacy
	main.c
	menu.c
	network.c
	sat.c
	sim.c
	sms.c
	sms_util.c
	phonebook.c
	modem.c
	ss.c
	call.c
	gps.c
)

ADD_EXECUTABLE(tapitest-legacy ${tapitest-legacy})
TARGET_LINK_LIBRARIES(tapitest-legacy ${pkgs_LDFLAGS} "-L${CMAKE_BINARY_DIR} -ltapi")
INSTALL(TARGETS tapitest-legacy RUNTIME DESTINATION bin/)
ADD_DEPENDENCIES(tapitest-legacy tapi)
