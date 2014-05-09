cmake_minimum_required(VERSION 2.8.9)

project(QuickModGenerator)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(APPLE)
	message(STATUS "Using APPLE CMAKE_CXX_FLAGS")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
elseif(UNIX)
	# assume GCC, add C++0x/C++11 stuff
	message(STATUS "Using UNIX CMAKE_CXX_FLAGS")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
elseif(MINGW)
	message(STATUS "Using MINGW CMAKE_CXX_FLAGS")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall")
endif()

find_package(Qt5 COMPONENTS Core Network Xml Gui Widgets REQUIRED)

include_directories(SYSTEM /usr/include/threadweaver)

set(SOURCES
 main.cpp

 QuickMod.h
 QuickModWriter.h
 QuickModWriter.cpp
 QuickModReader.h
 QuickModReader.cpp

 MainWidget.h
 MainWidget.cpp
 FixupWidget.h
 FixupWidget.cpp
 GraphWidget.h
 GraphWidget.cpp

 commands/DumpCommand.h
 commands/DumpCommand.cpp
 commands/FixupCommand.h
 commands/FixupCommand.cpp
 commands/FormatCommand.h
 commands/FormatCommand.cpp
 commands/GraphCommand.h
 commands/GraphCommand.cpp
 commands/IndexCommand.h
 commands/IndexCommand.cpp
 commands/SetupCommand.h
 commands/SetupCommand.cpp
 commands/VerifyCommand.h
 commands/VerifyCommand.cpp
 commands/UpdateCommand.h
 commands/UpdateCommand.cpp
 commands/GuiCommand.h
 commands/GuiCommand.cpp
 commands/CreateChecksumCommand.h
 commands/CreateChecksumCommand.cpp

 commands/AbstractCommand.h
 commands/AbstractCommand.cpp

 Util.h
 Util.cpp

# ThreadWeaver stuff
 threadweaver/src/queuesignals.h
 threadweaver/src/queuesignals.cpp
 threadweaver/src/queuestream.h
 threadweaver/src/queuestream.cpp
 threadweaver/src/queueapi.h
 threadweaver/src/queueapi.cpp
 threadweaver/src/exception.h
 threadweaver/src/exception.cpp
 threadweaver/src/queue.h
 threadweaver/src/queue.cpp
 threadweaver/src/weaver.h
 threadweaver/src/weaver.cpp
 threadweaver/src/debuggingaids.h
 threadweaver/src/debuggingaids.cpp
 threadweaver/src/thread.h
 threadweaver/src/thread.cpp
 threadweaver/src/job.h
 threadweaver/src/job.cpp
 threadweaver/src/iddecorator.h
 threadweaver/src/iddecorator.cpp
 threadweaver/src/qobjectdecorator.h
 threadweaver/src/qobjectdecorator.cpp
 threadweaver/src/executor_p.h
 threadweaver/src/executor.cpp
 threadweaver/src/executewrapper_p.h
 threadweaver/src/executewrapper.cpp
 threadweaver/src/state.h
 threadweaver/src/state.cpp
 threadweaver/src/weaverimplstate.h
 threadweaver/src/weaverimplstate.cpp
 threadweaver/src/inconstructionstate.h
 threadweaver/src/inconstructionstate.cpp
 threadweaver/src/workinghardstate.h
 threadweaver/src/workinghardstate.cpp
 threadweaver/src/suspendingstate.h
 threadweaver/src/suspendingstate.cpp
 threadweaver/src/suspendedstate.h
 threadweaver/src/suspendedstate.cpp
 threadweaver/src/shuttingdownstate.h
 threadweaver/src/shuttingdownstate.cpp
 threadweaver/src/destructedstate.h
 threadweaver/src/destructedstate.cpp
 threadweaver/src/collection.h
 threadweaver/src/collection.cpp
 threadweaver/src/sequence.h
 threadweaver/src/sequence.cpp
 threadweaver/src/dependencypolicy.h
 threadweaver/src/dependencypolicy.cpp
 threadweaver/src/dependency.h
 threadweaver/src/dependency.cpp
 threadweaver/src/resourcerestrictionpolicy.h
 threadweaver/src/resourcerestrictionpolicy.cpp

 ../MultiMC5/depends/util/src/modutils.cpp
)

qt5_wrap_ui(UIS MainWidget.ui FixupWidget.ui GraphWidget.ui)

include_directories(threadweaver/src commands ../MultiMC5/depends/util)

add_executable(QuickModGenerator ${SOURCES} ${UIS})
qt5_use_modules(QuickModGenerator Core Network Xml Gui Widgets)

set(MODPROBE_DIR "${CMAKE_SOURCE_DIR}/ModProbe")
set(MODPROBE_VERSION "1.0")
find_program(GRADLE gradle PATHS /home/jan/src/gradle-1.11/bin)
add_custom_target(build_ModProbe
 COMMAND ${GRADLE} shadow copyLibs
 WORKING_DIRECTORY ${MODPROBE_DIR}
)
add_custom_target(copy_ModProbe
 COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MODPROBE_DIR}/build/libs/ModProbe-${MODPROBE_VERSION}-shadow.jar" "${CMAKE_BINARY_DIR}/ModProbe.jar"
 COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MODPROBE_DIR}/build/libs/fml-1.7.2-7.2.18.797-universal.jar" "${CMAKE_BINARY_DIR}/fml.jar"
 DEPENDS build_ModProbe
)
add_custom_target(ModProbe
 DEPENDS copy_ModProbe
)
