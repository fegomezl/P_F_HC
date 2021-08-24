#include "header.h"

void start(const int &pid, const CONFIG &config, domain &section){
  
  //Se acomodan las trampas y pelotas en el suelo
  int n = std::sqrt(config.nt); //total de filas y columnas de trampas
  double space = config.box_size/(1.0*n); //espacio entre trampas
  double origin = space/2.0; //posición de referencia de la primera trampa

  double x = 0; //posiciones de las trampas y pelotas en el suelo
  double y = 0;

  section.min = pid*(config.box_size/config.nproc); //valores mínimo y máximo en Y de la sección
  section.max = section.min + (config.box_size/config.nproc);
  int raws = 0; //numero de filas de trampas en la sección
  int start = 0; //indice de la primera fila en la sección
  int end = 0; //indice de la ultima fila en la sección
  
  for (int i = 0; i < n; i++){ //encontrar los valores de raws, start y end
    x = origin + i*space;
    if (section.min<=x && x<section.max){
      raws += 1;
      end = i;
    }
  }
  start = end - (raws-1);
  section.local_size = raws*n;
  section.local_trap_size = raws*n;

  double ghost_len = config.rad_b+config.rad_t; //ancho de la región compartida de trampas

  double dis_upper_lim = section.max - (origin+end*space); //distancia de la ultima columna al maximo de la sección
  double dis_lower_lim = (origin+start*space) - section.min; //distancia de la primera columna al mínimo de la sección
  
  if (pid!=config.nproc-1 && (dis_upper_lim < ghost_len)){
    section.traps_send_up++;
  }
  if (pid!=0 && (dis_lower_lim < ghost_len)){
    section.traps_send_down++;
  }
  
  section.traps_send_up *= n;
  section.traps_send_down *= n; //trampas que estarán compartidas por los dominios

  //comunicar cuantas trampas se enviaran a los procesos adyacentes
  int tag = 0;
  
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

  if (raws != 0){
    section.traps.resize(section.local_trap_size+section.traps_recv_down+section.traps_recv_up, section.traps[0]);
    section.balls.resize(section.local_size, section.balls[0]);

    int index = 0; //indice en el vector de trampas y pelotas de cada arreglo
    int first = n*start; //primer y ultima bola en en la sección
    int last = n*(end+1);
    double gap = 0;
    
    for (int i = first; i < last; i++){ //Se inicializan las trampas y bolas de la sección
      x = origin + floor(i/n)*space;
      y = origin + (i%n)*space;
      gap = section.traps[index].rad-section.balls[index].rad;  //Desface de la posicion de la pelota en cuanto a la trampa
      if (section.traps_send_down != 0 && index < n) section.traps[index].send_down = 1;  //Si la trampa hace parte de la region compartida inferior
      else if (section.traps_send_up != 0 && index >= last - n) section.traps[index].send_up = 1;  //Si la trampa hace parte de la region compartida superior
      section.traps[index].pos.cargue(x,y,0);
      section.balls[index].pos.cargue(x,y-gap,config.rad_b);
      section.balls[index].angle.cargue(0,0,0);
      section.balls[index].vel.cargue(0,0,0);
      section.balls[index].omega.cargue(0,0,0);
      index++;
    }

    if (start<=floor(n/2) && floor(n/2)<=end){ //Se inicializa la pelota que causa la reacción
      section.local_size += 1;
      x = origin + floor(n/2)*space;
      y = origin + floor(n/2)*space;
      section.balls.emplace_back(section.balls[0]);
      section.balls[section.balls.size()-1].pos.cargue(x,y,config.z_init);
      section.balls[section.balls.size()-1].angle.cargue(0,0,0);
      section.balls[section.balls.size()-1].vel.cargue(0,0,0);
      section.balls[section.balls.size()-1].omega.cargue(0,0,0);
    }
  }
}
