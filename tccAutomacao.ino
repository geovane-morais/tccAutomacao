#include <PID_v1.h>
#include <LiquidCrystal.h> 
#include <max6675.h>  
     
#define FERROdeSOLDA  11              //definindo saida como o pwm do SSR
#define Dled          2               //pino que acende o led do DISPLAY 16x2
#define vibrar        digitalRead(12) //define o pino do sensor de vibração   

/* (DEFINFIÇÃO DE VARIAVEIS) ============================================================================*/
double a=0,minutos=0,tempo=0,x=0,z=0; //para fazer o
double segundos=0,milisegundos=0;     //tempo do programa

double FERROparado=0,T=0,t=0;         //variaveis para desligar o ferro de solda
double tempoESTIMADO=10;              //tempo de desligar o ferro em minutos

double atualizacao=0,on_off=0,b=0;    //variaveis para o max6675
double taxa=220;                      // tempo para verificar o termo par em milisegundos
int j=0;                              //variavel para tirar bug do display
 
double SetPoint, error=0;                              //
double aggKp=10,aggKi=0,aggKd=0;                       //parametros
double consKp=10.4, consKi=0.30, consKd=0.01;          //do PID
double Setpoint=0, Input, Output,temperatura=0,PWM=0;  // 
   
/*= Criação de instâncias ===============================================================================*/
LiquidCrystal lcd          (A3,A2,6,5,4,3); //(RS, E, D4, D5, D6, D7)CRIA UMA INSTÂNCIA para o diplay LCD
MAX6675       thermocouple (8, 9, 10);      //CRIA UMA INSTÂNCIA UTILIZANDO OS PINOS (CLK, CS, SO)
PID           myPID        (&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);
/*======================================================================================================*/

void setup()
{
  Serial.begin(9600);                 //INICIALIZA A  COMUNICAÇÃO SERIAL EM 9600´bps
  pinMode(12,INPUT);                  //define o pino 12 como entrada para o sensor de vibração
  pinMode(Dled,OUTPUT);
  digitalWrite(Dled,HIGH);                 
  myPID.SetMode(AUTOMATIC);           //seta em automatico o PID
  lcd.begin(16, 2);                   //Define o número de colunas e linhas do LCD
  delay(1000);                        //delay para iniciar o programa
}

void loop()
{  
  temporizador();                     //função do Tempo para desligar o ferro de solda 
  setpoint();                         //função do Setpoint
  pid();                              //função para o PID
  LCD();                              //função do display
  MonitorSerial();                    //função do Monitor Serial 
}

int setpoint()
{
 SetPoint = (210 * analogRead(A4) /1023) +100;
 
 if ((SetPoint-Setpoint) <= 2.7 && (SetPoint-Setpoint) >= -2.7); //este if serve para tirar
 else Setpoint = SetPoint;                                       //o ruido do potenciometro
 
 Setpoint = round(Setpoint*10)/10;  //Aredondar o valor da temperatura
 
 return (Setpoint,SetPoint);
}

void MonitorSerial()
{
   Serial.print("Tempo: ");           //IMPRIME O TEXTO NO MONITOR SERIAL 
   Serial.print(minutos);             //IMPRIME NO MONITOR SERIAL O VALOR DE TEMPO EM MINUTOS
   Serial.print(":");                 //IMPRIME O TEXTO NO MONITOR SERIAL
   Serial.print(segundos);            //IMPRIME NO MONITOR SERIAL O VALOR DE TEMPO EM SEGUNDOS
   Serial.print("    Temperatura: "); //IMPRIME O TEXTO NO MONITOR SERIAL
   Serial.print(temperatura);         //IMPRIME NO MONITOR SERIAL A TEMPERATURA MEDIDA
   Serial.print("*C");                //IMPRIME O TEXTO NO MONITOR SERIAL
   Serial.print("    FERROdeSOLDA: ");//IMPRIME O TEXTO NO MONITOR SERIAL
   Serial.print(PWM);                 //IMPRIME NO MONITOR SERIAL O VALOR DA SAIDA
   Serial.print("   Svibrar: ");      //IMPRIME O TEXTO NO MONITOR SERIAL
   Serial.print(vibrar);              //IMPRIME NO MONITOR SERIAL O VALOR DO SENSOR DE VIBRAÇÃO
   Serial.print("    Set-point: ");   //IMPRIME O TEXTO NO MONITOR SERIAL
   Serial.print(Setpoint);            //IMPRIME NO MONITOR SERIAL O VALOR DO SETPOINT
   Serial.println("*C");              //IMPRIME O TEXTO NO MONITOR SERIAL
}

