cmake_minimum_required (VERSION 2.6)
set(CMAKE_MODULE_PATH 
  ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/"
)

include(BoostUtils)
#include(BoostDocs)

#boost_add_documentation(yamail.qbk
#  HEADERS reference.qbk yamail
#  DOXYGEN_PARAMETERS "RECURSIVE = YES"
#)

include(XslTransform)

set(DOXYFILE ${CMAKE_SOURCE_DIR}/doc/reference.dox)

# Generate Doxygen XML
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/xml/index.xml
  COMMAND ${DOXYGEN} ${DOXYFILE}
  COMMENT "Generating Doxygen XML output for Boost.${PROJECT_NAME}..."
#  DEPENDS ${THIS_DOXY_HEADERS}
)

# Collect Doxygen XML into a single XML file
set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/xml/combine.xslt
  PROPERTIES GENERATED TRUE)

xsl_transform(
  ${CMAKE_CURRENT_BINARY_DIR}/xml/all.xml
  ${CMAKE_CURRENT_BINARY_DIR}/xml/index.xml
  STYLESHEET ${CMAKE_CURRENT_BINARY_DIR}/xml/combine.xslt
  COMMENT "Collecting Doxygen XML output for Boost.${PROJECT_NAME}...")

xsl_transform(${CMAKE_CURRENT_BINARY_DIR}/reference.qbk
  ${CMAKE_CURRENT_BINARY_DIR}/xml/all.xml
  STYLESHEET ${CMAKE_SOURCE_DIR}/doc/reference.xsl
  COMMENT "Transforming Doxygen XML into QuickBook format for Boost.${PROJECT_NAME}..."
  MAKE_TARGET ${PROJECT_NAME}-doxygen)

set(BOOSTBOOK_FILE yamail.xml)
add_custom_command(OUTPUT ${BOOSTBOOK_FILE}
  COMMAND quickbook "-I." "-I${CMAKE_SOURCE_DIR}/doc" "--output-file=${BOOSTBOOK_FILE}" ${CMAKE_SOURCE_DIR}/doc/yamail.qbk
  DEPENDS ${CMAKE_SOURCE_DIR}/doc/yamail.qbk ${PROJECT_NAME}-doxygen
  COMMENT "Generating BoostBook documentation for Boost.${PROJECT_NAME}...")

set(DOCBOOK_FILE yamail.docbook)
xsl_transform(
  ${CMAKE_CURRENT_BINARY_DIR}/yamail.docbook
  ${CMAKE_CURRENT_BINARY_DIR}/yamail.xml
  STYLESHEET "/Users/nikki/Work/boost/modular-clang/tools/boostbook/xsl/docbook.xsl"
  CATALOG ${CMAKE_BINARY_DIR}/catalog.xml
  COMMENT "Generating DocBook documentation for Boost.${PROJECT_NAME}..."
  MAKE_TARGET ${PROJECT_NAME}-docbook)

set_target_properties(
         ${PROJECT_NAME}-docbook PROPERTIES
         EXCLUDE_FROM_DEFAULT_BUILD TRUE
         EXCLUDE_FROM_ALL TRUE

xsl_transform(
  ${CMAKE_CURRENT_BINARY_DIR}/html
  ${CMAKE_CURRENT_BINARY_DIR}/yamail.docbook
  STYLESHEET ${BOOSTBOOK_XSL_DIR}/html.xsl
  CATALOG ${CMAKE_BINARY_DIR}/catalog.xml
  DIRECTORY HTML.manifest
  PARAMETERS admon.graphics.path=images/
             navig.graphics.path=images/
             boost.image.src=boost.png
             chapter.autolabel=0
             chunk.first.sections=1
             chunk.section.depth=8
             generate.section.toc.level=1
             toc.max.depth=1
             toc.section.depth=2
  COMMENT "Generating HTML documentaiton for Boost.${PROJECT_NAME}..."
  MAKE_TARGET ${PROJECT_NAME}-html)

set_target_properties(
         ${PROJECT_NAME}-html PROPERTIES
         EXCLUDE_FROM_DEFAULT_BUILD TRUE
         EXCLUDE_FROM_ALL TRUE

add_custom_command(TARGET ${PROJECT_NAME}-html
  POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/doc/html ${CMAKE_CURRENT_BINARY_DIR}/html
    # COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/boost.png ${CMAKE_CURRENT_BINARY_DIR}/html
)
