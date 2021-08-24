set term pdf
set o 'data/start.pdf'
set title 'Posiciones iniciales de trampas y pelotas'

set view equal xyz
set hidden front
set xyplane at 0
set parametric		#
set ur [0:2*pi]		#
set vr [-pi/2.0:pi/2.0]	# curvas parametricas

xsize = int(system("sed -n 1p data/init_data.txt | tr -d -c 0-9"))
zsize = int(system("sed -n 2p data/init_data.txt | tr -d -c 0-9"))
nt = system("sed -n 3p data/init_data.txt | tr -d -c 0-9")
rad_t = system("sed -n 6p data/init_data.txt | tr -d -c 0-9.")

densidad = 4*(rad_t*rad_t*nt)/(xsize*xsize)

set xr [0:xsize]
set yr [0:xsize]
set zr [0:zsize]

set xl sprintf("X(%i m)", xsize)
set yl sprintf("Y(%i m)", xsize)
set zl sprintf("Z(%i m)", zsize)

unset xtics
unset ytics
unset ztics

Init = sprintf(" {/:Bold Densidad:} %.2f", densidad)
set lab 1 Init at graph 1.2, 0

fileballs = 'data/visual/data_balls-0.csv'
filetraps = 'data/visual/data_traps-0.csv'

# get number of data-lines in fileballs
nlines   = system(sprintf('grep -v ^# %s | wc -l', fileballs))

plotballs = "" # aqui se guardan las instrucciones de ploteo

do for [i=1:nlines] {
   line = system( sprintf('grep -v ^# %s | awk "NR == %i {print; exit}" ', fileballs, i) )

   x = word(line,1)	#
   y = word(line,2)	#
   z = word(line,3)	#
   r = word(line,4)	# posición y radio de las pelotas

   plotballs = plotballs.sprintf('%s+%s*cos(v)*cos(u),%s+%s*cos(v)*sin(u),%s+%s*sin(v) lt rgb "blue" notitle', x, r, y, r, z, r)
   
   # añadir comas para separar instrucciones
   if(i<nlines) { plotballs = plotballs . ',  ' }
}

# procedimiento analogo para las trampas
nlines   = system(sprintf('grep -v ^# %s | wc -l', filetraps))

plottraps = ""

do for [i=1:nlines] {
   line = system( sprintf('grep -v ^# %s | awk "NR == %i {print; exit}" ', filetraps, i) )

   x = word(line,1)
   y = word(line,2)
   z = word(line,3)
   r = word(line,4)

   plottraps = plottraps.sprintf('%s+%s*cos(u),%s+%s*sin(u), %s lt rgb "violet" notitle', x, r, y, r, z)
   
   if(i<nlines) { plottraps = plottraps . ',  ' }
}

plotbox = "'data/visual/box.txt' u 1:2:3 w l lc rgb 'black' notitle" # datos para dibujar la caja

plot = plotballs.', '.plottraps.', '.plotbox # compilación de todas las instrucciones de ploteo

set macros
splot @plot
