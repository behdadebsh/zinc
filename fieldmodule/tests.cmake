
SET(CURRENT_TEST fieldmodule)
LIST(APPEND API_TESTS ${CURRENT_TEST})
SET(${CURRENT_TEST}_SRC
    ${CURRENT_TEST}/create_composite.cpp
    ${CURRENT_TEST}/create_derivatives.cpp
    ${CURRENT_TEST}/create_if.cpp
    ${CURRENT_TEST}/create_nodeset_operators.cpp
    )

SET(FIELDMODULE_EXNODE_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/nodes.exnode")
SET(FIELDMODULE_CUBE_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/cube.exformat")
