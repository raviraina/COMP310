#!/bin/bash

cd $1

ls
mkdir -p output

cp ~/Projects/COMP310test/A3Solution_modified/testcases_private/prog1 .
cp ~/Projects/COMP310test/A3Solution_modified/testcases_private/prog2 .
cp ~/Projects/COMP310test/A3Solution_modified/testcases_private/prog3 .
cp ~/Projects/COMP310test/A3Solution_modified/testcases_private/prog4 .
cp ~/Projects/COMP310test/A3Solution_modified/testcases_private/prog5 .
cp ~/Projects/COMP310test/A3Solution_modified/testcases_private/prog6 .
cp ~/Projects/COMP310test/A3Solution_modified/testcases_private/prog7 .
cp ~/Projects/COMP310test/A3Solution_modified/testcases_private/prog8 .
cp ~/Projects/COMP310test/A3Solution_modified/testcases_private/prog9 .
cp ~/Projects/COMP310test/A3Solution_modified/testcases_private/prog10 .
cp ~/Projects/COMP310test/A3Solution_modified/testcases_private/prog11 .
cp ~/Projects/COMP310test/A3Solution_modified/testcases_private/prog12 .

echo "-- test tc1 --"
make clean; make mysh framesize=18 varmemsize=10
./mysh < ../A3Solution_modified/testcases_private/tc1 > output/tc1_private_result
diff -y ../A3Solution_modified/testcases_private/tc1_result output/tc1_private_result

echo "-- test tc2 --"
make clean; make mysh framesize=18 varmemsize=10
./mysh < ../A3Solution_modified/testcases_private/tc2 > output/tc2_private_result
diff -y  ../A3Solution_modified/testcases_private/tc2_result output/tc2_private_result

echo "-- test tc3 --"
make clean; make mysh framesize=21 varmemsize=10
./mysh <  ../A3Solution_modified/testcases_private/tc3 > output/tc3_private_result
diff -y   ../A3Solution_modified/testcases_private/tc3_result output/tc3_private_result

echo "--test tc4 --"
make clean; make mysh framesize=18 varmemsize=10
./mysh <  ../A3Solution_modified/testcases_private/tc4 > output/tc4_private_result
diff -y   ../A3Solution_modified/testcases_private/tc4_result output/tc4_private_result

echo "-- test tc5 --"
make clean; make mysh framesize=6 varmemsize=10
./mysh < ../A3Solution_modified/testcases_private/tc5 > output/tc5_private_result
diff -y  ../A3Solution_modified/testcases_private/tc5_result output/tc5_private_result

echo "-- done --"
