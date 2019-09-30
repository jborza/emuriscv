copy /y d:\riscv-pk\build\bbl32.* .
get_function_table.sh 
python get_function_table.py > ..\debug_symbols.c