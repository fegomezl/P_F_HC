set g
set ls 1 lc rgb "blue" lt 1 lw 0.5
set ls 2 lc rgb "black" lt 1 lw 0.5
set ls 3 lc rgb "orange" lt 1 lw 0.5
set ls 4 lc rgb "red" lt 1 lw 0.5

file = 'data/results/analysis.txt'

set xl 'Densidad'
set yl 'Tasa de energía (J/s)'
set term pdf
set o 'data/analysis.pdf'
set key opaque
set key top center
set fit errorvar
set fit quiet

f(x) = a*x**b
a = 1
b = 0.5
fit[:] f(x) file u 1:2:3 yerrors via a,b

g(x) = c*x**d
c = 1
d = 0.5
fit[:] g(x) file u 1:4:5 yerrors via c,d

lamda = c/a
lamda_err = lamda*(c_err/c+a_err/a)
beta = 1/(b-d)
beta_err = beta*beta*(b_err+d_err)

inter = lamda**beta
inter_err = inter*(abs(log(lamda))*beta_err+beta*lamda_err/lamda)

Fit = sprintf(" y_{gain} = a*x^{b} \n a ≅ (%.0f ± %.0f) J/s \n b ≅ %.1f ± %.1f \n y_{lost} = c*x^{d} \n c ≅ (%.1f ± %.1f) J/s \n d ≅ (%.1f ± %.1f) \n\n x_{inter} ≅ %.2f ± %.2f", a, a_err, b, b_err, c, c_err, d, d_err, inter, inter_err)

set obj 2 rect from graph 0, 1 to graph 0.25, 0.59 fc rgb "white" front
set lab 2 Fit at graph 0, 0.97 front

plot_a = "'data/results/analysis.txt' u 1:2 w l ls 1 t 'Tasa de aumento', "
plot_a = plot_a."'data/results/analysis.txt' u 1:2:3 w errorb ls 1 notitle, "
plot_a = plot_a."'data/results/analysis.txt' u 1:4 w l ls 2 t 'Tasa de decremento', "
plot_a = plot_a."'data/results/analysis.txt' u 1:4:5 w errorb ls 2 notitle, "
plot_a = plot_a."f(x) ls 3 t 'Fit aumento', "
plot_a = plot_a."g(x) ls 4 t 'Fit decremento'"

p[:] @plot_a
