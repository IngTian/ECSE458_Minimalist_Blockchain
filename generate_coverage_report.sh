#!/bin/bash
cd ./cmake-build-debug
xcrun llvm-profdata merge -sparse default.profraw -o default.profdata
xcrun llvm-cov report ./test -instr-profile=default.profdata -show-instantiation-summary --show-functions ../src/**/*.c > ../coverage.log
cd ../