void LCD()
{
  if (temperatura < 100 && j==0)  //Tem como objetivo apenas
  {                               //limpar a tela quando
    lcd.clear();                  //o valor da temperatura
    j=1;                          //descer de 100*C para 99*C
  }                               //
  else if (temperatura > 100)j=0; //modo a evitar bug
  
  lcd.setCursor(0, 0);      //(linha 0 x coluna 0)
  lcd.print("Set-Point:");  //imprime a palavra Set-Point
  lcd.setCursor(11, 0);     //(linha 11 x coluna 0)
  lcd.print(Setpoint,0);    //imprime a variavel 'setpoint'
  lcd.print("*C");          //imprime a palavra *C
  lcd.setCursor(0, 1);      //(linha 0 x coluna 1)     
  lcd.print("Temp-Real:");  //imprime a palavra Temp-Real
  lcd.setCursor(11, 1);     //(linha 11 x coluna 1)     
  lcd.print(temperatura,0); //imprime a variavel 'Temperatura'
  lcd.print("*C");          //imprime a palavra *C
}

int temporizador()
{
  x++;
  z=x*129.87;        
  milisegundos = z;           //tempo em milisegundos desde quando o programa inicia
  tempo = milisegundos/1000;  //tempo em segundos desde quando o programa inicia
  segundos =  tempo - a;      //tempo em segundos de (0-60)
  
  if (segundos > 60.10)
   {
    minutos++;                //tempo em minutos desde quando o programa inicia
    a=a+60;                   //varialvel que faz a variavel segundos funcionar
    }             
        
        /*deligar o ferro de solda*/   
  if (vibrar == 0)            //salva o tempo do ferro
   T = minutos - t;           //de solda parado
  else                        
  {
    t = minutos;              //reseta caso haja
    T = 0;                    //movimento
  }   

  //quando T chega no valor estimado desliga o ferro de solda
  if (T > (tempoESTIMADO-1)) FERROparado=1; //manuseando o equipamento        
  else FERROparado=0;                       //equipamento parado
      
      /*TAXA de atualização para o MAX6675*/
  atualizacao = milisegundos - b;
  if (atualizacao < taxa) on_off = 0;
  else 
  {
    on_off = 1;
    b = b+taxa;
  } 
  
  return (segundos,milisegundos,FERROparado,on_off); 
}
/* (FUNÇÃO para o Monitor Serial) ========================================================================*/
int pid()
{
  error = Setpoint-temperatura;
  if(on_off == 1) 
  temperatura = thermocouple.readCelsius();  //Atribui a temperatura o valor do termopar
  Input = temperatura;                       //entrada que vai pro PID é "temperatura"
  
  if (error < 20)
  myPID.SetTunings(consKp, consKi, consKd);  //ativa o PID constante 
  else 
  myPID.SetTunings(aggKp, aggKi, aggKd);     //ativa o PID agressivo
  
  myPID.Compute();                           //Comando da biblioteca PID (calcula o PID)
  analogWrite(FERROdeSOLDA,PWM);             //manda PWM acionar o ferro de solda
  
  if (FERROparado==0)PWM=Output;             //quando o ferro estiver em uso
  else PWM=0;                                //PWM é o valor do calculo do PID
  temperatura = round(temperatura*10)/10;    //Aredondar o valor da temperatura
  
  return(temperatura,PWM);
}
