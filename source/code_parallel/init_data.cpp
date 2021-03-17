#include "header.h"

void init_data(const int &pid, CONFIG &config, domain &section){

    std::string line;
    std::ifstream init_data;
    init_data.open("data/init_data.txt");

    std::getline(init_data, line);
    config.box_size = std::stod(line.erase(line.find('#'), line.size()));
    
    std::getline(init_data, line);
    config.box_size_z = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.nt = std::stoi(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.ncells_per_line = std::stoi(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.seed = std::stoi(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.rad_t = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.rad_b = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.mass = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.z_init = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.dt = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.iterations = std::stoi(line.erase(line.find('#'), line.size()) );

    std::getline(init_data, line);
    config.resolution = std::stoi(line.erase(line.find('#'), line.size()) );

    std::getline(init_data, line);
    config.err = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.g = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.k_wall = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.k_ball = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.v_trap = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.gamma_wall = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.gamma_ball = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.mu_wall = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.mu_ball = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.mu_trap = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.desv_theta = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.desv_phi = std::stod(line.erase(line.find('#'), line.size()));

    std::getline(init_data, line);
    config.nproc = std::stoi(line.erase(line.find('#'), line.size()) );
    
    init_data.close();
    
    body ball_0;  //Se crea una trampa base
    trap trap_0;  //Se crea una pelota base
    
    trap_0.rad = config.rad_t;
    trap_0.force = config.v_trap*config.mass/config.dt;
    ball_0.rad = config.rad_b;  
    ball_0.mass = config.mass;

    section.traps.assign(1, trap_0);
    section.balls.assign(1, ball_0);
    section.activation.assign(config.ncells_per_line*config.ncells_per_line, 0); 

    //Condiciones de inicializacion correcta
    if (config.box_size < 1 || config.box_size_z < 1){
        if (pid == 0) std::cout << "Caja muy pequeña. Tamaño establecido a 1x1x1." << std::endl;
        config.box_size = 1;
        config.box_size_z = 1;
    }

    if (config.z_init > config.box_size_z){
        if (pid == 0) std::cout << "Pelota inicial fuera de la caja. Altura inicial establecida a la mitad de la caja." << std::endl;
        config.z_init = config.box_size_z/2;
    }
    
    double traps_area = 4*config.rad_t*config.rad_t*config.nt;
    double box_area = config.box_size*config.box_size;
    
    if (traps_area - box_area > config.err || config.nt == 0){
        if (pid == 0) std::cout << "Muchas trampas en la caja. Número de trampas establecido a 100." << std::endl;
        config.nt = 100;
    }
    
    int square_nt = (int)std::sqrt(config.nt)*(int)std::sqrt(config.nt);

    if (config.nt != square_nt){
        if (pid == 0) std::cout << "El numero de trampas no es un numero cuadrado. Número de trampas establecido al numero cuadrado anterior (" << square_nt << ")." << std::endl;
        config.nt = square_nt;
    }   
}
