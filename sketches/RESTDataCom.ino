/*

  RESTDataCom
 
 Extended example Bridge library to access the digital and analog pins 
 on the board through REST calls. It demonstrates how 
 you can create your own API when using REST style 
 calls through the browser.
 
 Possible commands created in this shetch:

 * "/arduino/digital/13"     -> digitalRead(13)
 * "/arduino/digital/13/1"   -> digitalWrite(13, HIGH)
 * "/arduino/analog/2/123"   -> analogWrite(2, 123)
 * "/arduino/analog/2"       -> analogRead(2)
 * "/arduino/mode/13/input"  -> pinMode(13, INPUT)
 * "/arduino/mode/13/output" -> pinMode(13, OUTPUT)
 
 This example code is part of the public domain
 
 http://arduino.cc/en/Tutorial/Bridge

  
  Sistema de adquisicion de datos Arduino.

Funciones de adquisicion de datos:\n\t 1-> tomar medida señal\n\t\t Params:(Num Señal, Num Repeticiones,Tiempo entre Repeticiones, filtrar Extremos, calculo: med,max,min,suma)
 ejemplo de uso: 0,1,3,8,1000,1 (traza inactiva 0, funcion 1, señal 3, 8 repeticiones, intervalo de 1000 ms entre rep., eliminar extremos,  calculo Media)
Funciones de tiempo: T->

 */

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

// Listen on default port 5555, the webserver on the Yun
// will forward there all the HTTP requests for us.
YunServer server;

// para depuracion, siempre activo depurar desde Serial
// info trace con parametro trace=1
//#define DEBUG 1
#define CODIGO_ERROR -9999

char incomingByte;      // a variable to read incoming client data into
int numSen=0,numRep=0,tiempoRep=1000,filtrar=1,calculo=1,trace=0;

// para lectura de sensor de ultrasonidos
#include <Ultrasonic.h>
//Ultrasonic ultrasonic(5,6); // (Trig PIN,Echo PIN)
// centímetros * 58 = Max.TimeOut
Ultrasonic ultrasonic(5,6,20000); // (Trig PIN,Echo PIN,Max.TimeOut in µsec )


void setup() {
  Serial.begin(9600);
  // Bridge startup
  pinMode(13,OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);
  
    // inicializar sensor ultrasonidos
  pinMode(4, OUTPUT); // VCC pin
  pinMode(7, OUTPUT); // GND ping
  digitalWrite(4, HIGH); // VCC +5V mode  
  digitalWrite(7, LOW);  // GND mode

  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  // Get clients coming from server
  YunClient client = server.accept();

  // There is a new client?
  if (client) {
    // Process request
    process(client);

    // Close connection and free resources.
    client.stop();
  }

  delay(50); // Poll every 50ms
}

void process(YunClient client) {
  // read the command
  String command = client.readStringUntil('/');

  // is "digital" command?
  if (command == "digital") {
    digitalCommand(client);
  }

  // is "analog" command?
  if (command == "analog") {
    analogCommand(client);
  }

  // is "mode" command?
  if (command == "mode") {
    modeCommand(client);
  }
  
  // is "data" command?
  if (command == "data") {
    dataCommand(client);
  }  
}

void dataCommand(YunClient client) {
  
    // read the oldest byte in the client buffer:
    incomingByte = client.read();
    // ver si hay que sacar info de traza
    if (incomingByte == '1')
      trace=1;
    else
      trace=0;
      
    if (trace)
      client.println(incomingByte);
    // read the next byte in the client buffer, coma
      incomingByte = client.read();
    // leer func.
      incomingByte = client.read();      
    //parse commands and functions:
    if (incomingByte == '1') {
        if (trace){
          client.println("1-> tomar medida señal");
        }
      // read the next byte in the client buffer, coma
      incomingByte = client.read();
      if (incomingByte == ',') {        
        // read the next byte in the client buffer like Integer
        numSen = client.parseInt();
        if (trace){
          client.print("numSen ");
          client.println(numSen);
        }
      }        
      // read the next byte in the client buffer, coma
      incomingByte = client.read();
      if (incomingByte == ',') {        
        // read the next byte in the client buffer like Integer
        numRep = client.parseInt();
        if (trace){
          client.print("numRep ");
          client.println(numRep);
        }
      }
      // read the next byte in the client buffer, coma
      incomingByte = client.read();
      if (incomingByte == ',') {        
        // read the next byte in the client buffer like Integer
        tiempoRep = client.parseInt();
          if (trace){
            client.print("tiempoRep ");
            client.println(tiempoRep);
          }
      }
      // read the next byte in the client buffer, coma
      incomingByte = client.read();
      if (incomingByte == ',') {        
        // read the next byte in the client buffer like Integer
        filtrar = client.parseInt();
          if (trace){
            client.print("filtrar ");
            client.println(filtrar);
          }
      }
      // read the next byte in the client buffer, coma
      incomingByte = client.read();
      if (incomingByte == ',') {        
        // read the next byte in the client buffer like Integer
        calculo = client.parseInt();
          if (trace){
            client.print("calculo ");
            client.println(calculo);
          }
      }  

    client.println(medidaSensor(client),3); 
    
    } else if(incomingByte == '/'){
              if (trace){
          client.println(incomingByte);
          client.println("funcion desconocida");
        }
        client.println(CODIGO_ERROR);
    }else{
        if (trace){
          client.println(incomingByte);
          client.println("funcion desconocida");
        }
        client.println(CODIGO_ERROR);
    } 
  
}


