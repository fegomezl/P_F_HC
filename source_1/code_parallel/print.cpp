#include "header.h"

void print(const int &time, const int &pid, const CONFIG &config,
           MPI_Datatype body_type, MPI_Datatype trap_type, domain &section){
    //Imprimir el estado del sistema en un momento específico

    //Procesadores que imprimen
    int pid_0 = time%config.nproc;
    int pid_1 = (time+1)%config.nproc;
    int pid_2 = (time+2)%config.nproc;
    int pid_3 = (time+3)%config.nproc;

    double energy_sum = 0;  //Suma de la energía mecanica
    double activation_sum = 0;  //Numero de activaciones parcial
    double activation_acc_sum = 0;  //Numero de activaciones acumulado
    int ncells = config.ncells_per_line*config.ncells_per_line;  //Numero de celdas
    int nt_actual = 0;  //Numero de trampas actual

    //Vector de activaciones del sistema por celda
    Vec_d activation_global(ncells,0);

    //Arreglos de cantidad de elementos y desplazamientos para mpi_gatherv
    int balls_size[config.nproc] {};
    int balls_disp[config.nproc] {};
    int traps_size[config.nproc] {};
    int traps_disp[config.nproc] {};

    //Eliminar informacion compartida
    section.balls.resize(section.local_size); 
    section.traps.resize(section.local_trap_size);
    //Calcula la energía mecanica y activaciones acumuladas de la sección
    for (auto b: section.balls) section.energy += (0.5*b.mass*norma2(b.vel) + 0.5*b.I*norma2(b.omega) + b.mass*b.pos.z()*config.g)*std::pow(10,-6);
    section.activation_total_sum += section.activation_partial_sum;

    //Suma la cantidad de energía y activaciones totales del sistema
    MPI_Reduce(&section.energy, &energy_sum, 1, MPI_DOUBLE, MPI_SUM, pid_0, MPI_COMM_WORLD);
    MPI_Reduce(&section.activation_partial_sum, &activation_sum, 1, MPI_DOUBLE, MPI_SUM, pid_0, MPI_COMM_WORLD);
    MPI_Reduce(&section.activation_total_sum, &activation_acc_sum, 1, MPI_DOUBLE, MPI_SUM, pid_0, MPI_COMM_WORLD);
    MPI_Reduce(section.activation.data(), activation_global.data(), ncells, MPI_DOUBLE, MPI_SUM, pid_1, MPI_COMM_WORLD);

    //Encuentra el numero de elementos a recibir por proceso
    MPI_Gather(&section.local_size, 1, MPI_INT, balls_size, 1, MPI_INT, pid_2, MPI_COMM_WORLD);
    MPI_Gather(&section.local_trap_size, 1, MPI_INT, traps_size, 1, MPI_INT, pid_3, MPI_COMM_WORLD);
    
    //Encuentra el arreglo de desplazamientos y el numero total de trampas
    if (pid == pid_2){
        for (int ii = 1; ii < config.nproc; ii++){
            balls_disp[ii] = balls_disp[ii-1]+balls_size[ii-1];
        }
    }
    if (pid == pid_3){
        nt_actual += traps_size[0];
        for (int ii = 1; ii < config.nproc; ii++){
            traps_disp[ii] = traps_disp[ii-1]+traps_size[ii-1];
            nt_actual += traps_size[ii];
        }
    }
    
    //Vectores que reciven la informacion
    Vec_b balls_recv(config.nt + 1);
    Vec_t traps_recv(nt_actual);

    //Recibir la informacion a imprimir
    MPI_Gatherv(section.balls.data(), section.local_size, body_type, balls_recv.data(), balls_size, balls_disp, body_type, pid_2, MPI_COMM_WORLD);
    MPI_Gatherv(section.traps.data(), section.local_trap_size, trap_type, traps_recv.data(), traps_size, traps_disp, trap_type, pid_3, MPI_COMM_WORLD);
    
    if (pid == pid_0){  //Datos globales del sistema
        //Promedia el numero de activaciones parcial
        activation_sum /= 1.0*config.resolution*config.dt;
        //Imprime los datos
        std::string fname_1 = "data/system_parameters/data_system-" + std::to_string(time) + ".csv";
        std::ofstream fout_1(fname_1);
        fout_1 << energy_sum << "\t"
               << activation_sum/config.nt << "\t"
               << activation_acc_sum/config.nt << "\n";
        fout_1.close();
    }
    if (pid == pid_1){  //Datos de activaciones por celda
        double space = config.box_size/(1.0*config.ncells_per_line);  //Espacio entre los centros de las celdas
        double origin = space/2.0;  //Centro de una celda
        double x = 0, y = 0;  //Posiciones de los centros de las celdas
        int index = 0;  //Indice de iteracion
        //Imprime los datos
        std::string fname_2 = "data/system_parameters/data_activation-" + std::to_string(time) + ".csv";
        std::ofstream fout_2(fname_2);
        for (auto a: activation_global){
            x = origin + (index/config.ncells_per_line)*space;
            y = origin + (index%config.ncells_per_line)*space;
            fout_2 << x << "\t"
                   << y << "\t"
                   << a/config.nt << "\n";
            index++;
        }
        fout_2.close();
    }
    if (pid == pid_2){  //Datos de las pelotas
        //Imprime los datos
        std::string fname_3 = "data/visual/data_balls-" + std::to_string(time) + ".csv"; 
        std::ofstream fout_3(fname_3);
        for (auto b: balls_recv){   
            fout_3 << b.pos.x() << "\t"
                   << b.pos.y() << "\t"
                   << b.pos.z() << "\t"
                   << b.rad << "\n";
        }
        fout_3.close();
    }
    if (pid == pid_3){  //Datos de las trampas
        //Imprime los datos
        std::string fname_4 = "data/visual/data_traps-" + std::to_string(time) + ".csv";  
        std::ofstream fout_4(fname_4);
        for (auto t: traps_recv){ 
            fout_4 << t.pos.x() << "\t"
                   << t.pos.y() << "\t"
                   << t.pos.z() << "\t"
                   << t.rad << "\n";
        }
        fout_4.close();
    }
    section.activation_partial_sum = 0; //Inicializa la cantidad de activaciones parciales
    section.energy = 0; //Inicializa la energía
}
