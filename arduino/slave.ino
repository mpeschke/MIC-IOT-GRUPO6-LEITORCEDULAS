#include <Wire.h>

// Porta no I2C bus para comunicação entre o microprocessador e o microcontrolador.
#define I2C_BUS_PORT 0x04

// Entrada do sensor IR.
const int GPI_IR = 13;

// Entradas e saídas do sensor RGB TCS230.
const int GPO_TCS230_S2 = 12;
const int GPO_TCS230_S3 = 11;

// Output.
const int GPI_TCS230_OUT = 10;

const int GPO_TCS230_S0 = 9;
const int GPO_TCS230_S1 = 8;
const int GPO_TCS230_OE = 7;

// Para o Arduíno a recomendação do fabricante é utilizar
// 20% da energia apenas.
const int FS_S0 = HIGH;
const int FS_S1 = LOW;

// Frequências capturadas pelo TCS230 em RGB.
int vermFreq = 0;
int verdFreq = 0;
int azulFreq = 0;

// Variáveis de controle na comunicação I2C.

// Intervalo em que o microprocessador solicita informações do microcontrolador (em ms).
const int tick = 3000;
// Valor da cédula. Caso zero, não houve leitura. Caso 1, cédula indefinida.
volatile int novaleitura = 0;
// Controla a execução da leitura de acordo com solicitação do master I2C.
volatile int runthread = 0;
// Tanto a thread principal como as interrupções gravam mensagens
// no leitor serial. Dessa forma, precisamos garantir que as mensagens nas threads
// sejam enviadas com um único Serial.println().
//           |10       |20       |30       |40       |50
//  1234567890123456789012345678901234567890123456789
// "R=9999999,G=9999999,B=9999999 (cor INDEFINIDA)"
char mainthrbuff[46];
const char* const mainthrbuffformat = "R=%4d,G=%4d,B=%4d (cor %s)";
//           |10       |20       |30       |40       |50
//  12345678901234567890123456789012345678901234567890123456789
// "Microprocessador ativo solicitando dados na porta 999: 999"
char intthrebuff[58];
const char* const intthrebuffformat = "Microprocessador ativo solicitando dados na porta %4d: %4d";

void setup() {
  // Entradas
  pinMode(GPI_IR, INPUT);
  pinMode(GPI_TCS230_OUT, INPUT);
  // Saídas
  pinMode(GPO_TCS230_S0, OUTPUT);
  pinMode(GPO_TCS230_S1, OUTPUT);
  pinMode(GPO_TCS230_S2, OUTPUT);
  pinMode(GPO_TCS230_S3, OUTPUT);
  // Inicia o TCS230 ligado.
  powerUpTCS230();
  // Habilita a leitura serial para medir as frequências 
  // capturadas pelo TCS230. 
  Serial.begin(9600);

  // Arduíno se registra no bus I2C como slave na porta indicada.
  Wire.begin(I2C_BUS_PORT);

  // Registra os eventos (na verdade, as interrupções que o microntrolador
  // irá receber/causar) durante a comunicação I2C.                
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

void loop()
{
  if(runthread > 0)
    run();
  delay(tick);  
}

void powerUpTCS230() {
  digitalWrite(GPO_TCS230_S0, FS_S0);
  digitalWrite(GPO_TCS230_S1, FS_S1);
}

void receiveEvent(int bytecount) {
  while (Wire.available())
  {
    memset(intthrebuff, 0, sizeof(intthrebuff));
    sprintf(intthrebuff, intthrebuffformat, I2C_BUS_PORT, Wire.read());
    Serial.println(intthrebuff);
    runthread = bytecount;
  }
}

void requestEvent()
{
  Wire.write(novaleitura);
  novaleitura = 0;
}

void run() {
  // Sensor IR detectou a passagem de uma nota (fluxo interrompido).
  if(digitalRead(GPI_IR) > 0)
  {
    // Lê a cor da nota.
    readRGB();
    // Comparação das frequências (a cor predominante é a menor frequência).
    memset(mainthrbuff, 0, sizeof(mainthrbuff));
    if(vermFreq < verdFreq && vermFreq < azulFreq)
    {
      sprintf(mainthrbuff, mainthrbuffformat, vermFreq, verdFreq, azulFreq, "VERMELHA");
      Serial.println(mainthrbuff);
      novaleitura = 10; // Cédula de 10 reais.
    }
    else if(verdFreq < vermFreq && verdFreq < azulFreq)
    {
      sprintf(mainthrbuff, mainthrbuffformat, vermFreq, verdFreq, azulFreq, "VERDE");
      Serial.println(mainthrbuff);
    }
    else if(azulFreq < verdFreq && azulFreq < vermFreq)
    {
      sprintf(mainthrbuff, mainthrbuffformat, vermFreq, verdFreq, azulFreq, "AZUL");
      Serial.println(mainthrbuff);
      novaleitura = 2; // Cédula de 2 reais.
    }
    else
    {
      sprintf(mainthrbuff, mainthrbuffformat, vermFreq, verdFreq, azulFreq, "INDEFINIDA");
      Serial.println(mainthrbuff);
      novaleitura = 1; // Não foi possível identificar a cédula por tonalidade RGB.
    }
  }
}

// Lê as três frequências RGB.
void readRGB() {
  readR();
  delay(250);
  readG();
  delay(250);
  readB();
}

void readR() {
  // Prepara os fotodiodos vermelhos para leitura.
  digitalWrite(GPO_TCS230_S2,LOW);
  digitalWrite(GPO_TCS230_S3,LOW);
  
  // Lê a frequência para o vermelho.
  vermFreq = pulseIn(GPI_TCS230_OUT, LOW);
}

void readG() {
  // Prepara os fotodiodos verdes para leitura.
  digitalWrite(GPO_TCS230_S2,HIGH);
  digitalWrite(GPO_TCS230_S3,HIGH);
  
  // Lê a frequência para o vermelho.
  verdFreq = pulseIn(GPI_TCS230_OUT, LOW);
}

void readB() {
  // Prepara os fotodiodos azuis para leitura.
  digitalWrite(GPO_TCS230_S2,LOW);
  digitalWrite(GPO_TCS230_S3,HIGH);
  
  // Lê a frequência para o vermelho.
  azulFreq = pulseIn(GPI_TCS230_OUT, LOW);
}
