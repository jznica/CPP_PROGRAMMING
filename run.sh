#!/usr/bin/env bash
# Task B2 - engineered error conditions, Predict the Output, Complete the Code, throw vs throw e
cd "$(dirname "$0")" && source ../common.sh
fresh B2_errors
cap B2_errors "$CXX b2_errors.cpp -o b2_errors"
cap B2_errors "./b2_errors"
fresh B2_errors_asan
cap B2_errors_asan "$CXX $ASAN b2_errors.cpp -o b2_errors_asan"
cap B2_errors_asan "./b2_errors_asan"
fresh B2_predict_output
cap B2_predict_output "$CXX predict_output.cpp -o predict_output"
cap B2_predict_output "./predict_output"
fresh B2_complete_the_code
cap B2_complete_the_code "$CXX complete_the_code.cpp -o complete_the_code"
cap B2_complete_the_code "./complete_the_code"
cap B2_complete_the_code "$CXX complete_the_code_throw_e.cpp -o complete_the_code_throw_e"
cap B2_complete_the_code "./complete_the_code_throw_e"
fresh B2_throw_vs_throw_e
cap B2_throw_vs_throw_e "$CXX rethrow_throw_e.cpp -o rethrow && ./rethrow"
cap B2_throw_vs_throw_e "$CXX -DRETHROW_COPY rethrow_throw_e.cpp -o rethrow_copy && ./rethrow_copy"
