nauty_path="/home/vercee/libraries/nauty2_9_3"

# Set k
k=8

n_min=$(expr $k + 3)
n_max=$(( 2*k ))

# for loop
for n in $(seq $n_min $n_max);
do
  # Set number of edges
  e_max=$(expr $n + $k)

  # Generate with nauty
  ${nauty_path}/geng -C -d3 $n  ${e_max}:${e_max} > ${n}_${k}.g6

  # Show them in human readble format
  ${nauty_path}/showg -t -A -q  ${n}_${k}.g6 > ${n}_${k}.txt
done
