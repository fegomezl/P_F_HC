set g
set ls 1 lc rgb "red" lt 1 lw 2
set ls 2 lc rgb '#FEE08B' # pale yellow-orange
set ls 3 lc rgb '#E6F598' # pale yellow-green
set ls 4 lc rgb '#ABDDA4' # pale green
set ls 5 lc rgb '#66C2A5' # green
set ls 6 lc rgb '#3288BD' # blue

set key font ",8"
set key opaque

set xr [1:]
set yr [0:]
set xl 'Número de procesadores'
set term pdf

set title 'Speed up vs Procesadores'
set yl 'Speed up'
set o 'data/speed_up.pdf'
f(x) = x

index = 1
plot_speedup = ""

do for [t in "1024 1369 2116 4096 6084"]{
   t = t + 0
   index = index + 1
   plot_speedup = plot_speedup.sprintf("'data/scaling/metrics-%i.txt' u 2:3 w l ls %i t 'N = %i', ", t, index, t)
   plot_speedup = plot_speedup.sprintf("'data/scaling/metrics-%i.txt' u 2:3:5 w errorb ls %i notitle, ", t, index)
}

plot_speedup = plot_speedup."f(x) ls 1 t 'Teórica'"

p @plot_speedup

set title 'Eficiencia paralela vs Procesadores'
set yl 'Eficiencia paralela'
set o 'data/parallel_efficiency.pdf'
g(x) = 1

index = 1
plot_efficiency  = ""

do for [t in "1024 1369 2116 4096 6084"]{
   t = t + 0
   index = index + 1
   plot_efficiency = plot_efficiency.sprintf("'data/scaling/metrics-%i.txt' u 2:4 w l ls %i t 'N = %i', ", t, index, t)
}

plot_efficiency = plot_efficiency."g(x) ls 1 t 'Teórica'"

p @plot_efficiency
