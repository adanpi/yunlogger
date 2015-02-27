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
  
    
   Extendido para Sistema de adquisicion de datos Arduino yun: yunlogger
  
  Funciones de adquisicion de datos: data 1-> tomar medida sensor ultrasonidos Params:(traza, funcion, Num Señal (1=ultrasonidos), Num Repeticiones,Tiempo entre Repeticiones, filtrar Extremos, calculo: med,max,min,suma)
   ejemplo de uso: 0,1,3,8,1000,1 (traza inactiva 0, funcion 1, señal 3, 8 repeticiones, intervalo de 1000 ms entre rep., eliminar extremos,  calculo Media)
  
  Ejemplos:
  http://localhost/usr-cgi/luci/arduino/analog/2
  http://localhost/usr-cgi/luci/arduino/data/0,1,1,5,200,1,1
  http://yun/usr-cgi/luci/arduino/data/1,1,1,5,200,1,1
  
  Incorporado a 17 de diciembre de 2014
  
  Funciones de tiempo: time 1-> leer fecha/hora del RTC, devuelve unixtime en segundos (desde 1-1-1970)
                            2-> leer fecha/hora del RTC, devuelve formato string (dd/MM/aaaa hh:mm:ss)
                            3-> establecer fecha/hora, parmetros: traza,año,mes,dia,hora,minuto,segundo
                            
  http://localhost/usr-cgi/luci/arduino/time/1,1
  http://yun/usr-cgi/luci/arduino/time/1,2
  http://yun/usr-cgi/luci/arduino/time/1,3,2014,12,17,16,30,10
  
  Incorporado a 29 de diciembre de 2014
  
  Funciones de escritura de servomotor: http://yun/usr-cgi/luci/arduino/servo/9/180
  
  Funciones de lectura de digitales 10,11 y 12 en bloque, con digital -1 (/arduino/digital/-1)
  
  11 de Febrero de 2014: se incorpora lectura modbus por rs232 de otro arduino
  
  */
  
  #include <Bridge.h>
  #include <YunServer.h>
  #include <YunClient.h>
  #include <Wire.h>
//  #include <Servo.h>
  #include "RTClib.h"

  // modbus
  #include <SimpleModbusMasterSoftSerial.h>
  #include <SoftwareSerial.h>
  SoftwareSerial mySerial(15, 16); // RX, TX
  //////////////////// Port information ///////////////////
#define baud 9600
#define timeout 1000
#define polling 200 // the scan rate
#define retry_count 10
// used to toggle the receive/transmit pin on the driver
#define TxEnablePin 2
// The total amount of available memory on the master to store data
#define TOTAL_NO_OF_REGISTERS 4

// This is the easiest way to create new packets
// Add as many as you want. TOTAL_NO_OF_PACKETS
// is automatically updated.
enum
{
  PACKET1,
//  PACKET2,
  TOTAL_NO_OF_PACKETS // leave this last entry
};

// Create an array of Packets to be configured
Packet packets[TOTAL_NO_OF_PACKETS];

// Masters register array
unsigned int regs[TOTAL_NO_OF_REGISTERS];
  // fin modbus

  RTC_DS1307 RTC;
