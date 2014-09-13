project(Babah_Scheme)
add_executable(scm main.c mpc/mpc.c)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -std=c99 -D_GNU_SOURCE -ggdb")
target_link_libraries(scm edit m)
