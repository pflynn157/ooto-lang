add_subdirectory(array)
add_subdirectory(basic)
add_subdirectory(class)
add_subdirectory(cond)
add_subdirectory(enum)
add_subdirectory(float)
add_subdirectory(func)
add_subdirectory(loop)
add_subdirectory(str)
add_subdirectory(struct)
add_subdirectory(syntax)
add_subdirectory(java)

add_custom_target(test_orka DEPENDS
    test_orka_array
    test_orka_basic
    test_orka_class
    test_orka_cond
    test_orka_enum
    test_orka_float
    test_orka_func
    test_orka_loop
    test_orka_str
    test_orka_struct
    test_orka_syntax
    test_orka_java
)

