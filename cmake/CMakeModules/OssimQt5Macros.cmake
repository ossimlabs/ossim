include(Qt5Macros)
MACRO (OSSIM_QT_WRAP_UI outfiles)
  QT5_EXTRACT_OPTIONS(ui_files ui_options ${ARGN})
  FOREACH (it ${ui_files})
    GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
    GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
    IF(UI_DESTINATION_DIR)
      SET(outfile ${UI_DESTINATION_DIR}/ui_${outfile}.h)
    ELSE(UI_DESTINATION_DIR)
      SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/ui_${outfile}.h)
    ENDIF(UI_DESTINATION_DIR)
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
      COMMAND ${QT_UIC_EXECUTABLE}
      ARGS ${ui_options} -o ${outfile} ${infile}
      MAIN_DEPENDENCY ${infile})
    SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH (it)

ENDMACRO (OSSIM_QT_WRAP_UI)
