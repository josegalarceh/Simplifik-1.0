// El Shogo se la come 
holaaaaaaaaa
nose que onda pero dice que hay 3 ramas..... que significa eso???
#include <SPI.h>
#include <Ethernet.h>
#include <DS3231.h>
#include <Wire.h>
//#define controlmega "1 c c"
#define controlmega "1    "
DS3231 clock;
RTCDateTime dt;
///****************flag control**********/
// 0: No muestra nada por el serial
// 1: Muestra todo por el serial
#define FLAG_CTRL 0
///***comunicacion por serial 1 (mega-chibi)***/
// 0: Mantener comunicacion
// 1: Desactivar comunicacion
#define Closeserial1 0

#define RELAY_ON 0   //Recordar que los reles se activan con nivel BAJO (0) 
#define RELAY_OFF 1
#define RELAY01 5    //rele 1 conectado al pin 2, Reset Chibi
#define RELAY02 6    //rele 2 conectado al pin 3, Rx mega Tx chibi
#define RELAY03 7    //rele 3 conectado al pin 4, Tx mega Rx chibi

//cadenas recibidas desde coo_chibi por el coo_mega
String payloadRx_chibi_mega;
String Linea1;
String Linea2;
String Linea3;
String Linea4;
int counter_l;
bool complete1 = false;
bool complete2 = false;
bool complete3 = false;
bool complete_All;
int lineok[3];
int cabecera;
int termino;
int resetear;

String palabra;
String trama;
int corte;
//cadena enviada desde coo_mega para coo_chibi
String payloadTx_mega_chibi;
//arreglos para control de aspersores
String payloadTx_mega_chibi_st01;
String payloadTx_mega_chibi_st03;

String M01SS;     //module 1 state                  -> 0: none, 1: automatico (se rige por el setpoint de temperatura), 2: manual (se activan remotamente los aspersores)
String M01SPT;    //module 1 set point Temperature, -> valor en grados celcius, 1 decimal
String M01SPK;    //module 1 sprinklers,            -> 0: none, 1: open, 2: close
String M03SS;     //module 3 state                  -> 0: none, 1: automatico (se rige por el setpoint de temperatura), 2: manual (se activan remotamente los aspersores)
String M03SPT;    //module 3 set point Temperature, -> valor en grados celcius, 1 decimal
String M03SPK;    //module 3 sprinklers,            -> 0: none, 1: open, 2: close

//arreglos para estado y valores de sensores
String M1SS1, M1S1T, M1S1H, M1SS2, M1S2T, M1S2H; //M1SS1: module 1 state sensor 1, M1S1T: module 1 sensor 1 temperature, M1S1H: module 1 sensor 1 relative humidity
String M2SS1, M2S1T, M2S1H, M2SS2, M2S2T, M2S2H;
String M3SS1, M3S1T, M3S1H, M3SS2, M3S2T, M3S2H;

//buffers para estado y valores de sensores
char m1ss1[1], m1s1t[5], m1s1h[5], m1ss2[1], m1s2t[5], m1s2h[5];
char m2ss1[1], m2s1t[5], m2s1h[5], m2ss2[1], m2s2t[5], m2s2h[5];
char m3ss1[1], m3s1t[5], m3s1h[5], m3ss2[1], m3s2t[5], m3s2h[5];
//buffer para Estado de aspersores 
char  m01spk[1], m03spk[1];
//buffer payloadTx_mega_web
char valor_2[11], valor_3[11], valor_4[11], valor_5[11];
char valor_6[11], valor_7[11], valor_8[11], valor_9[11];
char valor_10[11], valor_11[11], valor_12[11], valor_13[11];
char valor_14[13];
char valor_15[4], valor_16[4];
char valor_17[5], valor_18[5], valor_19[5];
 
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 5, 111); //IP en la toma 12 del switch
IPAddress gateway(192, 168, 5, 1); //IP del router dlink
IPAddress subnet(255, 255, 255, 0);
//http://requestb.in/10aadxw1?inspect
//http://166.62.82.132:8080/agendamiento/ws/procesaNuevaTrama/holaaaaaa primera prueba hugo
//http://166.62.82.132:8080/agrosmart/ws/procesaNuevaTrama/

