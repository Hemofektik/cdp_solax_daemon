file(GLOB_RECURSE SOURCES *.cpp *.cc *.h *.hpp *.c)

foreach(f ${SOURCES})
    # Get the path of the file relative to ${DIRECTORY},
    # then alter it (not compulsory)
    file(RELATIVE_PATH SRCGR ${CMAKE_CURRENT_SOURCE_DIR} ${f})
    set(SRCGR "Sources/${SRCGR}")

    # Extract the folder, ie remove the filename part
    string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})

    # Source_group expects \\ (double antislash), not / (slash)
    string(REPLACE / \\ SRCGR ${SRCGR})
    source_group("${SRCGR}" FILES ${f})
endforeach()

string(REPLACE solax.cpp "" SOURCESwithoutMain "${SOURCES}")

add_library(solax 
   ${SOURCESwithoutMain}
)

add_executable (
   solax_daemon
   solax.cpp
)

target_link_libraries(solax
   Backward::Interface
   OpenSSL::SSL
   cpprestsdk::cpprest
   CppLinuxSerial::CppLinuxSerial
   config++
)


target_link_libraries(solax_daemon PRIVATE 
   solax
)


install(TARGETS solax)
install(TARGETS solax_daemon DESTINATION /opt/solax)
