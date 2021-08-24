CXX_P = mpic++
CXXFLAGS = -std=c++17 -O3 -g
DEPENDENCIES = objects/init_data.o objects/start.o objects/print.o objects/ghost.o objects/compute_force.o objects/time_step.o

DT=$(shell sed -n 10p data/init_data.txt | tr -d -c 0-9.)
ITERATIONS=$(shell sed -n 11p data/init_data.txt | tr -d -c 0-9)
RESOLUTION=$(shell sed -n 12p data/init_data.txt | tr -d -c 0-9)
NPROC=$(shell sed -n 25p data/init_data.txt | tr -d -c 0-9)

DELAY=$(shell echo "100*$(RESOLUTION)*$(DT)" | bc)
T=$(shell echo "$(ITERATIONS)/$(RESOLUTION)-1" | bc)

all: main.x

mpi: main.x
	rm -f data/system_parameters/*.csv data/visual/*.csv
	mpirun -np $(NPROC) --oversubscribe ./$<
	bash scripts/print_system.sh

scaling: main.x
	rm -f data/system_parameters/*.csv data/visual/*.csv data/scaling/runtimes-*.txt data/scaling/metrics-*.txt
	bash scripts/scaling.sh

test: test.x
	mpirun -np $(NPROC) --oversubscribe ./$<

system_graphs:
	gnuplot scripts/plot_system_state.gp
	gnuplot scripts/plot_start.gp
	xpdf data/energy.pdf &
	xpdf data/activation_partial.pdf &
	xpdf data/activation_total.pdf &
	xpdf data/start.pdf &

scaling_graphs:
	gnuplot scripts/plot_scaling.gp
	xpdf data/speed_up.pdf &
	xpdf data/parallel_efficiency.pdf &

distribution:
	rm -f data/distribution/*.png
	gnuplot scripts/plot_distribution.gp > /dev/null 2>&1
	convert -delay $(DELAY) -loop 1 data/distribution/distribution-{0..$(T)}.png data/distribution.gif
	geeqie data/distribution.gif &

animation:
	rm -f data/animation/*.png
	gnuplot scripts/plot_animation.gp
	convert -delay $(DELAY) -loop 1 data/animation/animation-{0..$(T)}.png data/animation.gif
	geeqie data/animation.gif &

send:
	rm -f /media/sf_Data_machine/visual/*.csv
	cp -r data/visual/ /media/sf_Data_machine/
	cp data/*pdf data/*.gif /media/sf_Data_machine/

main.x: $(DEPENDENCIES) objects/main.o 
	$(CXX_P) $(CXXFLAGS) $^ -o $@

test.x: $(DEPENDENCIES) objects/test.o objects/test2.o objects/catch_mpi_test_main.o
	$(CXX_P) $(CXXFLAGS) $^ -o $@

objects/%.o: code_parallel/%.cpp
	$(CXX_P) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f data/visual/*.csv data/system_parameters/*.csv data/distribution/*.png data/animation/*.png *.x

oclean:
	rm -f objects/*.o
