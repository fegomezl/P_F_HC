set g
set key opaque
set term pdf
set ls 1 lc rgb "dark-blue" lt 1 lw 0.5
set ls 2 lc rgb "dark-red" lt 1 lw 0.5

box_size = system("sed -n 1p data/init_data.txt | tr -d -c 0-9")*1
nt = int(system("sed -n 3p data/init_data.txt | tr -d -c 0-9"))
rad = system("sed -n 6p data/init_data.txt | tr -d -c 0-9.")*1
densidad = 4*(rad*rad*nt)/(box_size*box_size)

Init = sprintf(" {/:Bold Densidad:} %.2f", densidad)

set xr [0:]
set xl 'Tiempo (s)'

set obj 1 rect from graph 0, 1 to graph 0.22, 0.93 fc rgb "white"
set lab 1 Init at graph 0, 0.96

set title 'Evolucion temporal de la energia mecanica del sistema'
set yr [0:]
set yl 'Energía (MJ)'
set o 'data/energy.pdf'
p 'data/results/system_state.txt' u 1:2 w l ls 1 t 'Energía'

set title 'Densidad de probabilidad de activacion'
set yr [0:]
set yl 'f(x)'
set o 'data/activation_partial.pdf'
p 'data/results/system_state.txt' u 1:3 w l ls 1 t 'f(x)'

set title 'Distribucion acumulada de probabilidad de activacion'
set yr [0:1]
set yl 'F(x)'
set o 'data/activation_total.pdf'
p 'data/results/system_state.txt' u 1:4 w l ls 1 t 'F(x)'
