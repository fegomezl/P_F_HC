#include "header.h"

void force_wall(const CONFIG &config, body &ball, const double &h, const double &H, const vector3D &direction);
void force_ball(const CONFIG &config, body &ball_2, body &ball_1);
void force_trap(const CONFIG &config, body &ball, trap &trap_act, std::normal_distribution<double> &desv_theta, std::normal_distribution<double> &desv_phi, std::mt19937 &gen);

void compute_force(const CONFIG &config, domain &section, std::normal_distribution<double> &desv_theta, std::normal_distribution<double> &desv_phi, std::mt19937 &gen){
  //Calcula las fuerzas del sistema

  //variables con la misma notacion que el articulo
  vector3D i, j, k;       //Vectores unitarios
  i.cargue(1,0,0); j.cargue(0,1,0); k.cargue(0,0,1);
  
  double h = 0, H = 0;    //distancia de interpenetracion paredes y trampas

  //Inicialización del estado dinamico del sistema
  for (int ii = 0; ii < section.local_size; ii++){
    section.balls[ii].force.cargue(0,0,-config.mass*config.g); //Resetar las fuerzas con solo PESO
    section.balls[ii].torque.cargue(0,0,0);              //Resetar el torque
  }

  for (int ii = 0; ii < section.local_size; ii++){

    //Fuerza con las paredes x=0 y x=Lx
    h = section.balls[ii].rad - section.balls[ii].pos.x();  
    H = section.balls[ii].rad + section.balls[ii].pos.x() - config.box_size;
    force_wall(config, section.balls[ii], h, H, i);

    //Fuerza con las paredes y=0 y y=Ly
    h = section.balls[ii].rad - section.balls[ii].pos.y();  
    H = section.balls[ii].rad + section.balls[ii].pos.y() - config.box_size;
    force_wall(config, section.balls[ii], h, H, j);

    //Fuerza con las paredes z=0 y z=Lz
    h = section.balls[ii].rad - section.balls[ii].pos.z();  
    H = section.balls[ii].rad + section.balls[ii].pos.z() - config.box_size_z;
    force_wall(config, section.balls[ii], h, H, k);

    //Eliminar las fuerzas en z si la pelota esta en el piso
    if (section.balls[ii].pos.z() == section.balls[ii].rad && section.balls[ii].vel.z() == 0){
      section.balls[ii].force.cargue(section.balls[ii].force.x(),section.balls[ii].force.y(),0);
    }

    //Fuerzas entre particulas
    for (int jj = ii + 1; jj < section.balls.size(); jj++) force_ball(config, section.balls[ii], section.balls[jj]);

    //Fuerzas con las trampas
    for (int jj = 0; jj < section.traps.size(); jj++){
      if (section.traps[jj].active == 1) force_trap(config, section.balls[ii], section.traps[jj], desv_theta, desv_phi, gen);
    }
  }
}

void force_wall(const CONFIG &config, body &ball, const double &h, const double &H, const vector3D &direction){
  //Fuerza con las paredes u=0 y u=Lu, con u = x,y,z
    
  vector3D V;          //velocidad de la bola
  vector3D Vn;         //velocidad normal relativa
  vector3D Vt;         //velocidad tangencial relativa
    
  vector3D FHertz;     //fuerza de Hertz
  vector3D Fnp;        //fuerza normal plastica
  vector3D Fn;         //fuerza normal total
    
  vector3D tangente;   //vector unitario en direccion tangente a dr
  vector3D Ftp;        //fuerza tangencial plastica

  vector3D Ftotal;     //fuerza total sobre el centro de masa
  vector3D torque;     //torque total
    
  double norm_Vt = 0;      //norma de vt

  if (h > config.err)//Pared u=0
  {
    V = ball.vel + (ball.omega^((ball.rad - h/2)*direction)); //velocidad de contacto de la bola con la pared

    //Fuerzas normales
    Vn = (V*direction)*direction; //velocidad normal 
    FHertz = (config.k_wall*h*std::sqrt(h))*direction; //fuerza de Hertz
    Fnp = (-config.gamma_ball*std::sqrt(h))*Vn;    //fuerza normal plastica
    Fn = FHertz + Fnp; //fuerza normal total

    //Fuerzas tangenciales
    Vt = V - Vn; //velocidad tangente 
    norm_Vt = norma(Vt); //norma de la velocidad tangente
    if (norm_Vt != 0) tangente = Vt/norm_Vt; //vector unitario tangente
    else tangente.cargue(0,0,0);
    Ftp = (-config.mu_wall*norma(Fn))*tangente;  //friccion

    //Fuerza total
    Ftotal = Fn + Ftp;
    torque = Ftp^((ball.rad - h/2)*direction);
    ball.force += Ftotal; ball.torque += torque;
  }
  else if (H > config.err) //Pared u=Lu
  {
    //Velocidades relativas
    V = ball.vel + (ball.omega^((ball.rad - H/2)*direction)); //velocidad de contacto de la bola con la pared

    //Fuerzas normales
    Vn = (V*direction)*direction; //velocidad normal 
    FHertz = (-config.k_wall*H*std::sqrt(H))*direction; //fuerza de Hertz
    Fnp = (config.gamma_ball*std::sqrt(H))*Vn;    //fuerza normal plastica
    Fn = FHertz + Fnp; //fuerza normal total

    //Fuerzas tangenciales
    Vt = V - Vn; //velocidad tangente 
    norm_Vt = norma(Vt); //norma de la velocidad tangente
    if (norm_Vt != 0) tangente = Vt/norm_Vt; //vector unitario tangente
    else tangente.cargue(0,0,0);
    Ftp = (-config.mu_wall*norma(Fn))*tangente;  //friccion

    //Fuerza total
    Ftotal = Fn + Ftp;
    torque = -1*(Ftp^((ball.rad - h/2)*direction));
    ball.force += Ftotal; ball.torque += torque;
  }
}

