# CMakeList.txt : CMake project for Chat_App, include source and define
# project specific logic here.
#

# with file
# The asterisk (*) is a wildcard character that matches any sequence of characters (except directory separators)
#file(GLOB_RECURSE SOURCES "*.cpp")
#file(GLOB_RECURSE HEADERS "includes/*.h")

#add_executable(Server ${SOURCES} ${HEADERS} 
#"includes/Server.h")

#with target_include_directories
add_executable(Server "Chat_App.cpp" "User.cpp" "UserDatabase.cpp" "Server.cpp")
target_include_directories(Server PRIVATE "includes/" "Utils/")
target_link_libraries(Server ws2_32)

# Add source to this project's executable.
add_executable (Client "Client/Client.cpp")
target_include_directories(Client PRIVATE "includes/" "Utils/")
target_link_libraries(Client ws2_32)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Server PROPERTY CXX_STANDARD 20)
endif()