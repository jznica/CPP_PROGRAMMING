#!/usr/bin/env bash
# Task B1 - polymorphic billing: validation, complete-the-code, binding and destructor experiments
cd "$(dirname "$0")" && source ../common.sh
fresh B1_validate
cap B1_validate "$CXX b1_validate.cpp -o b1_validate"
cap B1_validate "./b1_validate bills.txt"
cap B1_validate "$CXX $ASAN -fsanitize=undefined b1_validate.cpp -o b1_validate_asan"
cap B1_validate "./b1_validate_asan bills.txt"
fresh B1_complete_the_code
cap B1_complete_the_code "$CXX complete_the_code.cpp -o complete_the_code"
cap B1_complete_the_code "./complete_the_code"
cap B1_complete_the_code "$CXX $ASAN complete_the_code.cpp -o complete_the_code_asan && ./complete_the_code_asan"
cap B1_complete_the_code "$CXX experiments/complete_nonvirtual_total.cpp -o /dev/null"
fresh B1_3_runtime_binding
cap B1_3_runtime_binding "$CXX -DUSE_VIRTUAL experiments/b1_3_nonvirtual_total.cpp -o b1_3_virtual && ./b1_3_virtual"
cap B1_3_runtime_binding "$CXX experiments/b1_3_nonvirtual_total.cpp -o b1_3_nonvirtual && ./b1_3_nonvirtual"
fresh B1_4_virtual_destructor
cap B1_4_virtual_destructor "$CXX $ASAN -DUSE_VIRTUAL_DTOR experiments/b1_4_nonvirtual_dtor.cpp -o b1_4_virtual && ./b1_4_virtual"
cap B1_4_virtual_destructor "$CXX $ASAN experiments/b1_4_nonvirtual_dtor.cpp -o b1_4_nonvirtual"
cap B1_4_virtual_destructor "./b1_4_nonvirtual"
cap B1_4_virtual_destructor "ASAN_OPTIONS=new_delete_type_mismatch=0 ./b1_4_nonvirtual"
