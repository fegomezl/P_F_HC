set g
set ls 1 lc rgb "blue" lt 1 lw 0.5

set xr [-0.4:0.6]
set yr [0:9]
set title 'Medición experimental de posiciones de caidas'
set xl 'x (m)'
set yl 'y (m)'
set term pdf
set k b r

set o 'data/system_parameters/exp_pos.pdf'
p 'data/exp_data_pos.txt' u 1:3:2:4 w xyerrorbars ls 1 t 'Datos experimentales'

set xr [0.8:1.8]
set yr [8:8.8]
set title 'Medición experimental de distancia recorrida y tiempo de vuelo'
set xl 't (s)'
set yl 'r (m)'

set o 'data/system_parameters/exp_time.pdf'
p 'data/exp_data_time.txt' u 1:2 ls 1 t 'Datos experimentales'
