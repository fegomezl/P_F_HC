#include "header.h"

void print(const int &time, const int &pid, const CONFIG &config,
           MPI_Datatype body_type, MPI_Datatype trap_type, domain &section){
    //Imprimir el estado del sistema en un momento específico

    //Procesadores que imprimen
    int pid_0 = time%config.nproc;

    double energy_sum = 0;  //Suma de la energía mecanica
    double activation_sum = 0;  //Numero de activaciones parcial
    double activation_acc_sum = 0;  //Numero de activaciones acumulado

    //Calcula la energia mecanica de cada seccion
    for (int ii = 0; ii < section.local_size; ii++){
        //Energia mecanica por particula
        section.energy += (0.5*section.balls[ii].mass*norma2(section.balls[ii].vel) + 0.5*section.balls[ii].I*norma2(section.balls[ii].omega) 
                        + section.balls[ii].mass*section.balls[ii].pos.z()*config.g)*std::pow(10,-6);

        //Energia mecanica de interaccion con las paredes
        /*double h, H; //Distancias de interpenetracion con las paredes

        //Fuerza con las paredes x=0 y x=Lx
        h = section.balls[ii].rad - section.balls[ii].pos.x();  
        H = section.balls[ii].rad + section.balls[ii].pos.x() - config.box_size;
        if (h > config.err)
            section.energy += (2*config.k_wall/5)*std::pow(h, 2.5);
        else if (H > config.err)
            section.energy += (2*config.k_wall/5)*std::pow(H, 2.5);
       
        //Fuerza con las paredes y=0 y y=Ly
        h = section.balls[ii].rad - section.balls[ii].pos.y();  
        H = section.balls[ii].rad + section.balls[ii].pos.y() - config.box_size;
        if (h > config.err)
            section.energy += (2*config.k_wall/5)*std::pow(h, 2.5);
        else if (H > config.err)
            section.energy += (2*config.k_wall/5)*std::pow(H, 2.5);
       
        //Fuerza con las paredes z=0 y z=Lz
        h = section.balls[ii].rad - section.balls[ii].pos.z();  
        H = section.balls[ii].rad + section.balls[ii].pos.z() - config.box_size_z;
        if (h > config.err)
            section.energy += (2*config.k_wall/5)*std::pow(h, 2.5);
        else if (H > config.err)
            section.energy += (2*config.k_wall/5)*std::pow(H, 2.5);*/

        //Energia mecanica de interaccion entre particulas
        /*for (int jj = ii + 1; jj < section.balls.size(); jj++){
            double r = norma(section.balls[ii].pos - section.balls[jj].pos);
            double S = section.balls[ii].rad + section.balls[jj].rad - r;
            if (S > config.err){
                double a = 1.0;
                if (jj >= section.local_size) a = 0.5; //Si una de las particulas es compartida, la energia se divide a la mitad para tener en cuenta los aportes de distintas secciones
                section.energy += a*(2*config.k_ball/5)*std::pow(S, 2.5);
            }
        }*/
    }

    //Actualiza las activaciones acumuladas de la seccion
    section.activation_total_sum += section.activation_partial_sum;

    //Suma la cantidad de energía y activaciones totales del sistema
    MPI_Reduce(&section.energy, &energy_sum, 1, MPI_DOUBLE, MPI_SUM, pid_0, MPI_COMM_WORLD);
    MPI_Reduce(&section.activation_partial_sum, &activation_sum, 1, MPI_DOUBLE, MPI_SUM, pid_0, MPI_COMM_WORLD);
    MPI_Reduce(&section.activation_total_sum, &activation_acc_sum, 1, MPI_DOUBLE, MPI_SUM, pid_0, MPI_COMM_WORLD);
    
    if (pid == pid_0){  //Datos globales del sistema
        //Promedia el numero de activaciones parcial
        activation_sum /= 1.0*config.resolution*config.dt;
        //Imprime los datos
        std::string fname = "data/results/data_system-" + std::to_string(time) + ".csv";
        std::ofstream fout(fname);
        fout << energy_sum << "\t"
             << activation_sum/config.nt << "\t"
             << activation_acc_sum/config.nt << "\n";
        fout.close();
    }

    //Re-inicializa la energia y el numero de activaciones parciales
    section.energy = 0; 
    section.activation_partial_sum = 0;
}
