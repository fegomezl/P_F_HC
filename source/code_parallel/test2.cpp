#include "catch2/catch.hpp"
#include "header.h"


TEST_CASE ("Move tests"){
    
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
    double seg_iterations=1.0/config.dt;
    
    std::mt19937 gen(config.seed+pid);
    std::normal_distribution<double> desv_theta(0,config.desv_theta);
    std::normal_distribution<double> desv_phi(0,config.desv_phi);
    
    SECTION("Perdida de altura al rebotar"){
        int condition=0;
        if(pid==0) std::cout << "Test: Rebote de pelotas" << std::endl;
        config.nt=0;
        section.local_size=0;
        section.local_trap_size=0;
        start(pid, config, section);
        std::cout << "pelotas " << section.balls.size() << std::endl;
        for(int ii=0; ii<10*seg_iterations; ii++){
            time_step(pid, config, section, desv_theta, desv_phi, gen, body_type, trap_type); //Evoluciona el sistema un paso de tiempo dt
            if(section.balls.size()!=0){
                if(section.balls[0].pos.z() >= config.z_init){
                condition=1;
                break;
                }
            }
        }
        INFO("La pelota rebota mas alto que su punto de partida en proceso: " << pid << "\n altura: " << section.balls[0].pos.z());
        CHECK (condition==0);
    }
}