//Change to your server domain
//char serverName[] = "requestb.in";
char serverName[] = "166.62.82.132";//primera prueba con hugo

// change to your server's port
//int serverPort = 80;
int serverPort = 8080;
// change to the page on that server

//char pageName[] = " /13x8lmy1";
char pageName[] = " /agendamiento/ws/procesaNuevaTrama/";  //primera prueba con hugo
//char pageName[] = " /agrosmart/ws/procesaNuevaTrama/";   //segunda prueba hugo

EthernetClient client;
int totalCount = 0;
// insure params is big enough to hold your variables
char params[365];

// set this to the number of milliseconds delay
// this is 30 seconds
#define delayMillis 5000UL
int REB;
unsigned long thisMillis = 0;
unsigned long lastMillis = 0;

char webstrgread[101];
String webLine1;
String webLine2;
String webLine3;
String webLine4;
String webLine5;
String webLine6;
String webLine7;
String webLine8;
String webLine9;//linea util desde la web para controlar modulos
//webLine9[12]=    M01SS
//webLine9[14-17]= M01SPT
//webLine9[19]=    M01SPK1
//webLine9[21]=    M03SS
//webLine9[23-26]= M03SPT
//webLine9[28]=    spk_m02
String webLine10;
int webcont;
bool webcmplt1;
bool webcmplt2;
bool webcmplt3;
bool webcmplt4;
bool webcmplt5;
bool webcmplt6;
bool webcmplt7;
bool webcmplt8;
bool webcmplt9;
bool webcmplt10;
bool webcmplt_All;
bool webrcbd_dts;
int webcabecera;
int webtermino;
String webpalabra;

