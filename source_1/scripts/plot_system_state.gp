set g
set ls 1 lc rgb "blue" lt 1 lw 0.5
set ls 2 lc rgb "red" lt 1 lw 0.5

box_size = system("sed -n 1p data/init_data.txt | tr -d -c 0-9")*1
nt = int(system("sed -n 3p data/init_data.txt | tr -d -c 0-9"))
rad_t = system("sed -n 6p data/init_data.txt | tr -d -c 0-9.")*1
densidad = 4*(rad_t*rad_t*nt)/(box_size*box_size)

Init = sprintf(" {/:Bold Densidad:} %.2f", densidad)

set xr [0:]
set xl 'Tiempo (s)'
set key opaque
set term pdf

set obj 1 rect from graph 0.8, 0.9 to graph 1, 0.83 fc rgb "white"
set lab 1 Init at graph 0.8, 0.86 

set title 'Energia vs tiempo'
set yr [0:]
set yl 'Energía (MJ)'
set o 'data/energy.pdf'
p 'data/system_parameters/system_state.txt' u 1:2 w l ls 1 t 'Energía'

set obj 1 rect from graph 0, 1 to graph 0.2, 0.93 fc rgb "white"
set lab 1 Init at graph 0, 0.96

set title 'Porcentaje de activaciones vs tiempo'
set yr [0:]
set yl 'Porcentaje de trampas activadas'
set o 'data/activation_partial.pdf'
p 'data/system_parameters/system_state.txt' u 1:3 w l ls 1 t 'Activaciones'

unset format y

set title 'Porcentaje de activaciones acumuladas vs tiempo'
set yr [0:1]
set yl 'Porcentaje de trampas activadas'
set o 'data/activation_total.pdf'
p 'data/system_parameters/system_state.txt' u 1:4 w l ls 1 t 'Activaciones'