void digitalCommand(YunClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/digital/13/1"
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
  } 
  else {
    value = digitalRead(pin);
  }

  // Send feedback to client
  client.print(F("Pin D"));
  client.print(pin);
  client.print(F(" set to "));
  client.println(value);

  // Update datastore key with the current pin value
  String key = "D";
  key += pin;
  Bridge.put(key, String(value));
}

void analogCommand(YunClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/analog/5/120"
  if (client.read() == '/') {
    // Read value and execute command
    value = client.parseInt();
    analogWrite(pin, value);

    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" set to analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "D";
    key += pin;
    Bridge.put(key, String(value));
  }
  else {
    // Read analog pin
    value = analogRead(pin);

    // Send feedback to client
    client.print(F("Pin A"));
    client.print(pin);
    client.print(F(" reads analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "A";
    key += pin;
    Bridge.put(key, String(value));
  }
}

void modeCommand(YunClient client) {
  int pin;

  // Read pin number
  pin = client.parseInt();

  // If the next character is not a '/' we have a malformed URL
  if (client.read() != '/') {
    client.println(F("error"));
    return;
  }

  String mode = client.readStringUntil('\r');

  if (mode == "input") {
    pinMode(pin, INPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as INPUT!"));
    return;
  }

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as OUTPUT!"));
    return;
  }

  client.print(F("error: invalid mode "));
  client.print(mode);
}


// metodo encargado de las diferentes medidas
double medidaSensor(YunClient client){

  double medida=CODIGO_ERROR,maximo,minimo;
  double medidas[numRep];  
  int indiceMax=0,indiceMin=0,indiceAux=0; 
  boolean iniciadosValores=false;  
       
switch (numSen){
  // ultrasonidos
   case 1: 

       for (int rep=0;rep<numRep;rep++){
           double medida=ultrasonic.Ranging(CM);
           if (trace)
            client.println(medida);
           medidas[rep] = medida;
           if(medida>medidas[indiceMax])
             indiceMax=rep;
           if(medida<medidas[indiceMin])
             indiceMin=rep;     
           // intervalo de espera entre medidas
           delay(tiempoRep);
       }    
           // si los indices coinciden es que todas las medidas son iguales
            if(indiceMax==indiceMin)
              indiceMax=indiceMin++;
                if (trace){
                  client.println("Indices extremos"); 
                  client.println(indiceMax); 
                  client.println(indiceMin); 
                }   
                  Serial.println("Indices extremos"); 
                  Serial.println(indiceMax); 
                  Serial.println(indiceMin);                 
       // filtro de extremos: 
       if(filtrar){
         for (int rep=0;rep<numRep;rep++){
           if(rep==indiceMax)
            continue;
           if(rep==indiceMin)
             continue;
             
             if(!iniciadosValores){
               maximo=medidas[rep];
               minimo=medidas[rep];
               medida=0;               
               if (trace){
                client.println("inicializar medida"); 
                client.println(medida); 
               }
                Serial.println("inicializar medida"); 
                Serial.println(medida);                
                iniciadosValores=true;              
             }
             switch (calculo){
                case 2:  // max
                   if(medidas[rep]>maximo)
                     maximo=medidas[rep];
                  break; 
                case 3:  //min
                   if(medidas[rep]<minimo)
                     minimo=medidas[rep];                  
                  break;                   
                 default:
                   medida=medida+medidas[rep];
             }
           }
            
         
            if (trace){
                client.println("medida final"); 
                client.println(medida); 
             }
                Serial.println("medida final"); 
                Serial.println(medida);             
         //calculo final
             switch (calculo){
               case 1:  // med 
                medida=medida/(numRep-2); 
                  break;                 
                case 2:  // max
                  medida=maximo;
                  break; 
                case 3:  //min
                  medida=minimo;                
                  break;                   
                 default:
                   medida=medida;
             }        
       }else{
          for (int rep=0;rep<numRep;rep++){
             if(!iniciadosValores){
               maximo=medidas[rep];
               minimo=medidas[rep];
               medida=0;               
               if (trace){
                client.println("inicializar medida"); 
                client.println(medida); 
               }
                Serial.println("inicializar medida"); 
                Serial.println(medida);                
                iniciadosValores=true;              
             }
             switch (calculo){
                case 2:  // max
                   if(medidas[rep]>maximo)
                     maximo=medidas[rep];
                  break; 
                case 3:  //min
                   if(medidas[rep]<minimo)
                     minimo=medidas[rep];                  
                  break;                   
                 default:
                   medida=medida+medidas[rep];
             }
           
            
         }
              if (trace){
                client.println("medida final"); 
                client.println(medida); 
             } 
                Serial.println("medida final"); 
                Serial.println(medida);              
         //calculo final
             switch (calculo){
               case 1:  // med 
                medida=medida/(numRep); 
                  break;                 
                case 2:  // max
                  medida=maximo;
                  break; 
                case 3:  //min
                  medida=minimo;                
                  break;                   
                 default:
                   medida=medida;
             }         
         
       }
       break;
   default:
     break;
}

              if (trace){
                client.println("retorno"); 
                client.println(medida); 
             } 
                Serial.println("retorno"); 
                Serial.println(medida);   

return medida;

}
