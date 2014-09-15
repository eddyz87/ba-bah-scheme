project(Babah_Scheme)

add_library(mpc OBJECT mpc/mpc.c mpc/mpc.h)
add_library(utils OBJECT utils.c utils.h)
add_library(lisp_types OBJECT lisp_types.c lisp_types.h)
add_library(cons_pool OBJECT cons_pool.c cons_pool.h lisp_types.h)
add_library(symbols OBJECT symbols.c symbols.h lisp_types.h)

add_executable(scm
  main.c
  $<TARGET_OBJECTS:mpc>  
  $<TARGET_OBJECTS:utils>
  $<TARGET_OBJECTS:lisp_types>
  $<TARGET_OBJECTS:cons_pool>
  $<TARGET_OBJECTS:symbols>
  )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -std=c99 -D_GNU_SOURCE -ggdb")
target_link_libraries(scm edit m)
