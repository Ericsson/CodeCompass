message("Install userguide...")

file(WRITE ${INSTALL_USERGUIDE_DIR}/userguide.md "")
foreach(_userguide ${USERGUIDES})
  execute_process(
    COMMAND cat ${_userguide}
    OUTPUT_VARIABLE _content
    WORKING_DIRECTORY ${INSTALL_USERGUIDE_DIR})
  file(APPEND ${INSTALL_USERGUIDE_DIR}/userguide.md ${_content})
endforeach(_userguide)

execute_process(
  COMMAND doxygen
  WORKING_DIRECTORY ${INSTALL_USERGUIDE_DIR})

message("Install userguide has been finished.")