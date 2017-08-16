set key autotitle columnhead
set style fill solid border -1
set style data histograms
set xtics 0,1,30
#plot  "ftp_hist.csv" using 1:2 with lines
plot  "ftp_hist.csv" using 2:xticlabels(1) lc rgb 'green'