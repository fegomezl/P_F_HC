box_size = int(system("sed -n 1p data/init_data.txt | tr -d -c 0-9"))
nt = system("sed -n 3p data/init_data.txt | tr -d -c 0-9")
rad_t = system("sed -n 6p data/init_data.txt | tr -d -c 0-9.")
dt = system("sed -n 10p data/init_data.txt | tr -d -c 0-9.")
iterations = int(system("sed -n 11p data/init_data.txt | tr -d -c 0-9"))
resolution = int(system("sed -n 12p data/init_data.txt | tr -d -c 0-9"))

densidad = 4*(rad_t*rad_t*nt)/(box_size*box_size)
T = int(iterations/resolution)

set term png
set xl sprintf("X(%i m)", box_size)
set yl sprintf("Y(%i m)", box_size)
unset xtics
unset ytics
set ztics axis
set xyplane at 0

Init = sprintf(" {/:Bold Densidad:} %.2f", densidad)
set lab 1 Init at graph 1.2, 0

set dgrid3d 50,50 qnorm 2
set hidden3d
set pm3d
set cbrange [0:]

set title 'Distribución acumulada de probabilidad de exitación'

do for [t=0:(T-1)] {
  infile = sprintf("data/system_parameters/data_activation-%i.csv", t)
  outfile = sprintf("data/distribution/distribution-%i.png", t)
  time = (t*resolution)*dt
  time_t = sprintf("Time: %.3f",time)
  set o outfile
  sp [0:box_size][0:box_size][0:] infile u 1:2:3 w l t time_t
}
