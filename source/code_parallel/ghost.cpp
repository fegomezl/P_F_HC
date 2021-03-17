#include "header.h"

int lim_ball_up(domain &section, int (&N_send)[2], Vec_b &balls_send);
int lim_ball_down(domain &section, int (&N_send)[2], Vec_b &balls_send);
void lim_trap_up(const domain &section, Vec_t &traps_send);
void lim_trap_down(const domain &section, Vec_t &traps_send);

void ghost (const int &pid, const CONFIG &config, domain &section, MPI_Datatype body_type, MPI_Datatype trap_type)
{

  int tag = 3; //Mensaje de comunicación
  int N_send[2] {0,0}; //Número de pelotas que salen del dominio y pelotas que se comparten
  //N_send = {N_lost, N_shared}
  int total_sent = 0; //Perdidas + compartidas
  
  int N_recv[2] {0,0}; //Número de pelotas que entran al dominio y pelotas que se comparten
  //N_recv = {N_stolen, N_shared}
  int total_recv = 0; //Recibidas + compartidas

  //Limites de ciclo for
  int first = 0;
  int last = 0;

  int T_send_up = section.traps_send_up;
  int T_send_down = section.traps_send_down; //Número de trampas que se envian

  int T_recv_up =  section.traps_recv_up;
  int T_recv_down = section.traps_recv_down; //Número de trampas que se reciben
  
  Vec_b balls_send_up;
  Vec_b balls_send_down; //Vector de bolas que salen del dominio o se comparten por el limite superior o inferior

  Vec_b balls_recv_up;
  Vec_b balls_recv_down; //Vector de pelotas que entran al dominio o se comparten por el limite superior o inferior

  Vec_t traps_send_up;
  Vec_t traps_send_down; //Vector de trampas que se comparten por el limite superior o inferior
  
  Vec_t traps_recv_up;
  Vec_t traps_recv_down; //Vector de trampas que se comparten por el limite superior o inferior

  //Eliminar informacion compartida anterior
  section.balls.resize(section.local_size);
  section.traps.resize(section.local_trap_size);

  if (pid == 0){

    //Verificar limite superior
    total_sent = lim_ball_up(section, N_send, balls_send_up);
    lim_trap_up(section, traps_send_up);

    //Enviar cantidad de pelotas al proceso superior
    MPI_Send(N_send, 2, MPI_INT, pid+1, tag, MPI_COMM_WORLD);
    
    if (total_sent!=0){
      MPI_Send(balls_send_up.data(), total_sent, body_type, pid+1, tag, MPI_COMM_WORLD);
    }

    //Enviar trampas al proceso superior
    if (T_send_up!=0){
      MPI_Send(traps_send_up.data(), T_send_up, trap_type, pid+1, tag, MPI_COMM_WORLD);
    }

    //Recibir numero de pelotas del proceso superior
    MPI_Recv(N_recv, 2, MPI_INT, pid+1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    total_recv = N_recv[0] + N_recv[1];
    
    if (total_recv!=0){
      balls_recv_up.resize(total_recv);
      MPI_Recv(balls_recv_up.data(), total_recv, body_type, pid+1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      section.local_size += N_recv[0];
      for (int i = 0; i < N_recv[0]; i++){
        section.balls.emplace_back(balls_recv_up[i]); //Añadir pelotas robadas del proceso superior
      }
      for (int i = N_recv[0]; i < total_recv; i++){
        section.balls.emplace_back(balls_recv_up[i]); //Añadir pelotas compartidas del proceso superior
      }
    }

    //Recibir trampas del proceso superior
    if (T_recv_up!=0){
      traps_recv_up.resize(T_recv_up);
      MPI_Recv(traps_recv_up.data(), T_recv_up, trap_type, pid+1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (auto t : traps_recv_up){
        section.traps.emplace_back(t); //Añadir trampas compartidas del proceso superior
      }
    }
    
  }
  else if (0<pid && pid<config.nproc-1){

    //Recibir numero de pelotas del proceso anterior
    MPI_Recv(N_recv, 2, MPI_INT, pid-1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    total_recv = N_recv[0] + N_recv[1];
    
    if (total_recv!=0){
      balls_recv_down.resize(total_recv);
      MPI_Recv(balls_recv_down.data(), total_recv, body_type, pid-1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      section.local_size += N_recv[0];
      for (int i = 0; i< N_recv[0]; i++){
        section.balls.emplace_back(balls_recv_down[i]); //Añadir pelotas robadas del proceso anterior
      }
      first = N_recv[0];
      last = total_recv;
    }

    //Recibir trampas del proceso anterior
    if (T_recv_down!=0){
      traps_recv_down.resize(T_recv_down);
      MPI_Recv(traps_recv_down.data(), T_recv_down, trap_type, pid-1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (auto t : traps_recv_down){
        section.traps.emplace_back(t); //Añadir trampas compartidas del proceso anterior
      }
    }

    //Revisar limite superior
    total_sent = lim_ball_up(section, N_send, balls_send_up);
    lim_trap_up(section, traps_send_up);
    
    //Enviar numero de pelotas al proceso superior
    MPI_Send(N_send, 2, MPI_INT, pid+1, tag, MPI_COMM_WORLD);
    
    if (total_sent!=0){
      MPI_Send(balls_send_up.data(), total_sent, body_type, pid+1, tag, MPI_COMM_WORLD);
    }

    //Enviar trampas al proceso superior
    if (T_send_up!=0){
      MPI_Send(traps_send_up.data(), T_send_up, trap_type, pid+1, tag, MPI_COMM_WORLD);
    }
    
    //Recibir el numero de pelotas del proceso superior
    MPI_Recv(N_recv, 2, MPI_INT, pid+1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    total_recv = N_recv[0] + N_recv[1];
    
    if (total_recv!=0){
      balls_recv_up.resize(total_recv);
      MPI_Recv(balls_recv_up.data(), total_recv, body_type, pid+1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      section.local_size += N_recv[0];
      for (int i = 0; i < N_recv[0]; i++){
          section.balls.emplace_back(balls_recv_up[i]); //Añadir pelotas robadas del proceso superior
      }
      if (last != 0){
          for (int i = first; i< last; i++){
              section.balls.emplace_back(balls_recv_down[i]); //Añadir pelotas compartidas del proceso anterior
          }
      }
      for (int i = N_recv[0]; i < total_recv; i++){
        section.balls.emplace_back(balls_recv_up[i]); //Añadir pelotas compartidas del proceso superior
      }
    }

    //Recibir trampas del proceso superior
    if (T_recv_up!=0){
      traps_recv_up.resize(T_recv_up);
      MPI_Recv(traps_recv_up.data(), T_recv_up, trap_type, pid+1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      
      for (auto t : traps_recv_up){
        section.traps.emplace_back(t); //Añadir trampas compartidas del proceso superior
      }
    }
    
    //Revisar limite inferior
    total_sent = lim_ball_down(section, N_send, balls_send_down);
    lim_trap_down(section, traps_send_down);

    //Enviar numero de pelotas al proceso anterior
    MPI_Send(N_send, 2, MPI_INT, pid-1, tag, MPI_COMM_WORLD);
    
    if (total_sent!=0){
      MPI_Send(balls_send_down.data(), total_sent, body_type, pid-1, tag, MPI_COMM_WORLD);
    }

    //Enviar trampas al proceso anterior
    if (T_send_down!=0){
      MPI_Send(traps_send_down.data(), T_send_down, trap_type, pid-1, tag, MPI_COMM_WORLD);
    }

  }
  else {

    //Recibir el numero de pelotas del proceso anterior
    MPI_Recv(N_recv, 2, MPI_INT, pid-1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    total_recv = N_recv[0] + N_recv[1];
    if (total_recv!=0){
      balls_recv_down.resize(total_recv);
      MPI_Recv(balls_recv_down.data(), total_recv, body_type, pid-1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      section.local_size += N_recv[0];
      for (int i = 0; i < N_recv[0]; i++){
        section.balls.emplace_back(balls_recv_down[i]); //Añadir pelotas robadas del proceso anterior
      }
      for (int i = N_recv[0]; i < total_recv; i++){
        section.balls.emplace_back(balls_recv_down[i]); //Añadir pelotas compartidas del proceso anterior
      }
    }

    //Recibir trampas del proceso anterior
    if (T_recv_down!=0){
      traps_recv_down.resize(T_recv_down);
      MPI_Recv(traps_recv_down.data(), T_recv_down, trap_type, pid-1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (auto t : traps_recv_down){
        section.traps.emplace_back(t); //Añadir trampas compartidas del proceso anterior
      }
    }
    
    //Revisar limite inferior
    total_sent = lim_ball_down(section, N_send, balls_send_down);
    lim_trap_down(section, traps_send_down);

    //Enviar pelotas al proceso anterior
    MPI_Send(N_send, 2, MPI_INT, pid-1, tag, MPI_COMM_WORLD);

    if (total_sent!=0){
      MPI_Send(balls_send_down.data(), total_sent, body_type, pid-1, tag, MPI_COMM_WORLD);
    }

    //Enviar trampas al proceso anterior
    if (T_send_down!=0){
      MPI_Send(traps_send_down.data(), T_send_down, trap_type, pid-1, tag, MPI_COMM_WORLD);
    }
  }
  
}

int lim_ball_up(domain &section, int (&N_send)[2], Vec_b &balls_send)
{
  N_send[0] = 0;
  N_send[1] = 0; //Eliminar numero de pelotas de comunicaciones anteriores
  
  section.balls.erase( std::remove_if( section.balls.begin(), section.balls.begin()+section.local_size,
                                       [&section, &N_send, &balls_send](body ball){
                                         if (section.max <= ball.pos.x()){ //Encontrar pelotas perdidas y eliminarlas
                                           N_send[0]++;
                                           section.local_size--;
                                           balls_send.emplace_back(ball);
                                           return true;
                                         } else {return false;}
                                       })
                       ,section.balls.begin()+section.local_size
  );
  std::copy_if( section.balls.begin(), section.balls.begin()+section.local_size, std::back_inserter(balls_send), 
                [&section, &N_send](body ball){
                  if (section.max-2*ball.rad<ball.pos.x() && ball.pos.x()<section.max){ //Encontrar pelotas compartidas del limite superior
                    N_send[1]++;
                    return true;
                  } else {return false;}
                });
  
  return N_send[0] + N_send[1];
}

int lim_ball_down(domain &section, int (&N_send)[2], Vec_b &balls_send)
{
  N_send[0] = 0;
  N_send[1] = 0; //Eliminar numero de pelotas de comunicaciones anteriores
  
  section.balls.erase( std::remove_if( section.balls.begin(), section.balls.begin()+section.local_size,
                                       [&section, &N_send, &balls_send](body ball){
                                         if (ball.pos.x() < section.min){ //Encontrar pelotas perdidas y eliminarlas
                                           N_send[0]++;
                                           section.local_size--;
                                           balls_send.emplace_back(ball);
                                           return true;
                                         } else {return false;}
                                       })
                       ,section.balls.begin()+section.local_size
  );
  std::copy_if( section.balls.begin(), section.balls.begin()+section.local_size, std::back_inserter(balls_send), 
                [&section, &N_send](body ball){
                  if (section.min<=ball.pos.x() && ball.pos.x()<section.min+2*ball.rad){ //Encontrar pelotas compartidas del limite anterior
                    N_send[1]++;
                    return true;
                  } else {return false;}
                });
  
  return N_send[0] + N_send[1];
}

void lim_trap_up(const domain &section, Vec_t &traps_send)
{
  int first = section.local_trap_size-section.traps_send_up;
  std::copy_n( section.traps.begin()+first, section.traps_send_up, std::back_inserter(traps_send));
}

void lim_trap_down(const domain &section, Vec_t &traps_send)
{
  std::copy_n( section.traps.begin(), section.traps_send_down, std::back_inserter(traps_send));
}