//  Servo myservo;
  // Listen on default port 5555, the webserver on the Yun
  // will forward there all the HTTP requests for us.
  YunServer server;
  
  // para depuracion, siempre activo depurar desde //Serial
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
  
  const int buttonPin = 12;     // the number of the pushbutton pin
  const int ledPin =  11;      // the number of the LED pin
  int buttonState = 0;         // variable for reading the pushbutton status


  void setup() {
    ////Serial.begin(9600);
    // Bridge startup
    pinMode(13,OUTPUT);
    digitalWrite(13, LOW);
    Bridge.begin();
    digitalWrite(13, HIGH);
    
    //iniciar leds
    pinMode(10,OUTPUT);
    pinMode(ledPin,OUTPUT);
    //boton    
    pinMode(buttonPin,INPUT);
    

      // inicializar sensor ultrasonidos
    pinMode(4, OUTPUT); // VCC pin
    pinMode(7, OUTPUT); // GND ping
    digitalWrite(4, HIGH); // VCC +5V mode  
    digitalWrite(7, LOW);  // GND mode
  
    // Listen for incoming connection only from localhost
    // (no one from the external network could connect)
    //server.listenOnLocalhost();
    server.begin();
    
    Wire.begin();
    RTC.begin();
//    myservo.attach(9);  // attaches the servo on pin 9 to the servo object

    // modbus
    modbus_construct(&packets[PACKET1], 1, READ_HOLDING_REGISTERS, 0, 4, 0);
    modbus_configure(&mySerial, baud, SERIAL_8N1, timeout, polling, retry_count, TxEnablePin, packets, TOTAL_NO_OF_PACKETS, regs);
    // fin modbus
  }
  
  void loop() {
    
      // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    // turn LED on:
    digitalWrite(ledPin, HIGH);
  }
  else {
    // turn LED off:
    digitalWrite(ledPin, LOW);
  }
  
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
    /*
    // is "mode" command?
    if (command == "mode") {
      modeCommand(client);
    }
    */
    // is "data" command?
    if (command == "data") {
      dataCommand(client);
    }  
    
      // is "time" command?
    if (command == "time") {
      timeCommand(client);
    }  
    
    // is "servo" command?
    if (command == "servo") {
      //servoCommand(client);
    } 
    
    // is "modbus" command?
    if (command == "modbus") {
      mbCommand(client);
    }
    
  }

  
  void mbCommand(YunClient client){

    client.println("ModBus");
    modbus_update();
    client.println(regs[0]);
    client.println(regs[1]);
    client.println(regs[2]);
    client.println(regs[3]);
  }
 /* 
  void servoCommand(YunClient client){
    int pin, value;
       // Read pin number
    pin = client.parseInt();
  
    // If the next character is a '/' it means we have an URL
    // with a value like: "/servo/5/120"
    if (client.read() == '/') {
      // Read value and execute command
      value = client.parseInt();
      analogWrite(pin, value);
  
      // Send feedback to client
      client.print(F("Servo en pin "));
      client.print(pin);
      client.print(F(" set to degrees "));
      client.println(value);
      if(value>180)
        client.println("valor > de 180");
      else
        myservo.write(value);
      }else{
       client.println("falta valor del servo ej: /servo/9/45");
     }
    
  }
  */
  void timeCommand(YunClient client) {
    
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
            client.println("1-> unix time seconds");
          }
       if (! RTC.isrunning()) {
      client.println("RTC is NOT running!");
      return;
      }
          DateTime now = RTC.now();
          client.println(now.unixtime(),3);
        } else if (incomingByte == '2') {
          if (trace){
            client.println("2-> unix time string");
          }
      if (! RTC.isrunning()) {
      client.println("RTC is NOT running!");
      return;
      }
      DateTime now = RTC.now();    
      if(now.day()<10)  client.print('0');
      client.print(now.day(), DEC);        
      client.print('/');
      if(now.month()<10)  client.print('0');
      client.print(now.month(), DEC);
      client.print('/');
      client.print(now.year(), DEC);    
      client.print(' ');
      if(now.hour()<10)  client.print('0');
      client.print(now.hour(), DEC);
      client.print(':');
      if(now.minute()<10)  client.print('0');
      client.print(now.minute(), DEC);
      client.print(':');
      if(now.second()<10)  client.print('0');
      client.print(now.second(), DEC);
      client.println();
  
        } else if (incomingByte == '3') {
          if (trace){
            client.println("3-> set RTC time");
          }
        // variables establecer fecha 
        int anio=-1,mes=-1,dia=-1,hora=-1,minuto=-1,sec=-1;
        // read the next byte in the client buffer, coma
        incomingByte = client.read();
        if (incomingByte == ',') {        
          // read the next byte in the client buffer like Integer
          anio = client.parseInt();
          if (trace){
            client.print("anio ");
            client.println(anio);
          }
        }        
        // read the next byte in the client buffer, coma
        incomingByte = client.read();
        if (incomingByte == ',') {        
          // read the next byte in the client buffer like Integer
          mes = client.parseInt();
          if (trace){
            client.print("mes ");
            client.println(mes);
          }
        }   
              
        // read the next byte in the client buffer, coma
        incomingByte = client.read();
        if (incomingByte == ',') {        
          // read the next byte in the client buffer like Integer
          dia = client.parseInt();
          if (trace){
            client.print("dia ");
            client.println(dia);
          }
        } 
                
        // read the next byte in the client buffer, coma
        incomingByte = client.read();
        if (incomingByte == ',') {        
          // read the next byte in the client buffer like Integer
          hora = client.parseInt();
          if (trace){
            client.print("hora ");
            client.println(hora);
          }
        } 
                
        // read the next byte in the client buffer, coma
        incomingByte = client.read();
        if (incomingByte == ',') {        
          // read the next byte in the client buffer like Integer
          minuto = client.parseInt();
          if (trace){
            client.print("min ");
            client.println(minuto);
          }
        } 
                
        // read the next byte in the client buffer, coma
        incomingByte = client.read();
        if (incomingByte == ',') {        
          // read the next byte in the client buffer like Integer
          sec = client.parseInt();
          if (trace){
            client.print("sec ");
            client.println(sec);
          }
        } 
        
         if (! RTC.isrunning()) {
          client.println("RTC is NOT running!");
        return;
        }
        if (!(anio!=-1 & mes!=-1 & dia!=-1 & hora!=-1 & minuto!=-1 & sec!=-1)){
         client.println("faltan parametros");
        return;
        }
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
      
      RTC.adjust(DateTime(anio, mes, dia, hora, minuto, sec));
      
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
    
    // si se pide el pin -1 se leen los pines 10 a 12 y se devuelven seguidos ej: 101
    if(pin==-1)  {
      value = digitalRead(10);
      client.print(value);
      value = digitalRead(11);
      client.print(value);
      value = digitalRead(12);
      client.print(value);
      client.println("00000");
    }else{
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
  //    client.print(F("Pin A"));
  //    client.print(pin);
  //    client.print(F(" reads analog "));
      client.println(value);
  
      // Update datastore key with the current pin value
      String key = "A";
      key += pin;
      Bridge.put(key, String(value));
    }
  }
  /*
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
  
  */
  
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
                    //Serial.println("Indices extremos"); 
                    //Serial.println(indiceMax); 
                    //Serial.println(indiceMin);                 
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
                  //Serial.println("inicializar medida"); 
                  //Serial.println(medida);                
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
                  //Serial.println("medida final"); 
                  //Serial.println(medida);             
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
                  //Serial.println("inicializar medida"); 
                  //Serial.println(medida);                
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
                  //Serial.println("medida final"); 
                  //Serial.println(medida);              
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
                  //Serial.println("retorno"); 
                  //Serial.println(medida);   
  
  return medida;
  
  }
