#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <random>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include "Vector.h"
#include "mpi.h"

//Condiciones iniciales
struct CONFIG{
  double box_size = 0;     //Tamaño lateral de la caja en X y Y 
  double box_size_z = 0;   //Tamaño lateral de la caja en Z
  int nt = 0;              //Número de trampas
  int ncells_per_line = 0; //Número de columnas de celdas por procesador para el calculo de probabilidad
  int seed = 0;            //Semilla aleatorea
  
  double rad_t = 0;      //Radio de las trampas
  double rad_b = 0;      //Radio de las pelotas
  double mass = 0;       //Masa de las pelotas
  double z_init = 0;     //Altura de la pelota de activacion
  
  double dt = 0;         //Paso de tiempo
  int iterations = 0;    //Numero de iteraciones
  int resolution = 0;    //Cantidad de pasos para imprimir de nuevo
  double err = 0;        //Rango de error para condicionales cercanos a cero
  int nproc = 1;         //Procesadores a usar
  
  double g = 0;          //Aceleración de la gravedad
  
  double k_wall = 0;     //Constante de dureza de las paredes
  double k_ball = 0;     //Constante de dureza entre pelotas
  double v_trap = 0;     //Velocidad que imprime la trampa a las pelotas (exp)

  double gamma_wall = 0; //Constante de choque plastico con la pared
  double gamma_ball = 0; //Constante de choque plastico entre pelotas
  
  double mu_wall = 0;    //Constante de friccion con la pared
  double mu_ball = 0;    //Constante de friccion con entre pelotas
  double mu_trap = 0;    //Constante de friccion con la trampa

  double desv_theta = 0; //Desviación en dirección azimutal del golpe de la trampa (exp)
  double desv_phi = 0;   //Desviación en dirección polar del golpe de la trampa (exp)
};

//Pelotas de ping pong
struct body{
  double mass = 0;               //Masa
  double rad = 0;                //Radio
  double I = (2*mass*rad*rad)/5; //Momento de inercia

  vector3D pos;                  //Posición
  vector3D vel;                  //Velocidad
  vector3D force;                //Fuerza

  vector3D angle;                //Angulo de giro
  vector3D omega;                //Velocidad angular
  vector3D torque;               //Torque
};

//Trampas de raton
struct trap{
  double force = 0;  //Fuerza que imprime la trampa
  double rad = 0;    //Radio de la trampa
  int active = 0;    //La trampa esta activa (1) o no (0)
  int send_up = 0;   //La trampa se comparte con el proceso posterior (1) o no (0)
  int send_down = 0; //La trampa se comparte con el proceso anterior (1) o no (0)
    
  vector3D pos;     //Posición de la trampa
};

//Elementos de un dominio
struct domain{
  double min = 0;           //Columna inicial de trampas
  double max = 0;           //Columna final de trampas
  int local_size = 0;       //Número de pelotas en el dominio
  int local_trap_size = 0;  //Número de pelotas y trampas en el dominio
  
  int traps_send_up = 0;
  int traps_send_down = 0;  //Número de trampas a enviar al proceso siguiente y anterior
  
  int traps_recv_up = 0;
  int traps_recv_down = 0;  //Número de trampas a recibir del proceso siguiente y anterior
  
  std::vector<trap> traps;  //Trampas dentro del dominio
  std::vector<body> balls;  //Pelotas dentro del dominio
  
  std::vector<double> activation;          //Número de trampas activadas en el dominio por celda
  double activation_partial_sum = 0;       //Numero de trampas activadas en el dominio en un momento dado
  double activation_total_sum = 0;         //Numero de trampas activadas en el dominio
  double energy = 0;                       //Energía del dominio
};

//Simplificación de notación
typedef std::vector<int> Vec_i;
typedef std::vector<double> Vec_d;
typedef std::vector<body> Vec_b;
typedef std::vector<trap> Vec_t;

//Funciones principales

void init_data(const int &pid, CONFIG &config, domain &section);  //Lee las condiciones iniciales del sistema

void start(const int &pid, const CONFIG &config, domain &section); //Ubica las particulas

void print(const int &time, const int &pid, const CONFIG &config,
           MPI_Datatype body_type, MPI_Datatype trap_type, domain &section);  //Imprime el estado del sistema

void ghost(const int &pid, const CONFIG &config, domain &section,
           MPI_Datatype body_type, MPI_Datatype trap_type);  //Procesos de información compartida entre fronteras

void compute_force(const CONFIG &config, domain &section,
                   std::normal_distribution<double> &desv_theta, std::normal_distribution<double> &desv_phi, std::mt19937 &gen);  //Calcula las fuerzas

void time_step(const int &pid, const CONFIG &config, domain &section,
                std::normal_distribution<double> &desv_theta, std::normal_distribution<double> &desv_phi, std::mt19937 &gen,
               MPI_Datatype body_type, MPI_Datatype trap_type);  //Da un paso de tiempo
