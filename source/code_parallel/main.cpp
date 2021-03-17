#include "header.h"

int main (int argc, char **argv){

    MPI_Init(&argc, &argv);
    
    int pid = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);

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
    int lenghts_3[6] = {1, 1, 1, 1, 1, 1};
    MPI_Aint displacements_3[6] = {offsetof(trap, force), offsetof(trap, rad), offsetof(trap, active),
                                   offsetof(trap, send_up), offsetof(trap, send_down),
                                   offsetof(trap, pos)};
    MPI_Datatype types_3[6] = {MPI_DOUBLE, MPI_DOUBLE, MPI_INT,
                               MPI_INT, MPI_INT,
                               vector_type};
    MPI_Type_create_struct(6, lenghts_3, displacements_3, types_3, &trap_type);
    MPI_Type_commit(&trap_type);

    CONFIG config;
    domain section;

    MPI_Barrier(MPI_COMM_WORLD);
    if (pid == 0) std::cout << "\nIntitializing simulation...\n";
    double begin = MPI_Wtime();

    init_data(pid, config, section);  //Se leen las condiciones de la simulación
    start(pid, config, section);  //Se establecen las condiciones iniciales de la simulacion
    print(0, pid, config, body_type, trap_type, section);  //Se imprime el estado inicial

    std::mt19937 gen(config.seed+pid);
    std::normal_distribution<double> desv_theta(0,config.desv_theta);
    std::normal_distribution<double> desv_phi(0,config.desv_phi);

    for (int ii = 1; ii < config.iterations; ii++){
        time_step(pid, config, section, desv_theta, desv_phi, gen, body_type, trap_type); //Evoluciona el sistema un paso de tiempo dt
        if (ii % config.resolution == 0){
            print(ii/config.resolution, pid, config, body_type, trap_type, section); //Imprime el estado del sistema cada resolution
            std::cout << (100*ii)/config.iterations << "% from pid " << pid << ".\r";
            std::cout.flush();
        }
    }
    
    double time = MPI_Wtime() - begin;
    MPI_Barrier(MPI_COMM_WORLD);
    if (pid == 0) std::cout << "\n100%! Simulation completed!\n\n";

    double sumtime = 0;
    MPI_Reduce(&time, &sumtime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (pid == 0){
        double avgtime = sumtime/config.nproc;
        std::string fname = "data/scaling/runtimes-" + std::to_string(config.nt) + ".txt";
        std::fstream fs(fname, std::ios_base::app);
        
        fs << config.nt << "\t"
           << std::fixed << std::setprecision(12) << avgtime << "\n";
      
        fs.close();
    }
    
    MPI_Type_free(&vector_type);
    MPI_Type_free(&body_type);
    MPI_Type_free(&trap_type);
    MPI_Finalize();
    return 0;
}

