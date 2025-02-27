set(CORE_TEST_SRC
    test1
    bitwise_and bitwise_or bitwise_xor
    bool1 bool2
    ret_math1 ret_math2 ret_math3 ret_math4 ret_math5
    byte1 ubyte1
    #char1
    const1 const2
    int64_1 uint64_t
    neg1
    short1 ushort1
    uint1
    op_pred mod math
    call1
    
    associative1 comma_var1 parens1
    
    cond1 cond2 cond3 cond4
    cond_byte cond_ubyte
    cond_short cond_ushort
    cond_int64 cond_uint64
    lg_and1 lg_or1
    
    break continue for1 forall1
    nested1 nested2 nested3
    repeat1 step1 while1
    
    struct1 struct2
    struct3 struct4
    
    array1
)

foreach(ITEM ${CORE_TEST_SRC})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.exe
        COMMAND ${CMAKE_BINARY_DIR}/riya-lang/riyac ${CMAKE_CURRENT_SOURCE_DIR}/${ITEM}.ry -o ${ITEM}.exe
        COMMAND ./${ITEM}.exe
        COMMAND rm ${ITEM}.exe
        COMMAND echo "[PASS] ${ITEM}.ry"
    )
    
    set(TEST_OUTPUTS
        ${TEST_OUTPUTS}
        ${CMAKE_CURRENT_BINARY_DIR}/${ITEM}.exe
    )
endforeach()

add_custom_target(test_core
    DEPENDS ${TEST_OUTPUTS}
)

add_dependencies(test_core riyac)

