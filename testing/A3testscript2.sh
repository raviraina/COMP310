#!/bin/bash

cd $1

ls
mkdir -p output

cp ../A3Solution_modified/testcases_public/prog1 .
cp ../A3Solution_modified/testcases_public/prog2 .
cp ../A3Solution_modified/testcases_public/prog3 .
cp ../A3Solution_modified/testcases_public/prog4 .
cp ../A3Solution_modified/testcases_public/prog5 .
cp ../A3Solution_modified/testcases_public/prog6 .
cp ../A3Solution_modified/testcases_public/prog7 .
cp ../A3Solution_modified/testcases_public/prog8 .
cp ../A3Solution_modified/testcases_public/prog9 .
cp ../A3Solution_modified/testcases_public/prog10 .
cp ../A3Solution_modified/testcases_public/prog11 .
cp ../A3Solution_modified/testcases_public/prog12 .

echo "-- test tc1 --"
make clean; make mysh framesize=18 varmemsize=10
./mysh < ../A3Solution_modified/testcases_public/tc1 > output/tc1_result
diff -y ../A3Solution_modified/testcases_public/tc1_result output/tc1_result

echo "-- test tc2 --"
make clean; make mysh framesize=18 varmemsize=10
./mysh < ../A3Solution_modified/testcases_public/tc2 > output/tc2_result
diff -y  ../A3Solution_modified/testcases_public/tc2_result output/tc2_result

echo "-- test tc3 --"
make clean; make mysh framesize=21 varmemsize=10
./mysh <  ../A3Solution_modified/testcases_public/tc3 > output/tc3_result
diff -y   ../A3Solution_modified/testcases_public/tc3_result output/tc3_result

echo "--test tc4 --"
make clean; make mysh framesize=18 varmemsize=10
./mysh <  ../A3Solution_modified/testcases_public/tc4 > output/tc4_result
diff -y   ../A3Solution_modified/testcases_public/tc4_result output/tc4_result

echo "-- test tc5 --"
make clean; make mysh framesize=6 varmemsize=10
./mysh < ../A3Solution_modified/testcases_public/tc5 > output/tc5_result
diff -y  ../A3Solution_modified/testcases_public/tc5_result output/tc5_result

echo "-- done --"