String clientline;
String payloadRx_serial = "1";
void setup() {
  Wire.begin();
  Serial.begin(9600);   //habilitar comunicacion puerto serial (revision/control PC)
  Serial1.begin(9600);  //habilitar comunicacion puerto serial 1 (coo_mega-coo_chibi)
  //Asegurar nivel ALTO en cada entrada de rele
  digitalWrite (RELAY01, RELAY_OFF); //Activa la salida digital 5 para matener en Off el Rele de Reset
  digitalWrite (RELAY02, RELAY_OFF); //rele 2 Cable Rx mega-Tx chibi
  digitalWrite (RELAY03, RELAY_OFF); //rele 3 Cable Tx mega-Rx chibi
  //Definir los pines como salida
  pinMode (RELAY01, OUTPUT); //rele 1 Reset Chibi
  pinMode (RELAY02, OUTPUT); //rele 2 Cable Rx mega-Tx chibi
  pinMode (RELAY03, OUTPUT); //rele 3 Cable Tx mega-Rx chibi
  delay (10);
  if (Closeserial1 == 1)CloseSerial1(); //cierra comunicacion serial1 
  Serial1.write(controlmega);     //le pide al coo_chibi los valores de los sensores
  if (FLAG_CTRL != 0)control_1(); //revisa mensaje enviado
  counter_l = 1;
  webcont = 1;
  delay(1000);
  // disable SD SPI
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  //para no tener problemas, solo se le carga una vez la hora y asi queda seteada para "siempre"
  // Manual          (Year, M, D, H, M, S)
  //clock.setDateTime(2016,11,18,15,45,00);  // Seteo de Tiempo, sumarle 1 dia 1 hora a la fecha actual
  //clock.setDateTime(1477533180);  // Seteo de Tiempo
  Serial.print(F("Starting ethernet: "));
  REB = Ethernet.begin(mac);
  if (REB == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    Ethernet.begin(mac, ip);
  } else {
    Serial.println(F("Ok"));
  }
  control_15();
}
void loop() {
  if (Serial.available() > 0) {    //si existe datos disponibles los leemos
    payloadRx_serial = Serial.readString();// read the incoming data as string
    Serial.print(F("payloadRx_serial: "));
    Serial.println(payloadRx_serial);
  }
  if (Serial1.available()) { // Loop para obtener datos desde el coo_chibi (serial 1)
    char caracter = Serial1.read();
    payloadRx_chibi_mega += caracter; //entra al if cuando hay "una nueva linea"
    if (caracter == 10) { //10: nueva linea
      switch (counter_l) {
        case 1:
          Linea1 = payloadRx_chibi_mega;  //almacena la primera linea
          payloadRx_chibi_mega = "";      //limpia string
          Serial1.flush();                //limpia el buffer
          counter_l += 1;                 //salta a la siguiente linea
          break;
        case 2:
          Linea2 = payloadRx_chibi_mega;
          payloadRx_chibi_mega = "";
          Serial1.flush(); 
          counter_l += 1;
          break;
        case 3:
          Linea3 = payloadRx_chibi_mega;
          payloadRx_chibi_mega = "";
          Serial1.flush();  
          counter_l += 1;
          break;
        case 4:
          Linea4 = payloadRx_chibi_mega;
          payloadRx_chibi_mega = "";
          Serial1.flush();  
          counter_l += 1;
          break;
        default:
          // default is optional
          break;
      }//switch
    }//if(caracter == 10)
    if (counter_l > 4) {
      Serial.print(F("Linea1--->"));
      Serial.print(Linea1);
      Serial.print(F("Linea2--->"));
      Serial.print(Linea2);
      Serial.print(F("Linea3--->"));
      Serial.print(Linea3);
      /*Serial.print(F("Linea4--->"));
      Serial.print(Linea4);*/
      // REVISION DE DATOS DE ENTRADA
      cabecera = Linea1.indexOf("|#|1|");
      termino = Linea1.indexOf("|#|$");
      if (cabecera > 0) {
        if (termino > 0) {
          complete1 = true;
        } else {
          Serial.println(F("reenvio1"));
        }
      } else {
        Serial.println(F("reenvio1"));
      }//if cabecera termino ok
      cabecera = Linea2.indexOf("|#|2|");
      termino = Linea2.indexOf("|#|$");
      if (cabecera > 0) {
        if (termino > 0) {
          complete2 = true;
        } else {
          Serial.println(F("reenvio2"));
        }
      } else {
        Serial.println(F("reenvio2"));
      }//if cabecera termino ok
      cabecera = Linea3.indexOf("|#|3|");
      termino = Linea3.indexOf("|#|$");
      if (cabecera > 0) {
        if (termino > 0) {
          complete3 = true;
        } else {
          Serial.println(F("reenvio3"));
        }
      } else {
        Serial.println(F("reenvio3"));
      }//if-else cabecera termino ok
      //cabecera_termino(Linea3);
      //asegura que las 3 lineas esten completas
      if(complete1 && complete2 && complete3) {
        Serial.println(F("Lineas Completas"));
        Estructura();
        ethernet();
      }else{//
        resetear_chibi();
        Linea1 == "";
        Linea2 == "";
        Linea3 == "";
        Linea4 == "";
        Serial1.flush();
        delay(10); 
        Serial1.write(controlmega);//le pide los valores al chibi de los sensores
        Serial.println(F(" "));
        Serial.print(F("No estan completas las lineas, controlmega: "));
        Serial.println(controlmega);
        Serial.println(F("reenvio ALL"));
        delay(10);
      }//if lineas completas
    }//if(counter_l > 4)
  }//if(Serial1.available())
}//void loop
void Estructura() {
  Serial.println(F("  ... Estructura ... "));
  palabra = Linea1;
  palabra.replace("@|#|1|", "");
  M1SS1 = palabra.substring(0, 1);
  palabra.remove(0, 2);
  corte = palabra.indexOf(";");
  M1S1T  = palabra.substring(0, corte);
  palabra.remove(0, (corte + 1));
  corte = palabra.indexOf("|");
  M1S1H = palabra.substring(0, corte);
  palabra.remove(0, (corte + 1));
  corte = palabra.indexOf("|");
  M1SS2 = palabra.substring(0, 1);
  palabra.remove(0, 2);
  corte = palabra.indexOf(";");
  M1S2T  = palabra.substring(0, corte);
  palabra.remove(0, (corte + 1));
  corte = palabra.indexOf("|");
  M1S2H = palabra.substring(0, corte);
  Serial.print(F(" M1SS1 = "));
  Serial.print(M1SS1);
  Serial.print(F("\t"));
  Serial.print(F("M1S1T = "));
  Serial.print(M1S1T);
  Serial.print(F("\t"));
  Serial.print(F("M1S1H = "));
  Serial.println(M1S1H);
  Serial.print(F(" M1SS2 = "));
  Serial.print(M1SS2);
  Serial.print(F("\t"));
  Serial.print(F("M1S2T = "));
  Serial.print(M1S2T);
  Serial.print(F("\t"));
  Serial.print(F("M1S2H = "));
  Serial.println(M1S2H);

  palabra = Linea2;
  palabra.replace("@|#|2|", "");
  M2SS1 = palabra.substring(0, 1);
  palabra.remove(0, 2);
  corte = palabra.indexOf(";");
  M2S1T  = palabra.substring(0, corte);
  palabra.remove(0, (corte + 1));
  corte = palabra.indexOf("|");
  M2S1H = palabra.substring(0, corte);
  palabra.remove(0, (corte + 1));
  corte = palabra.indexOf("|");
  M2SS2 = palabra.substring(0, 1);
  palabra.remove(0, 2);
  corte = palabra.indexOf(";");
  M2S2T  = palabra.substring(0, corte);
  palabra.remove(0, (corte + 1));
  corte = palabra.indexOf("|");
  M2S2H = palabra.substring(0, corte);
  Serial.print(F(" M2SS1 = "));
  Serial.print(M2SS1);
  Serial.print(F("\t"));
  Serial.print(F("M2S1T = "));
  Serial.print(M2S1T);
  Serial.print(F("\t"));
  Serial.print(F("M2S1H = "));
  Serial.println(M2S1H);
  Serial.print(F(" M2SS2 = "));
  Serial.print(M2SS2);
  Serial.print(F("\t"));
  Serial.print(F("M2S2T = "));
  Serial.print(M2S2T);
  Serial.print(F("\t"));
  Serial.print(F("M2S2H = "));
  Serial.println(M2S2H);

  palabra = Linea3;
  palabra.replace("@|#|3|", "");
  M3SS1 = palabra.substring(0, 1);
  palabra.remove(0, 2);
  corte = palabra.indexOf(";");
  M3S1T  = palabra.substring(0, corte);
  palabra.remove(0, (corte + 1));
  corte = palabra.indexOf("|");
  M3S1H = palabra.substring(0, corte);
  palabra.remove(0, (corte + 1));
  corte = palabra.indexOf("|");
  M3SS2 = palabra.substring(0, 1);
  palabra.remove(0, 2);
  corte = palabra.indexOf(";");
  M3S2T  = palabra.substring(0, corte);
  palabra.remove(0, (corte + 1));
  corte = palabra.indexOf("|");
  M3S2H = palabra.substring(0, corte);
  Serial.print(F(" M3SS1 = "));
  Serial.print(M3SS1);
  Serial.print(F("\t"));
  Serial.print(F("M3S1T = "));
  Serial.print(M3S1T);
  Serial.print(F("\t"));
  Serial.print(F("M3S1H = "));
  Serial.println(M3S1H);
  Serial.print(F(" M3SS2 = "));
  Serial.print(M3SS2);
  Serial.print(F("\t"));
  Serial.print(F("M3S2T = "));
  Serial.print(M3S2T);
  Serial.print(F("\t"));
  Serial.print(F("M3S2H = "));
  Serial.println(M3S2H);

  Linea1 == "";
  Linea2 == "";
  Linea3 == "";
  //Linea4 == "";
  resetear_chibi();
}//void Estructura
void ethernet() {
  Serial.println(F("  ... Iniciando Envio via Ethernet ..."));
  dt = clock.getDateTime();
  Imprime_timestamp();
  // If using a static IP, comment out the next line
  if (REB != 0)Ethernet.maintain();
  /*trama = "data=grupo,dispositivo,timestamp,temperatura,humedad,lat,lon\r\n63ac215c-4478-11e6-beb8-9e71128cae77,09:55:22:AC:9E:C4,";
    trama += clock.dateFormat("U", dt);
    trama += "001,";
    trama += M1S1T;
    trama += ",";
    trama += M1S2T;
    trama += ",";
    trama += M1S1H;
    trama += ",";
    trama += M1S2H;
    trama += ",-32.872210,-71.209331\r\n63ac215c-4478-11e6-beb8-9e71128cae77,09:55:22:AC:9E:C5,";
    trama += clock.dateFormat("U", dt);
    trama += "002,";
    trama += M2S1T;
    trama += ",";
    trama += M2S2T;
    trama += ",";
    trama += M2S1H;
    trama += ",";
    trama += M2S2H;
    trama += ",-32.871701,-71.208575\r\n63ac215c-4478-11e6-beb8-9e71128cae77,09:55:22:AC:9E:C6,";
    trama += clock.dateFormat("U", dt);
    trama += "003,";
    trama += M3S1T;
    trama += ",";
    trama += M3S2T;
    trama += ",";
    trama += M3S1H;
    trama += ",";
    trama += M3S2H;
    trama += ",-32.872566,-71.208554";
    trama.toCharArray(params, 400);
    Serial.println(F("Trama Completa:"));*/
  //cargando datos a la url de hugo
  String TSUNIX = clock.dateFormat("U", dt);
  char tsunix[12];
  TSUNIX.toCharArray(tsunix, 12);
  M1S1T.toCharArray(m1s1t, 5); M1S1H.toCharArray(m1s1h, 5); M1S2T.toCharArray(m1s2t, 5); M1S2H.toCharArray(m1s2h, 5);
  M2S1T.toCharArray(m2s1t, 5); M2S1H.toCharArray(m2s1h, 5); M2S2T.toCharArray(m2s2t, 5); M2S2H.toCharArray(m2s2h, 5);
  M3S1T.toCharArray(m3s1t, 5); M3S1H.toCharArray(m3s1h, 5); M3S2T.toCharArray(m3s2t, 5); M3S2H.toCharArray(m3s2h, 5);
  char dataEDS[150];//buffer, aca va ir toda la data desde el EDS

  //////prueba para el EDA///// tb saltar a linea 203 app if()...
  //char dataEDA[150] = "2,42.8,1|2,69.4,1";
  //char dataEDA[150];
  //payloadRx_serial.toCharArray(dataEDA, 50);
  /////////////////////////////

  /*sprintf(dataEDS, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", tsunix,
  m1s1t, m1s1h, m1s2t, m1s2h,
  m2s1t, m2s1h, m2s2t, m2s2h,
  m3s1t, m3s1h, m3s2t, m3s2h);*/
  /*sprintf(dataEDS, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", tsunix,
  m1s1t, m1s1h, m1s2t, m1s2h,
  m2s1t, m2s1h, m2s2t, m2s2h,
  m3s1t, m3s1h, m3s2t, m3s2h,
  m01spk,m03spk);*/
  sprintf(valor_2,"S50_%s_11",m1s1t);
  sprintf(valor_3,"S51_%s_11",m1s1h);
  sprintf(valor_4,"S52_%s_11",m1s2t);
  sprintf(valor_5,"S53_%s_11",m1s2h);
  sprintf(valor_6,"S54_%s_11",m2s1t);
  sprintf(valor_7,"S55_%s_11",m2s1h);
  sprintf(valor_8,"S56_%s_11",m2s2t);
  sprintf(valor_9,"S57_%s_11",m2s2h);
  sprintf(valor_10,"S58_%s_11",m3s1t);
  sprintf(valor_11,"S59_%s_11",m3s1h);
  sprintf(valor_12,"S60_%s_11",m3s2t);
  sprintf(valor_13,"S61_%s_11",m3s2h);
  sprintf(valor_14,"S62_402307_11");
  sprintf(valor_15,"C1_22");
  sprintf(valor_16,"C2_22");
  sprintf(valor_17,"C20_31");
  sprintf(valor_18,"C21_41");
  sprintf(valor_19,"C22_31");
  sprintf(dataEDS, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", tsunix,
  valor_2, valor_3, valor_4, valor_5,
  valor_6, valor_7, valor_8, valor_9,
  valor_10, valor_11, valor_12, valor_13,
  valor_14,
  valor_15, valor_16,
  valor_17, valor_18, valor_19);
  thisMillis = millis();
  Imprime_millis();
  if (thisMillis - lastMillis > delayMillis) {
    lastMillis = thisMillis;
    Serial.println(F(" --->Sending..."));
    // params must be url encoded.
    sprintf(params, params, "");
    if (!postPage(serverName, serverPort, pageName, params, dataEDS)) { //<--aca se debe cambiar dataEDA por data EDS
      Serial.print(F("  ---> Fail "));
    }
    else {
      Serial.println(F("  ---> Espera XXXs..."));
      delay(60000); // TIEMPO DE ESPERA  PARA REENVIAR LOS DATOS
      Serial.print(F("  ---> Pass "));
    }
    totalCount++;
    Serial.println(totalCount, DEC);
    /*Serial1.write(controlmega);//le pide los valores al chibi de los sensores
    //Serial1.write(controlmega);//le pide los valores al chibi de los sensores
    //Serial.println("Controlmega no enviado <-----");*/
    /*Serial.println(" ");
      Serial.print(F("Controlmega luego del postpage: "));
      Serial.println(controlmega);*/
    Serial.print(F("Trama enviada a Chibi: "));
    //Serial.println(payloadTx_mega_chibi);
    Serial.println(controlmega);
    Serial.println(F("Trama esta siendo enviada a Chibi ..."));
    //Serial1.print(payloadTx_mega_chibi);
    Serial1.write(controlmega);
    counter_l = 1;
    complete1 = false;
    complete2 = false;
    complete3 = false;
    //webcont = 1;
    void loop();
  }
}//void ethernet
byte postPage(char* domainBuffer, int thisPort, char* page, char* thisData, char* dataEDS) {
  char inChar;
  char outBuf[150];
  char payloadTxweb[150];
  sprintf(payloadTxweb, "%s%s", page, dataEDS);
  Serial.println(F("    ---> Connecting to simplifik..."));
  int returnclienteconect = client.connect(domainBuffer, thisPort);
  if (returnclienteconect == 1) {
    Serial.println(F("      ---> Connected!!!"));
    // send the header
    sprintf(outBuf, "POST %s HTTP/1.1", payloadTxweb);
    client.println(outBuf);
    sprintf(outBuf, "Host: %s", domainBuffer);
    client.println(outBuf);
    client.println(F("Connection: close"));
    client.println(F("User-Agent: Simplifi-k-->Arduino/1.0"));
    client.println(F("Content-Type: application/x-www-form-urlencoded"));
    sprintf(outBuf, "Content-Length: %u\r\n", strlen(thisData));
    client.println(outBuf);
    client.println();
    // send the body (variables)
    client.print(thisData);
  } else {
    Serial.print(F("      ---> Return cliente conect simplifik: "));
    Serial.println(returnclienteconect);
    switch (returnclienteconect) {
      case 0:
        Serial.println(F(" Connection simplifik failed"));
        break;
      case -1:
        Serial.println(F(" Time out"));
        break;
      case -2:
        Serial.println(F(" Invalid server"));
        break;
      case -3:
        Serial.println(F(" Truncated"));
        break;
      case -4:
        Serial.println(F("Invalid response"));
        break;
      default:
        Serial.println(F(" Otro caso, sin conectarse con el cliente..."));
        break;
    }
    return 0;
  }
  int connectLoop = 0;
  Serial.println(F("........ Desde la WWW.............."));
  int index = 0;
  while (client.connected()) {
    while (client.available()) {
      inChar = client.read();
      Serial.write(inChar);
      //aca debe ir el codigo para saber si estan activos o no las valvulas
      // If it isn't a new line, add the character to the buffer:webstrgread
      /*if (inChar != '\n') { //'\r' o 13 o CR: retorno de carro //&& inChar != '\r'
        webstrgread[index++] = inChar;
        continue;
      }
      webstrgread[index] = 0;
      index = 0;
      if (inChar == 10) { //'\n' o 10 o LF: nueva linea
        switch (webcont) {
          case 1:
            webLine1 = webstrgread;
            webcont += 1;
            break;
          case 2:
            webLine2 = webstrgread;
            webcont += 1;
            break;
          case 3:
            webLine3 = webstrgread;
            webcont += 1;
            break;
          case 4:
            webLine4 = webstrgread;
            webcont += 1;
            break;
          case 5:
            webLine5 = webstrgread;
            webcont += 1;
            break;
          case 6:
            webLine6 = webstrgread;
            webcont += 1;
            break;
          case 7:
            webLine7 = webstrgread;
            webcont += 1;
            break;
          case 8:
            webLine8 = webstrgread;
            webcont += 1;
            break;
          case 9:
            webLine9 = webstrgread;
            webcont += 1;
            break;
          case 10:
            webLine10 = webstrgread;
            webcont += 1;
            break;
          default:
            // default is optional
            break;
        }//switch
      }//if(inChar == 10)*/
      //...................................................................*/

      connectLoop = 0;
    }//while client available
    delay(1);
    connectLoop++;
    if (connectLoop > 10000) {
      Serial.println();
      Serial.println(F("Timeout"));
      client.stop();
    }
    if (!client.connected())Serial.println(F("........ fin .........."));
  }//while client connected
  Serial.println();
  Serial.println(F("    ---> Disconnecting..."));
  /*Serial.print("Linea9--->");
  Serial.println(webLine9);*/
  client.stop();
  //aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
  /*
  //guardan valores para control del modulo 1
  //M01SS = webLine9[12]; // Determina valor del estado del modulo 1
  M01SS = "2";
  M01SPT = webLine9[14];    // Determina valor del Set point del modulo 1
  M01SPT += webLine9[15];
  M01SPT += webLine9[16];
  M01SPT += webLine9[17];
  //M01SPK1 = webLine9[19];   // Determina valor del Sprinkle del modulo 1
  M01SPK1 = "2";
  //M01SPK1 = webLine9[21];
  //guardan valores para control modulo 3
  //M03SS = webLine9[21]; // Determina valor del estado del modulo 2
  M03SS = "2";
  //M03SS = webLine9[12];
  M03SPT = webLine9[23];    // Determina valor del Set point del modulo 2
  M03SPT += webLine9[24];
  M03SPT += webLine9[25];
  M03SPT += webLine9[26];
  //M03SPK1 = webLine9[28];// Determina valor del Sprinkle del modulo 2
  M03SPK1 = "2";
  //M03SPK1 = webLine9[21];
  //Conviertiendo a estado de los aspersores
  if (M01SS == "2") {//<-- aca deberia ser un 2
    switch (M01SPK1.toInt()) {  // Determina valor del Spk del modulo 1
      case 1:
        payloadTx_mega_chibi_st01 = "o ";
        break;
      case 2:
        payloadTx_mega_chibi_st01 = "c ";
        break;
      default:
        // Si, no hay ninguno de los casos anteriores , no hace nada.
        break;
    }
  }
  if (M03SS == "2") {
    switch (M03SPK1.toInt()) {   // Determina valor del Spk del modulo 2
      case 1:
        payloadTx_mega_chibi_st03 = "o";
        break;
      case 2:
        payloadTx_mega_chibi_st03 = "c";
        break;
      default:
        // Si, no hay ninguno de los casos anteriores , no hace nada.
        break;
    }
  }
  payloadTx_mega_chibi = "1 ";
  payloadTx_mega_chibi += payloadTx_mega_chibi_st01;
  payloadTx_mega_chibi += payloadTx_mega_chibi_st03; // Genera la estructura para esto = "2 c c"

  Serial.print("State01_____: ");
  Serial.println(M01SS);
  Serial.print("Set_Point01_: ");
  Serial.println(M01SPT);
  Serial.print("Sprinkle01__: ");
  Serial.println(M01SPK1);
  Serial.print("State03_____: ");
  Serial.println(M03SS);
  Serial.print("Set_Point03_: ");
  Serial.println(M03SPT);
  Serial.print("Sprinkle03__: ");
  Serial.println(M03SPK1);
*/
  //client.stop();
  return 1;
  delay(1000);
}//postPage
void Imprime_millis() {
  Serial.print(F("\t"));
  Serial.print(F("thisMillis: "));
  Serial.println(thisMillis);
  Serial.print(F("\t"));
  Serial.print(F("lastMillis: "));
  Serial.println(lastMillis);
  Serial.print(F("\t"));
  Serial.print(F("delayMillis:"));
  Serial.println(delayMillis);
}//void Imprime_millis()

