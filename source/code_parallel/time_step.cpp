#include "header.h"

//Constantes para la integracion con PEFRL de orden 4, la organizacion de los numeros es el orden de su uso
const double const1 = 0.1786178958448091;   //Xi
const double const3 = -0.06626458266981849; //Ji
const double const4 = -0.2123418310626054;  //Lambda
const double const2 = (1-const4)/2;         //(1-Lambda)/2
const double const5 = 1-2*(const3+const1);  //1+2*(Xi+Ji)

void time_step (const int &pid, const CONFIG &config, domain &section, std::normal_distribution<double> &desv_theta, std::normal_distribution<double> &desv_phi, std::mt19937 &gen, MPI_Datatype body_type, MPI_Datatype trap_type){
    //Genera un paso de tiempo en el sistema

    vector3D dr;           //Posicion relativa entre pelotas y trampas

    double norm_dr = 0;    //Distancia relativa entre pelotas
    double h = 0;          //Deformaciones
    double v_t = 0;        //Velocidad radial de la pelota
    double v_0 = 0;        //Velocidad de activación

    //Comparte info entre procesos de pelotas cercanas a la frontera
    if (config.nproc != 1) ghost (pid, config, section, body_type, trap_type);
    
    //Verifica si alguna trampa se prende
    for (int ii = 0; ii < section.local_trap_size; ii++){
        for (auto& b: section.balls){
            dr = b.pos-section.traps[ii].pos;
            norm_dr = norma(dr);
            h = b.rad + section.traps[ii].rad/2 - norm_dr;
            if (h > config.err) {
                v_t = -(b.vel*dr)/norm_dr;
                v_0 = std::sqrt(2*config.g*section.traps[ii].rad);
                if (v_t >= v_0){
                    section.traps[ii].active = 1;
                    break;
                }
            }
        }
    }
    
    //Computa la fuerza, modifica la posicion y la velocidad con PEFRL orden 4
    for (int ii = 0; ii < section.local_size; ii++){
        section.balls[ii].pos += (config.dt*const1)*section.balls[ii].vel;
        section.balls[ii].angle += (config.dt*const1)*section.balls[ii].omega;
    }
    if (config.nproc != 1) ghost (pid, config, section, body_type, trap_type);
    compute_force(config, section, desv_theta, desv_phi, gen);
    for (int ii = 0; ii < section.local_size; ii++){
        section.balls[ii].vel += (config.dt*const2/section.balls[ii].mass)*section.balls[ii].force;
        section.balls[ii].omega += (config.dt*const2/section.balls[ii].mass)*section.balls[ii].torque;
        section.balls[ii].pos += (config.dt*const3)*section.balls[ii].vel;
        section.balls[ii].angle += (config.dt*const3)*section.balls[ii].omega;
    }
    if (config.nproc != 1) ghost (pid, config, section, body_type, trap_type);
    compute_force(config, section, desv_theta, desv_phi, gen);
    for (int ii = 0; ii < section.local_size; ii++){
        section.balls[ii].vel += (config.dt*const4/section.balls[ii].mass)*section.balls[ii].force;
        section.balls[ii].omega += (config.dt*const4/section.balls[ii].mass)*section.balls[ii].torque;
        section.balls[ii].pos += (config.dt*const5)*section.balls[ii].vel;
        section.balls[ii].angle += (config.dt*const5)*section.balls[ii].omega;
    }
    if (config.nproc != 1) ghost (pid, config, section, body_type, trap_type);
    compute_force(config, section, desv_theta, desv_phi, gen);
    for (int ii = 0; ii < section.local_size; ii++){
        section.balls[ii].vel += (config.dt*const4/section.balls[ii].mass)*section.balls[ii].force;
        section.balls[ii].omega += (config.dt*const4/section.balls[ii].mass)*section.balls[ii].torque;
        section.balls[ii].pos += (config.dt*const3)*section.balls[ii].vel;
        section.balls[ii].angle += (config.dt*const3)*section.balls[ii].omega;
    }
    if (config.nproc != 1) ghost (pid, config, section, body_type, trap_type);
    compute_force(config, section, desv_theta, desv_phi, gen);
    for (int ii = 0; ii < section.local_size; ii++){
        section.balls[ii].vel += (config.dt*const2/section.balls[ii].mass)*section.balls[ii].force;
        section.balls[ii].omega += (config.dt*const2/section.balls[ii].mass)*section.balls[ii].torque;
        section.balls[ii].pos += (config.dt*const1)*section.balls[ii].vel;
        section.balls[ii].angle += (config.dt*const1)*section.balls[ii].omega;
    }

    double space = config.box_size/(1.0*config.ncells_per_line);  //Espacio entre los centros de las celdas
    int x = 0, y = 0;  //Celdas de la trampa activada
    int index = 0;

    //Quita y cuenta las trampas activadas
    section.traps.erase(std::remove_if(section.traps.begin(), section.traps.begin() + section.local_trap_size,
                                       [config, space, &x, &y, &index, &section](trap trap_i){
                                           index++;
                                           if (trap_i.active == 1){ //Si la trampa esta activa
                                               section.local_trap_size--;  //Reduce el numero de trampas locales
                                               x = std::floor(trap_i.pos.x()/space);    //Verifica en que celda esta la particula|
                                               y = std::floor(trap_i.pos.y()/space);
                                               section.activation[y + config.ncells_per_line*x] += 1;  //Aumenta el numero de activaciones en la celda correspondiente
                                               section.activation_partial_sum += 1;  //Aumenta el numero de activaciones en la seccion
                                               if (trap_i.send_up == 1) section.traps_send_up--;  //Informa si la trampa removida se compartia al procesador posterior
                                               else if (trap_i.send_down == 1) section.traps_send_down--;  //Informa si la trampa removida se compartia al procesador anterior
                                               return true;
                                           } else {return false;}
                                       })
                        ,section.traps.begin()+section.local_trap_size
    );
    
    int tag = 4;

    if (config.nproc != 1){
        if (pid == 0){
            MPI_Send(&section.traps_send_up, 1, MPI_INT, pid+1, tag, MPI_COMM_WORLD);
            MPI_Recv(&section.traps_recv_up, 1, MPI_INT, pid+1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else if (pid < config.nproc-1){
            MPI_Recv(&section.traps_recv_down, 1, MPI_INT, pid-1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(&section.traps_send_up, 1, MPI_INT, pid+1, tag, MPI_COMM_WORLD);
            MPI_Recv(&section.traps_recv_up, 1, MPI_INT, pid+1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(&section.traps_send_down, 1, MPI_INT, pid-1, tag, MPI_COMM_WORLD);
        }
        else {
            MPI_Recv(&section.traps_recv_down, 1, MPI_INT, pid-1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(&section.traps_send_down, 1, MPI_INT, pid-1, tag, MPI_COMM_WORLD);
        }
    }
}
