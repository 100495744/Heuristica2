#!/bin/bash

# Definicion del directorio que contiene los tests
test_dir="./ASTAR-tests"

# Example calls for the A* program
./ASTARRodaje "$test_dir/mapa1.csv" 1
./ASTARRodaje "$test_dir/mapa2.csv" 1
./ASTARRodaje "$test_dir/mapa1.csv" 2
./ASTARRodaje "$test_dir/mapa2.csv" 2

# Notify completion
echo "All test cases executed. Check the output and stat files in $test_dir."