NPROC=$(shell sed -n 25p data/init_data.txt | tr -d -c 0-9)
OUT=/media/sf_Archdata/

#Compiling parameters
CXX = mpic++
FLAGS = -std=c++17 -O3 -g
RUN = mpirun -np $(NPROC) --oversubscribe ./
SOURCES = $(wildcard code/*.cpp)
DEPENDENCIES = $(SOURCES:code/%.cpp=.objects/%.o)

.PHONY: all run graph show send clean oclean

all: main.x
	@echo 'Program Compiled.'

run: main.x
	@echo -e 'Running program...'
	@$(RUN)$<
	@echo -e 'Done!'

graph:
	@echo -e 'Ploting system variables... \c'
	@bash scripts/print_system.sh
	@gnuplot scripts/plot_system_state.gp
	@echo -e 'Done!'

show:
	@xpdf data/energy.pdf &
	@xpdf data/activation_partial.pdf &
	@xpdf data/activation_total.pdf &

send:
	@cp data/*pdf $(OUT)

main.x: $(DEPENDENCIES)
	@echo -e 'Compiling' $@ '... \c'
	@$(CXX) $(FLAGS) $^ -o $@
	@echo -e 'Done!\n'

.objects/%.o: code/%.cpp
	@echo -e 'Building' $@ '... \c'
	@$(CXX) $(FLAGS) -c $< -o $@
	@echo -e 'Done!\n'

clean:
	@rm -f data/*.pdf data/results/*.csv data/results/*.txt *.x

oclean:
	@rm -f .objects/*.o