void force_ball(const CONFIG &config, body &ball_2, body &ball_1){
  //Fuerza entre pelotas de ping pong
  
  vector3D V1;             //velocidad de la bola 1
  vector3D V2;             //velocidad de la bola 2
  vector3D Vc;             //velocidad relativa entre las dos pelotas 
  vector3D Vn;             //velocidad normal relativa
  vector3D Vt;             //velocidad tangencial relativa
  
  vector3D dr;             //vector que va desde el centro de una pelota a otra
  vector3D normal;         //vector unitario en direccion dr
  vector3D FHertz;         //fuerza de Hertz
  vector3D Fnp;            //fuerza normal plastica
  vector3D Fn;         //fuerza normal total

  vector3D tangente;       //vector unitario en direccion tangente a dr
  vector3D Ftp;            //fuerza tangencial plastica
  vector3D Ftpmax;         //fuerza tangencial plastica maxima
  vector3D Ft;             //fuerza tangencial 

  vector3D Ftotal;         //fuerza total sobre el centro de masa
  vector3D torque;         //torque total
  
  double norm_dr = 0;          //norma de dr
  double norm_Vt = 0;          //norma de vt
  double S = 0;            //distancia de interpenetracion entre bolas
  double x = 0;            //Longitud del resorte de cundall

  dr = ball_2.pos - ball_1.pos; //Distancia relativa entre centro de las pelotas
  norm_dr = norma(dr);
  S = ball_2.rad + ball_1.rad - norm_dr; //distancia de interpenetracion
      
  if (S > config.err)   
  {       
    V2 = ball_2.vel - (ball_2.omega^(dr*(1/2))); //velocidad de la bola 2
    V1 = ball_1.vel + (ball_1.omega^(dr*(1/2))); //velocidad de la bola 1
    Vc = V2 - V1; //velocidad relativa

    //Fuerzas normales
    normal = dr/norm_dr; //calculo del vector normal unitario
    Vn = (Vc*normal)*normal; //velocidad normal relativa
    FHertz = (config.k_ball*S*std::sqrt(S))*normal; //fuerza de Hertz
    Fnp = (-0.5*config.gamma_ball*config.mass*std::sqrt(S))*Vn; //fuerza normal plastica
    Fn = FHertz + Fnp;  //fuerza normal total

    //Fuerzas tangenciales
    Vt = Vc - Vn; //velocidad tangente relativa
    norm_Vt = norma(Vt); //norma de la velocidad tangente
    if (norm_Vt != 0) tangente = Vt/norm_Vt; //vector unitario tangente
    else tangente.cargue(0,0,0);
    x = norma(Vt)*config.dt; //longitud del resorte de cundall
    Ftp = (-config.k_ball*x)*tangente;   //fuerza tangente plastica
    Ftpmax = (-config.mu_ball*norma(Fn))*tangente;  //fuerza tangencial maxima, friccion cinetica
    if (norma(Ftp) < norma(Ftpmax)) Ft = Ftp; //fuerza tangente, friccion estatica
    else Ft = Ftpmax;   //fuerza tangente maxima, friccion cinetica

    //Fuerza total
    Ftotal = Fn + Ft;
    torque = Ft^(dr*(1/2));
    ball_2.force += Ftotal;  ball_2.torque += torque;
    ball_1.force -= Ftotal;  ball_1.torque -= torque;
  }
}

