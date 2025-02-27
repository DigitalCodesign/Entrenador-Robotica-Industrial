#include "CinematicaInversa.h"

int l = 0;
float resta1[2];
float resta2[2];
float r[2];
float distancia = 1;
float dif_angulo = 0;
float AnguloPaso;
float angulo1;
float angulo2;
float limite;
int contador = 0;
float limite_bajo;
float limite_alto;

void CI (float x, float y, float z, int l1, int l2, int l3, int limite_min, int limite_max, float &Paso0, float &Grado1, float &Grado2, float &Grado3) {

  //El código se queda en un blucle while hasta que alcanza el valor deseado

  distancia = 1;
  contador = 0;

  limite_bajo = limite_min*(2*M_PI/360);
  limite_alto = limite_max*(2*M_PI/360);

  //Cálculo del paso que tiene que alcanzar el stepper

  if( x == 0){

    if(z >= 0){

      AnguloPaso = M_PI/2;

    }

    if(z < 0){

      AnguloPaso = -M_PI/2;

    }

  }

  if(x > 0){

    if(z > 0){

      AnguloPaso = atan(z/x);

    }

    if(z < 0){

      AnguloPaso = - atan(abs(z)/x);

    }

    if(z == 0){

        AnguloPaso = 0;

    }

  }

  if(x < 0){

    if(z > 0){

      AnguloPaso = atan(z/abs(x)) + M_PI/2;

    }

    if(z < 0){

      AnguloPaso = - atan(z/x) - M_PI/2;

    }

    if(z == 0){

        AnguloPaso = M_PI;

    }

  }

  if (AnguloPaso >= 0){

    Paso0 = (AnguloPaso * M_PI) / 90.0;
    //Paso0 = map(AnguloPaso, 0, M_PI, 0, 90);

  }

  if (AnguloPaso <= 0){

    Paso0 = (AnguloPaso * M_PI) / -90.0;
    //Paso0 = map(AnguloPaso, 0, -M_PI, 0, -90);

  }

  Grado1 = M_PI/2;
  Grado2 = M_PI/2;
  Grado3 = M_PI/2;

  while(distancia >= 0.01){

    //Comenzamos con un if con un contador para que haga los cálculos una sola vez

    if (contador == 0){

      //Se calcula los vectores para el punto final

      r[0] = sqrt(sq(x)+sq(z));
      r[1] = y;
      l = sqrt(sq(l1-l3)+sq(l2));  //Límite del propio brazo
      limite = sqrt(sq(r[0])+sq(r[1]));  //Alcance del brazo

    }

    float O00[2] = {0, 0};
    float O01[2] = {cos(Grado1)*l1,sin(Grado1)*l1};
    float O02[2] = {cos(Grado2-(M_PI/2 - Grado1))*l2 + O01[0],sin(Grado2-(M_PI/2 - Grado1))*l2 + O01[1]};
    float O03[2] = {cos(Grado3-(M_PI/2 - (Grado2-(M_PI/2-Grado1))))*l3 + O02[0],sin(Grado3-(M_PI/2 - (Grado2 - (M_PI/2 - Grado1))))*l3 + O02[1]};

    resta1[0] = r[0] - O02[0];
    resta1[1] = r[1] - O02[1];
    resta2[0] = O03[0] - O02[0];
    resta2[1] = O03[1] - O02[1];


    if (resta1[0] == 0){

      if (resta1[1] >= 0){

        angulo1 = M_PI/2;

      }

      if (resta1[1] < 0){

        angulo1 = -M_PI/2;

      }

    } 

    if (resta1[0] != 0){

      if(resta1[0] >= 0){

        if (resta1[1] >= 0){

          angulo1 = abs(atan(resta1[1] / resta1[0]));

        }

        if (resta1[1] < 0){

          angulo1 = - abs(atan(resta1[1] / resta1[0]));

        }

      }

      if(resta1[0] < 0){

        if (resta1[1] >= 0){

          angulo1 = abs(atan(resta1[1] / resta1[0]));

        }

        if (resta1[1] < 0){

          angulo1 = - abs(atan(resta1[1] / resta1[0]));

        }

      }

    }

    if (resta2[0] == 0){

      if (resta2[1] >= 0){

        angulo2 = M_PI/2;

      }

      if (resta2[1] < 0){

        angulo2 = -M_PI/2;

      }

    } 
  
    if (resta2[0] != 0){

      if(resta2[0] >= 0){

        if (resta2[1] >= 0){

          angulo2 = abs(atan(resta2[1] / resta2[0]));

        }

        if (resta2[1] < 0){

          angulo2 = - abs(atan(resta2[1] / resta2[0]));

        }

      }

      if(resta2[0] < 0){

        if (resta2[1] >= 0){

          angulo2 = abs(atan(resta2[1] / resta2[0]));

        }

        if (resta2[1] < 0){

          angulo2 = - abs(atan(resta2[1] / resta2[0]));

        }

      }

    }

    dif_angulo = angulo2 - angulo1;

    Grado3 = Grado3 - dif_angulo;

    if (Grado3 <= limite_bajo){

      Grado3 = limite_bajo;

    }

    if (Grado3 >= limite_alto){

      Grado3 = limite_alto;

    }

    O03[0] = cos(Grado3-(M_PI/2 - (Grado2-(M_PI/2-Grado1))))*l3 + O02[0];
    O03[1] = sin(Grado3-(M_PI/2 - (Grado2-(M_PI/2-Grado1))))*l3 + O02[1];

    resta1[0] = r[0] - O01[0];
    resta1[1] = r[1] - O01[1];
    resta2[0] = O03[0] - O01[0];
    resta2[1] = O03[1] - O01[1];

    if (resta1[0] == 0){

      if (resta1[1] >= 0){

        angulo1 = M_PI/2;

      }

      if (resta1[1] < 0){

        angulo1 = -M_PI/2;

      }

    } 
  
    if (resta1[0] != 0){

      if(resta1[0] >= 0){

        if (resta1[1] >= 0){

          angulo1 = abs(atan(resta1[1] / resta1[0]));

        }

        if (resta1[1] < 0){

          angulo1 = - abs(atan(resta1[1] / resta1[0]));

        }

      }

      if(resta1[0] < 0){

        if (resta1[1] >= 0){

          angulo1 = abs(atan(resta1[1] / resta1[0]));

        }

        if (resta1[1] < 0){

          angulo1 = - abs(atan(resta1[1] / resta1[0]));

        }

      }

    }

    if (resta2[0] == 0){

      if (resta2[1] >= 0){

        angulo2 = M_PI/2;

      }

      if (resta2[1] < 0){

        angulo2 = -M_PI/2;

      }

    } 

    if (resta2[0] != 0){

      if(resta2[0] >= 0){

        if (resta2[1] >= 0){

          angulo2 = abs(atan(resta2[1] / resta2[0]));

        }

        if (resta2[1] < 0){

          angulo2= - abs(atan(resta2[1] / resta2[0]));

        }

      }

      if(resta2[0] < 0){

        if (resta2[1] >= 0){

          angulo2 = abs(atan(resta2[1] / resta2[0]));

        }

        if (resta2[1] < 0){

          angulo2 = - abs(atan(resta2[1] / resta2[0]));

        }

      }

    }

    dif_angulo = angulo2 - angulo1;

    Grado2 = Grado2 - dif_angulo;

    if (Grado2 <= limite_bajo){

      Grado2 = limite_bajo;

    }

    if (Grado2 >= limite_alto){

      Grado2 = limite_alto;

    }

    O02[0] = cos(Grado2-(M_PI/2 - Grado1))*l2 + O01[0];
    O02[1] = sin(Grado2-(M_PI/2 - Grado1))*l2 + O01[1];
    O03[0] = cos(Grado3-(M_PI/2 - (Grado2-(M_PI/2-Grado1))))*l3 + O02[0];
    O03[1] = sin(Grado3-(M_PI/2 - (Grado2-(M_PI/2-Grado1))))*l3 + O02[1];

    resta1[0] = r[0] - O00[0];
    resta1[1] = r[1] - O00[1];
    resta2[0] = O03[0] - O00[0];
    resta2[1] = O03[1] - O00[1];

    if (resta1[0] == 0){

      if (resta1[1] >= 0){

        angulo1 = M_PI/2;

      }

      if (resta1[1] < 0){

        angulo1 = -M_PI/2;

      }

    } 

    if (resta1[0] != 0){

      if(resta1[0] >= 0){

        if (resta1[1] >= 0){

          angulo1 = abs(atan(resta1[1] / resta1[0]));

        }

        if (resta1[1] < 0){

          angulo1 = - abs(atan(resta1[1] / resta1[0]));

        }

      }

      if(resta1[0] < 0){

        if (resta1[1] >= 0){

          angulo1 = abs(atan(resta1[1] / resta1[0]));

        }

        if (resta1[1] < 0){

          angulo1 = - abs(atan(resta1[1] / resta1[0]));

        }

      }

    }

    if (resta2[0] == 0){

      if (resta2[1] >= 0){

        angulo2 = M_PI/2;

      }

      if (resta2[1] < 0){

        angulo2 = -M_PI/2;

      }

    } 

    if (resta2[0] != 0){

      if(resta2[0] >= 0){

        if (resta2[1] >= 0){

          angulo2 = abs(atan(resta2[1] / resta2[0]));

        }

        if (resta2[1] < 0){

          angulo2= - abs(atan(resta2[1] / resta2[0]));

        }

      }

      if(resta2[0] < 0){

        if (resta2[1] >= 0){

          angulo2 = abs(atan(resta2[1] / resta2[0]));

        }

        if (resta2[1] < 0){

          angulo2 = - abs(atan(resta2[1] / resta2[0]));

        }

      }

    }

    dif_angulo = angulo2 - angulo1;

    Grado1 = Grado1 - dif_angulo;

    if (Grado1 <= limite_bajo){

      Grado1 = limite_bajo;

    }

    if (Grado1 >= limite_alto){

      Grado1 = limite_alto;

    }

    O01[0] = cos(Grado1)*l1;
    O01[1] = sin(Grado1)*l1;
    O02[0] = cos(Grado2-(M_PI/2 - Grado1))*l2 + O01[0];
    O02[1] = sin(Grado2-(M_PI/2 - Grado1))*l2 + O01[1];
    O03[0] = cos(Grado3-(M_PI/2 - (Grado2-(M_PI/2-Grado1))))*l3 + O02[0];
    O03[1] = sin(Grado3-(M_PI/2 - (Grado2-(M_PI/2-Grado1))))*l3 + O02[1];

    distancia = sqrt(sq(abs(O03[0]-r[0]))+sq(abs(O03[1]-r[1])));
    contador += 1;

  }

  Grado1 = Grado1 * (180.0 / M_PI);
  Grado2 = Grado2 * (180.0 / M_PI);
  Grado3 = Grado3 * (180.0 / M_PI);

}