void Imprime_timestamp(){
  Serial.print(F("\t"));
  Serial.print(F("Timestamp: "));
  Serial.print(clock.dateFormat("U", dt));
  Serial.print(F("\t"));
  Serial.println(clock.dateFormat("d-m-Y H:i:s", dt));
}//void Imprime_timestamp()

void control_15(){
  Serial.println(F("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n"));
  Serial.print(F("Return EB         : "));
  Serial.println(REB);
  Serial.print(F("IP Address        : "));
  Serial.println(Ethernet.localIP());
  Serial.print(F("Subnet Mask       : "));
  Serial.println(Ethernet.subnetMask());
  Serial.print(F("Default Gateway IP: "));
  Serial.println(Ethernet.gatewayIP());
  Serial.print(F("DNS Server IP     : "));
  Serial.println(Ethernet.dnsServerIP());
  delay(1000);      // Esperar a que Ethernet se inicie
  Serial.println(F("                                    Ready"));
  Serial.println(F("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n"));
}//void control_1()
void resetear_chibi(){
  resetear = M1SS1.toInt() + M1SS2.toInt() + 
  M2SS1.toInt() + M2SS2.toInt() + 
  M3SS1.toInt() + M3SS2.toInt() + 
  M1S1T.toInt() + M1S1H.toInt() + 
  M1S2T.toInt() + M1S2H.toInt() + 
  M2S1T.toInt() + M2S1H.toInt() + 
  M2S2T.toInt() + M2S2H.toInt() + 
  M3S1T.toInt() + M3S1H.toInt() + 
  M3S2T.toInt() + M3S2H.toInt();
  Serial.print(F("\t\t\t\t\t\t"));
  Serial.print(F("Rst: "));
  Serial.println(resetear);
  if (resetear < 1){
    Serial.print(F("\t\t\t\t\t\t"));
    Serial.println(F("Rst: siiiiiiiii"));
    digitalWrite(RELAY01, RELAY_ON);   // Activa rele reset con bajo
    delay(1000);                       // espera un segundo
    digitalWrite(RELAY01, RELAY_OFF);  // Desactiva rele reset con bajo
  }
  delay(2000);//tiempo de espera para que chibi almacene los valores de los sensores
}
void cabecera_termino(char *line){
        //cabecera = line.indexOf("@|#|");
        //termino = line.indexOf("|#|$");
        if (cabecera > 0) {
          if (termino > 0) {
            complete3 = true;
          }else{
            Serial.println(F("reenvio3"));
          }
        } else {
          Serial.println(F("reenvio3"));
        }//if-else cabecera termino ok
}
void CloseSerial1(){
  digitalWrite (RELAY02, RELAY_ON); //rele 2 elevtrovalvula
  digitalWrite (RELAY03, RELAY_ON); //rele 3 bomba
}
void control_1(){
  Serial.println(F(" "));
  Serial.print(F("Controlmega Setup: "));
  Serial.println(controlmega);
}

