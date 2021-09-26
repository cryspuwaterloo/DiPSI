./PSImain.cpp_exe -n 4096 -m 2097150 -b 21 -L 450 2>&1 | tee n4096_m2097153_b21_L450.txt &&
./PSImain.cpp_exe -n 1024 -m 2097150 -b 21 -L 450 2>&1 | tee n1024_m2097153_b21_L450.txt &&
./PSImain.cpp_exe -n 8192 -m 2097150 -b 21 -L 450 2>&1 | tee n8192_m2097153_b21_L450.txt &&
./PSImain.cpp_exe -n 8192 -m 65534 -b 16 -L 300 2>&1 | tee n8192_m65535_b16_L300.txt &&
./PSImain.cpp_exe -n 8192 -m 32767 -b 15 -L 300 2>&1 | tee n8192_m32767_b15_L300.txt &&

./PSImain.cpp_exe -n 8192 -m 4094 -b 12 -L 300 2>&1 | tee n8192_m4095_b12_L300.txt &&
./PSImain.cpp_exe -n 8192 -m 8190 -b 13 -L 300 2>&1 | tee n8192_m8191_b13_L300.txt &&
./PSImain.cpp_exe -n 8192 -m 16382 -b 14 -L 300 2>&1 | tee n8192_m16383_b14_L300.txt &&
./PSImain.cpp_exe -n 8192 -m 131070 -b 17 -L 450 2>&1 | tee n8192_m131071_b17_L450.txt &&
./PSImain.cpp_exe -n 8192 -m 262142 -b 18 -L 450 2>&1 | tee n8192_m262143_b18_L450.txt &&
./PSImain.cpp_exe -n 8192 -m 524286 -b 19 -L 450 2>&1 | tee n8192_m524287_b19_L450.txt &&
./PSImain.cpp_exe -n 8192 -m 1048574 -b 20 -L 450 2>&1 | tee n8192_m1048575_b20_L450.txt &&


./PSImain.cpp_exe -n 4096 -m 1048574 -b 20 -L 450 2>&1 | tee n4096_m1048575_b20_L450.txt &&
./PSImain.cpp_exe -n 4096 -m 524286 -b 19 -L 450 2>&1 | tee n4096_m524287_b19_L450.txt &&
./PSImain.cpp_exe -n 4096 -m 262142 -b 18 -L 450 2>&1 | tee n4096_m262144_b18_L450.txt &&
./PSImain.cpp_exe -n 4096 -m 131070 -b 17 -L 450 2>&1 | tee n4096_m131071_b17_L450.txt 

