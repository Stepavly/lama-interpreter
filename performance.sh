lamac=$1
$lamac -b performance/Sort.lama
mv Sort.bc performance/
touch performance/empty

echo "Executing lamac interpreter"
time $lamac -i performance/Sort.lama < performance/empty

echo "Executing iterative interpreter"
time ./byterun/byterun performance/Sort.bc

rm performance/Sort.bc performance/empty