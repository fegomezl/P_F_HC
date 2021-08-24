#include "catch2/catch.hpp"
#include "header.h"


TEST_CASE ("Condition tests"){

    
    int pid = 0, nproc = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    //Creacion de tipos de variable para MPI
    MPI_Datatype vector_type;  //Se crea el tipo de variable vector3D para MPI
    int lenghts_1[1] = {3};
    MPI_Aint displacements_1[1] = {0};
    MPI_Datatype types_1[1] = {MPI_DOUBLE};
    MPI_Type_create_struct(1, lenghts_1, displacements_1, types_1, &vector_type);
    MPI_Type_commit(&vector_type);
    
    MPI_Datatype body_type;  //Se crea el tipo de variable body para MPI
    int lenghts_2[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
    MPI_Aint displacements_2[9] = {offsetof(body, mass), offsetof(body, rad), offsetof(body, I),
                                    offsetof(body, pos), offsetof(body, vel), offsetof(body, force),
                                    offsetof(body, angle), offsetof(body, omega), offsetof(body, torque)};
    MPI_Datatype types_2[9] = {MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE,
                               vector_type, vector_type, vector_type,
                               vector_type, vector_type, vector_type};
    MPI_Type_create_struct(9, lenghts_2, displacements_2, types_2, &body_type);
    MPI_Type_commit(&body_type);

    MPI_Datatype trap_type;  //Se crea el tipo de variable trap para MPI
    int lenghts_3[4] = {1, 1, 1, 1};
    MPI_Aint displacements_3[4] = {offsetof(trap, force), offsetof(trap, rad), offsetof(trap, active),
                                   offsetof(trap, pos)};
    MPI_Datatype types_3[4] = {MPI_DOUBLE, MPI_DOUBLE, MPI_INT,
                               vector_type};
    MPI_Type_create_struct(4, lenghts_3, displacements_3, types_3, &trap_type);
    MPI_Type_commit(&trap_type);

    CONFIG config;
    domain section;
    MPI_Barrier(MPI_COMM_WORLD);

    init_data(pid, config, section);  //Se leen las condiciones de la simulación
    config.iterations = config.iterations/5;  //Se hace la quinta parte de las iteraciones
    start(pid, config, section);  //Se establecen las condiciones iniciales de la simulacion
    

    std::mt19937 gen(config.seed+pid);
    std::normal_distribution<double> desv_theta(0,config.desv_theta);
    std::normal_distribution<double> desv_phi(0,config.desv_phi);
    
    SECTION ("Numero de particulas constante"){
        int n_count=0, nt=config.nt + 1, aux=0;
        if(pid == 0) std::cout << "Test: Numero de particulas constante" << std::endl;
        for (int ii = 1; ii < config.iterations; ii++){
            aux=0;
            time_step(pid, config, section, desv_theta, desv_phi, gen, body_type, trap_type); //Evoluciona el sistema un paso de tiempo dt
            MPI_Reduce(&section.local_size, &aux, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
            if(nt != aux) n_count++;
        }

        if(pid == 0) CHECK (n_count == 0);
    }

    SECTION ("Pelotas estan adentro de la caja"){
        int lx_count=0, ly_count=0;
        if(pid == 0) std::cout << "Test: Pelotas dentro de la caja" << std::endl;
        for(int ii = 1; ii < config.iterations; ii++){
            time_step(pid, config, section, desv_theta, desv_phi, gen, body_type, trap_type); //Evoluciona el sistema un paso de tiempo dt
            if(pid==0){
                for(auto j: section.balls){
                    if(j.pos.x() < -4*j.rad) lx_count++;
                    if(j.pos.y() < -4*j.rad || j.pos.y() > config.box_size+4*j.rad) ly_count++;
                }
            }
            else if(pid < config.nproc-1){
                for(auto j: section.balls){
                    if(j.pos.y() < -4*j.rad || j.pos.y() > config.box_size+4*j.rad) ly_count++;
                }
            }
            else if(pid == config.nproc-1){
                for(auto j: section.balls){
                    if(j.pos.y() < -4*j.rad || j.pos.y() > config.box_size+4*j.rad) ly_count++;
                    if(j.pos.x() > config.box_size+4*j.rad) lx_count++;
                }
            }
        }

        INFO("Fallo en proceso: " << pid);
        CHECK(lx_count == 0);
        CHECK(ly_count == 0);
    }

    SECTION("Energía siempre positiva"){
        int e_count=0;
        double energy_sum;
        if(pid==0) std::cout << "Test: Energia siempre positiva (sin overflow)" << std::endl;
        for(int ii = 1; ii < config.iterations; ii++){
            energy_sum=0.0;
            time_step(pid, config, section, desv_theta, desv_phi, gen, body_type, trap_type); //Evoluciona el sistema un paso de tiempo dt
            for (auto b: section.balls) section.energy += (0.5*b.mass*norma2(b.vel) + 0.5*b.I*norma2(b.omega) + b.mass*b.pos.z()*config.g)*std::pow(10,-6);
            MPI_Reduce(&section.energy, &energy_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
            if(pid==0 && (energy_sum<0 || isnan(energy_sum))) e_count++;
        }
        if(pid==0) CHECK(e_count==0);
    }

    SECTION("Envios de primer y ultima particula"){
        int count=0;
        if(pid==0) std::cout << "Test: Envios del primer y ultimo proceso" << std::endl;
        for(int ii = 1; ii < config.iterations; ii++){
            time_step(pid, config, section, desv_theta, desv_phi, gen, body_type, trap_type); //Evoluciona el sistema un paso de tiempo dt
            if(pid==0 && (section.traps_send_down!=0 || section.traps_recv_down!=0)) count++;
            if(pid==config.nproc-1 && (section.traps_send_up!=0 || section.traps_recv_up!=0)) count++;
        }
        INFO("Fallo en proceso: " << pid);
        CHECK(count == 0);
    }
}
