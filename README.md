# Proyecto Final Herramientas Computacionales
---
## Integrantes:
Andrés Felipe Vargas Londoño

Carlos Andres del Valle Urberuaga

Felipe Gómez Lozada

Julián David Jiménez Paz

---

## Uso:

#### make:
El comando make se encarga de compilar todos los archivos (.CPP) necesarios
para la simulacion en archivos objeto en el directorio objects, luego los
compila para crear el ejecutable main.x Todos los archivos se compilan usando
mpic++ y las banderas especificadas, en este caso -std=c++17 -O3 -g.

#### make mpi:
Este comando elimina los resultados que existan en data/system_parameters y data/visual. Revisa que exista main.x y corre la simulacion con mpirun para el
numero de procesadores especificado en init_data.txt. Corre el script de bash
print_system.sh.

#### make scaling:
Revisa que exista main.x, elimina los resultados de los directorios
data/system_parameters, data/sacaling y data/visual. Luego corre el script de
bash scaling.sh

#### make test:
Revisa que exista el ejecutable test.x y lo ejecuta usando mpirun y el numero de
procesadores especificado en init_data.txt

#### make system_graps:
Ejecuta los scripts de genuplot plot_system_state.gp, plot_start.gp y muestra
los resultados energy.pdf, activation_partial.pdf, activation_total.pdf y
start.pdf usando xpdf.

#### make scaling_graphs:
Ejecuta el script de gnuplot plot_scaling.gp y muestra los resultados
speed_up.pdf y parallel_efficiency.pdf usando xpdf.

#### make distribution:
Elimina los resultados que haya en data/distribution. Ejecuta el script de
gnuplot plot_distribution.gp y usa convert para convertir la salida (imagenes
.png) en distribution.gif

#### make animaiton:
Elimina los resultados que haya en data/animation, corre el script de gnuplot
plot_animation.gp y convierte la salida (imagenes .png) en animation.gif

#### make send:
**Importante:** Este target se usa si se corre la simulacion en una maquina
virtual y se encarga de enviar los archivos de data/visual (archivos.csv) a la
carpeta compartida entre la maquina virtual y el sistema host. Para usarlo
correctamente hay que cambiar el directorio "/media/sf_Data_machine" de los
comandos por la dirección de la carpeta compartida respectiva.

#### make clean:
Elimina los archivos .csv que haya en data/visual y data/system_parameters, los
archivos. png en data/distribution y data/atctivation y los archivos .x

#### make oclean:
elimina los archivos objeto (.o) en la carpeta objects

## Parametros de funcionamiento:

Para correr la simulacion se debe tener cuidado con ciertas condiciones
iniciales que permiten que el sistema funcione correctamente. Por fuera de estos
rangos, pueden encontrarse problemas de ejecución y problemas con los resultados
de la simulacion, por ejemplo, overflow en la energia.
El parametro **dt**
optimo es del orden de 10^-3. Los parametros **k_wall** y **k_ball** no deberian
exceder 10000. El sistema se ha probado para un **nt** de hasta 10000 pelotas,
mas alla de eso no se garantiza un funcionamiento correcto. Si **box_size_z**
es demasiado pequeño, puede que las particulas se salgan de la caja y el
programa falle.
