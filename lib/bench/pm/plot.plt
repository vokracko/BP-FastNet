set terminal pdf enhanced
set output "pm.pdf"

set style data histogram
set style histogram cluster gap 1
set key outside
set key off
set style fill solid border rgb "black"
set auto x
set yrange [0:*]
set ylabel "ms"

plot "output/output.dat" using 2:xtic(1) title col
