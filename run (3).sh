#!/usr/bin/env bash
# Task B4 - extension (new bill type + new payer), sanitizer report, plug-in alternative
cd "$(dirname "$0")" && source ../common.sh
fresh B4_extension
cap B4_extension "$CXX b4_extension.cpp -o b4_extension"
cap B4_extension "./b4_extension"
fresh B4_regression
cap B4_regression "cd ../TaskB1 && $CXX b1_validate.cpp -o b1_validate && ./b1_validate bills.txt | tail -2"
cap B4_regression "cd ../TaskB2 && $CXX b2_errors.cpp -o b2_errors && ./b2_errors | grep -E 'summary|finished'"
fresh B4_asan_report
cap B4_asan_report "$CXX $ASAN -fsanitize=undefined b4_extension.cpp -o b4_extension_asan"
cap B4_asan_report "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./b4_extension_asan > asan_run.log 2>&1; echo \"exit status \$?\"; echo \"sanitizer findings: \$(grep -cE '==ERROR|runtime error:|LeakSanitizer' asan_run.log)\"; tail -3 asan_run.log"
fresh B4_alt_plugin_rules
cap B4_alt_plugin_rules "$CXX -O2 alt_plugin_rules.cpp -o alt_plugin_rules"
cap B4_alt_plugin_rules "./alt_plugin_rules"
