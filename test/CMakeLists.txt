add_subdirectory(riya/core)
add_subdirectory(riya/output)
add_subdirectory(riya/lex)
add_subdirectory(orka)
add_subdirectory(riyai)

if (DISABLE_OUTPUT_TEST)
    add_custom_target(test_llvm DEPENDS
        test_core
        test_lex
    )
else()
    add_custom_target(test_llvm DEPENDS
        test_core
        test_output
        test_lex
    )
endif()

##
## The target for all tests
##
add_custom_target(test DEPENDS
    test_llvm
    test_orka
    test_riyai
)

