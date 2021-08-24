set g
set ls 1 lc rgb "blue" lt 1 lw 0.5
set ls 2 lc rgb "red" lt 1 lw 0.5
set ls 3 lc rgb "orange" lt 1 lw 0.5

file = "data/results/system_state_25.txt"
n = 10000

box_size = system("sed -n 1p data/init_data.txt | tr -d -c 0-9")*1
nt = int(system("sed -n 3p data/init_data.txt | tr -d -c 0-9"))
rad_t = system("sed -n 6p data/init_data.txt | tr -d -c 0-9.")*1
densidad = 4*(rad_t*rad_t*nt)/(box_size*box_size)

Init = sprintf(" {/:Bold Densidad:} %.2f", densidad)
set obj 1 rect from graph 0, 1 to graph 0.2, 0.93 fc rgb "white"
set lab 1 Init at graph 0, 0.96

set xl 'Tiempo (s)'
set term pdf
set key opaque
set fit errorvar
set fit quiet

cgamma_d(x) = igamma(r,x*lamda)
lamda = 0.8
r = 17
fit cgamma_d(x) file u 1:4 via lamda,r
xmax = r/lamda
xmax_err = xmax*(lamda_err/lamda+r_err/r)

Fit = sprintf(" x ≅ Gamma(r,λ) \n r ≅ %.1f ± %.1f \n λ ≅ (%.3f ± %.3f) 1/s \n t_{esperado} = (%.1f ± %.1f) s", r, r_err, lamda, lamda_err, xmax, xmax_err)

set arrow 1 from xmax, 0 to xmax, 1 nohead
set obj 2 rect from graph 0.67, 0.85 to graph 1, 0.6 fc rgb "white" front
set lab 2 Fit at graph 0.67, 0.81 front

set title 'Porcentaje de activaciones acumuladas vs tiempo'
set yl 'Porcentaje de trampas activadas'
set o 'data/activation_total.pdf'
p[0:60][0:1] file u 1:4 w l ls 1 t 'Activaciones', cgamma_d(x) ls 2 t "Fit"  

gamma_d(x) = lamda*exp(-lamda*x)*((lamda*x)**(r-1))/gamma(r)
fit gamma_d(x) file u 1:3 via lamda,r

Fit = sprintf(" x ≅ Gamma(r,λ) \n r ≅ %.1f ± %.1f \n λ ≅ (%.2f ± %.2f) 1/s", r, r_err, lamda, lamda_err)

unset arrow 1
set obj 2 rect from graph 0.67, 0.85 to graph 1, 0.65 fc rgb "white" front
set lab 2 Fit at graph 0.67, 0.81 front

set title 'Porcentaje de activaciones vs tiempo'
set yl 'Porcentaje de trampas activadas'
set o 'data/activation_partial.pdf'
p[0:60][0:0.12] file u 1:3 w l ls 1 t 'Activaciones', gamma_d(x) ls 2 t "Fit"

Egamma(x) = E*igamma(r,x*lamda)
E = 0.02
fit[0:xmax] Egamma(x) file u 1:2 via E,lamda,r

u = 1000000*E*lamda/n
u_err = u*(E_err/E+lamda_err/lamda)

Fit = sprintf(" x = E*IGamma_{(r,λ)}(t) \n E ≅ (%.3f ± %.3f) MJ \n λ ≅ %.1f ± %.1f \n λE ≅ (%.4f ± %.4f) J/s", E, E_err, lamda, lamda_err, u, u_err)

set obj 2 rect from graph 0, 0.93 to graph 0.33, 0.67 fc rgb "white" front
set lab 2 Fit at graph 0, 0.89 front

Eexp(x) = a*exp(b*x)
a = 0.012
b = -0.01
c = Egamma(xmax)
fit[xmax:][0:0.9*c] Eexp(x) file u 1:2 via a,b

u = 1000000*a*b/n
u_err = u*(a_err/a+b_err/b)

Fit = sprintf(" x = a*exp(b*x) \n a ≅ (%.4f ± %.4f) MJ \n b ≅ (%.5f ± %.5f) 1/s \n λE ≅ (%.4f ± %.4f) J/s", a, a_err, b, b_err, u, u_err)

set obj 3 rect from graph 0.64, 0.80 to graph 1, 0.54 fc rgb "white" front
set lab 3 Fit at graph 0.64, 0.76 front

set title 'Energia vs tiempo'
set yl 'Energía (MJ)'
set o 'data/energy.pdf'
p[0:][0:0.025] file u 1:2 w l ls 1 t 'Energía', Egamma(x) ls 2 t "Fit gamma", Eexp(x) ls 3 t "Fit exp"
