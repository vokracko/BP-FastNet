filename_tbm(ipv, stride) = sprintf("ipv%d-tbm-%s.dat", ipv, stride)
filename_bspl(ipv) = sprintf("ipv%d-bspl-1.dat", ipv)
filename_output(ipv) = sprintf("ipv%d-bench.pdf", ipv)

set terminal pdf enhanced
set output filename_output(ipv)

set style data histogram
set style histogram cluster gap 1

set style fill solid border rgb "black"
set auto x
set yrange [0:*]



plot for [stride in strides] filename_tbm(ipv, stride) using 2:xtic(1) title col, \
	filename_bspl(ipv) using 2:xtic(1) title col