void force_trap(const CONFIG &config, body &ball, trap &trap_act, std::normal_distribution<double> &desv_theta, std::normal_distribution<double> &desv_phi, std::mt19937 &gen){
  //Fuerza de una trampa activa a una pelota

  vector3D V;              //velocidad de la trampa
  vector3D Vunit;          //velocidad unitaria

  vector3D dr;             //vector que va desde el centro de una pelota al centro de una trampa
  vector3D R;              //vector que va de la trampa al punto de contacto
  vector3D r;              //vector que va del punto de contacto a la pelota
  vector3D runit;          //vector unitario en direccion de r
  vector3D runitpert;      //vector unitario en direccion de r perturbado
  vector3D Fn;             //fuerza normal total
  vector3D i, j, k;       //Vectores unitarios
  i.cargue(1,0,0); j.cargue(0,1,0); k.cargue(0,0,1); 
  
  vector3D tangente;       //vector unitario en direccion tangente a dr
  vector3D Ftp;            //fuerza tangencial plastica

  vector3D Ftotal;         //fuerza total sobre el centro de masa
  vector3D torque;         //torque total
  
  double norm_dr = 0;          //Norma de dr
  double norm_drxdrxj = 0;     //Norma del producto cruz entre dr y j
  double norm_V = 0;           //Norma de la velocidad de la trampa
  double norm_r = 0;           //Norma de la direccion de la fuerza sin perturbar
  double norm_runitxk = 0;     //Norma del producto cruz entre runit y k
  double S = 0;                //distancia de interpenetracion entre bolas
  double alpha = 0;            //parametros para la fuerza de colision con trampas
  double beta = 0;             //parametros para la fuerza de colision con trampas
  double beta_aux = 0;         //Termino dentro de la raiz de beta

  dr = ball.pos - trap_act.pos;   //vector que va desde el centro de la trampa hasta el centro de la pelota
  norm_dr = norma(dr);            
  S = ball.rad - std::fabs(norm_dr - trap_act.rad);  //Condicion para que la pelota le pegue la varilla de la trampa
        
  if (S > config.err)
  {
    alpha = 0.5*(1 + (std::pow(trap_act.rad,2) - std::pow(ball.rad,2))/std::pow(norm_dr,2)); //parametro de escala alpha
    norm_drxdrxj = norma(dr^(dr^j));
    if (norm_drxdrxj != 0){   //Condicional para evitar posible overflow
        beta = std::sqrt(std::pow(trap_act.rad,2) - std::pow(norm_dr*alpha,2))/norm_drxdrxj;  //parametro de escala beta
        R = alpha*dr + beta*(dr^(dr^j)); //vector que va de la trampa al punto de contacto
    } else {
      R = alpha*dr; //vector que va de la trampa al punto de contacto
    }
    r = dr - R;     //vector que va del punto de contacto a la pelota
    V = (M_PI/config.dt)*(R^i); //velocidad de la trampa en el punto de contacto
    norm_V = norma(V);
    norm_r = norma(r);
    if (norm_V != 0) Vunit = V/norma(V); //vector unitario en direccion de la velocidad
    else Vunit.cargue(0,0,0);
    if (norm_r != 0) runit = r/norma(r); //vector unitario en direccion r
    else runit.cargue(0,0,0);
    

    //Fuerza normal
    norm_runitxk = norma(runit^k);
    if (norm_runitxk != 0) runitpert = runit + ((desv_theta(gen)*k + desv_phi(gen)*((runit^k)/norm_runitxk))^runit); //vector unitario en direccion r perturbado
    else runitpert = runit + ((desv_theta(gen)*k)^runit);
    Fn = (trap_act.force*(runit*Vunit))*runitpert;

    //Fuerza tangencial
    tangente = V - (runit*V)*runit; //vector unitario tangente
    Ftp = (-norma(Fn)*config.mu_trap)*tangente;

    //Fuerza total
    Ftotal = Fn + Ftp;
    torque = Ftp^r; 
    ball.force += Ftotal;   ball.torque+= torque;
  }
}
