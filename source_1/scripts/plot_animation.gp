box_size = int(system("sed -n 1p data/init_data.txt | tr -d -c 0-9"))
box_size_z = int(system("sed -n 2p data/init_data.txt | tr -d -c 0-9"))
nt = system("sed -n 3p data/init_data.txt | tr -d -c 0-9")
rad_t = system("sed -n 6p data/init_data.txt | tr -d -c 0-9.")
dt = system("sed -n 10p data/init_data.txt | tr -d -c 0-9.")
iterations = int(system("sed -n 11p data/init_data.txt | tr -d -c 0-9"))
resolution = int(system("sed -n 12p data/init_data.txt | tr -d -c 0-9"))

densidad = 4*(rad_t*rad_t*nt)/(box_size*box_size)
T = int(iterations/resolution)

set term png

set view equal xyz
set hidden front
set xr [0:box_size]
set yr [0:box_size]
set zr [0:box_size_z]
set xyplane at 0

set xl sprintf("X(%i m)", box_size)
set yl sprintf("Y(%i m)", box_size)
set zl sprintf("Z(%i m)", box_size_z)

unset xtics
unset ytics
unset ztics

Init = sprintf(" {/:Bold Densidad:} %.2f", densidad)
set lab 1 Init at graph 1.2, 0

set ls 1 lc rgb "blue" pt 7 ps 0.01
set ls 2 lc rgb "black" pt 7 ps 0.02

set title 'Posiciones de trampas y pelotas'

do for [t=0:(T-1)] {
  outfile = sprintf("data/animation/animation-%i.png", t)
  time = (t*resolution)*dt
  time_t = sprintf("Time: %.3f",time)
  
  file_b = sprintf("data/visual/data_balls-%i.csv", t)
  file_t = sprintf("data/visual/data_traps-%i.csv", t)

  set o outfile
  sp file_b u 1:2:3 w p ls 1 t time_t, file_t w p ls 2 notitle, 'data/visual/box.txt' u 1:2:3 w l lc rgb 'black' notitle
}

