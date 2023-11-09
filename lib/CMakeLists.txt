add_library(lluacCompiler Lexer.cpp)
target_link_libraries(lluacCompiler PRIVATE LLVMSupport)
