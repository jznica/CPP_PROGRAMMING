#!/usr/bin/env bash
# Rebuilds and reruns every program of Assignment 1.
# Each run is saved next to its source as <name>_output.txt (command, compiler messages, output).
cd "$(dirname "$0")"
C="g++ -std=c++17 -Wall -Wextra"

run() {   # run <folder> <output name> <compile command> <run command>
    local dir=$1 name=$2 build=$3 exe=$4
    (
        cd "$dir"
        echo "\$ $build${exe:+ && $exe}"
        if eval "$build" 2>&1; then
            [ -n "$exe" ] && { eval "$exe" 2>&1; echo "[exit code $?]"; }
        else
            echo "[compilation failed]"
        fi
    ) > "$dir/${name}_output.txt" 2>&1
    echo "done: $dir/${name}_output.txt"
}

run Task02 constants_demo     "$C constants_demo.cpp -o constants_demo"            "./constants_demo"
run Task02 constants_error    "$C -DSHOW_ERROR -c constants_demo.cpp"              ""
run Task03 types_demo         "$C types_demo.cpp -o types_demo"                    "./types_demo"
run Task04 predict            "$C predict.cpp -o predict"                          "./predict"
run Task04 fixed              "$C fixed.cpp -o fixed"                              "./fixed"
run Task04 fixed_error        "$C -DSHOW_ERROR -c fixed.cpp"                       ""
run Task05 predict            "$C predict.cpp -o predict"                          "./predict"
run Task05 expressions        "$C expressions.cpp -o expressions"                  "./expressions"
run Task06 complete           "$C complete.cpp -o complete"                        "./complete"
run Task06 boundary           "$C boundary.cpp -o boundary"                        "./boundary"
run Task07 procedural         "$C procedural.cpp -o procedural"                    "./procedural"
run Task07 oop                "$C oop.cpp -o oop"                                  "./oop"
run Task07 oop_error          "$C -DSHOW_ERROR -c oop.cpp"                         ""
run Task08 gate_io            "$C gate_io.cpp -o gate_io"                          "./gate_io < input_bad.txt"
run Task08 getline_stoi       "$C getline_stoi.cpp -o getline_stoi"                "./getline_stoi < hours.txt"
run Task09 metro_fare         "$C metro_fare.cpp -o metro_fare"                    "./metro_fare"
run Task10 totals             "$C -O2 totals.cpp -o totals"                        "./totals"
run Task10 warnings_demo      "$C warnings_demo.cpp -o warnings_demo"              "./warnings_demo"
run Task11 config_loader      "$C -O2 config_loader.cpp -o config_loader"          "./config_loader fares.cfg"
run Task12 impact_demo        "$C impact_demo.cpp -o impact_demo"                  "./impact_demo"
run Task12 metro_v2           "$C metro_v2.cpp -o metro_v2"                        "./metro_v2"
run Benchmarks benchmarks     "$C -O2 benchmarks.cpp -o benchmarks"                "./benchmarks"

# remove compiled files so the repository holds only sources and outputs
find Task* Benchmarks -maxdepth 1 -type f ! -name '*.cpp' ! -name '*.txt' ! -name '*.cfg' ! -name '*.md' ! -name '*.png' -delete
