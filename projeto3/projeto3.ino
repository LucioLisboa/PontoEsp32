#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include "Credentials.h"
#include <MySQL_Generic.h>

//SDA pin 21, SCL pin 22
//teclado pin 19, 18, 5, 17, 16, 4, 0, 2

#define red 33
#define green 32
#define blue 34

#define MYSQL_DEBUG_PORT    Serial
#define _MYSQL_LOGLEVEL_    1

#define col  16
#define lin   2
#define ende  0x27

#define ROW_NUM     4
#define COLUMN_NUM  4

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM]      = {19, 18, 5, 17};
byte pin_column[COLUMN_NUM] = {16, 4, 0, 2};

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

LiquidCrystal_I2C lcd(ende,16,2);

IPAddress server(192, 168, 137, 1);

uint16_t server_port = 3306;

MySQL_Connection conn((Client *)&client);

// Create an instance of the cursor passing in the connection
MySQL_Query sql_query = MySQL_Query(&conn);
MySQL_Query *query_mem;

String F;
String aux;
unsigned int pos = 0;
byte flag = 0;
bool op =  false;
bool conf = false;
bool C = false;
String admin = "12345";

void setup(){
  Serial.begin(115200);
  while (!Serial && millis() < 5000); // wait for serial port to connect

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);

  lcd.init();
  lcd.clear();
  lcd.backlight(); 
  lcd.print("Conectando...");

  digitalWrite(red, HIGH);
  digitalWrite(green, HIGH);

  MYSQL_DISPLAY1("\nInicializando ", ARDUINO_BOARD);
  MYSQL_DISPLAY(MYSQL_MARIADB_GENERIC_VERSION);
  MYSQL_DISPLAY1("Conectando ao ponto de acesso:", ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    MYSQL_DISPLAY0(".");
  }

  // print out info about the connection:
  MYSQL_DISPLAY1("\nConectado a AP. Meu IP é:", WiFi.localIP());

  MYSQL_DISPLAY3("Conectando ao servidor SQL @", server, ", Port =", server_port);
  Serial.println("Iniciado");

  lcd.clear();
  lcd.println("Conectado!");
  digitalWrite(red, LOW);
  digitalWrite(green, LOW);
  delay(1000);
  lcd.clear();
}

void runInsert(){
  // Inicializa uma instancia da classe para a query
  MySQL_Query query_mem = MySQL_Query(&conn);

  if (conn.connected()){
    //Consulta SQL para realizar insert
    Serial.println(aux);
    Serial.println(F);
    String sql_query = "INSERT INTO projeto.usuarios (matricula, senha, autorizado) values (" + aux + ", " + F + ", 1);";
    MYSQL_DISPLAY(sql_query);

    if (!query_mem.execute(sql_query.c_str())){
      //Se a inserção sql falhou, avisa o usuario
      MYSQL_DISPLAY("Erro na inserção");
      lcd.setCursor(0, 0);
      lcd.print("Erro na inserção");
      digitalWrite(red, HIGH);
      delay(2000);
      lcd.clear();
      digitalWrite(red, LOW);
    }
    else{
      //Se a inserção sql foi efetuada com sucesso retorna um aviso
      MYSQL_DISPLAY("Dado inserido.");
      lcd.setCursor(0, 0);
      lcd.print("Usuario inserido");
      digitalWrite(green, HIGH);
      delay(2000);
      lcd.clear();
      digitalWrite(green, LOW);
    }
  }
  else{
    //Aviso de problema de conexão com o servidor
    MYSQL_DISPLAY("Desconectado do servidor. Impossivel inserir.");
    digitalWrite(red, HIGH);
    delay(2000);
    digitalWrite(red, LOW);
  }
}

void runSelect(){
  digitalWrite(red, LOW);
  digitalWrite(green, LOW);

  String query = "SELECT senha, autorizado FROM projeto.usuarios WHERE matricula = '" + aux + "'";

  MYSQL_DISPLAY(query);
  
  MySQL_Query query_mem = MySQL_Query(&conn);
  
  if ( !query_mem.execute(query.c_str()) )
  {
    MYSQL_DISPLAY("Querying error");
    return;
  }
  
  column_names *cols = query_mem.get_columns();
  
  // Read the rows and print them
  row_values *row = NULL;
  String senha;
  String aut;
  
  do{
    row = query_mem.get_next_row();
    if (row != NULL){
      senha = row->values[0];
      aut = row->values[1];
    }
  } while (row != NULL);

  lcd.clear();
  lcd.setCursor(0, 0);

  if(senha.equals(NULL)){
    lcd.print("Matricula nao");
    lcd.setCursor(0, 1);
    lcd.print("encontrada!");
    digitalWrite(red, HIGH);
    delay(2000);
    digitalWrite(red, LOW);
    lcd.clear();
    return;
  }

  if(F.equals(senha)){
    lcd.print("Senha Correta!");
    lcd.setCursor(0, 1);
    Serial.println("certo");
    if(aut == "1"){
      lcd.println("autorizado!");
      digitalWrite(green, HIGH);
    }
    else{
      lcd.print("nao autorizado!");
      digitalWrite(red, HIGH);
    }
    delay(2000);
    digitalWrite(green, LOW);
    digitalWrite(red, LOW);
  }else{
    digitalWrite(red, HIGH);
    lcd.print("Senha Incorreta!");
    Serial.println("errado");
    delay(2000);
    digitalWrite(red, LOW);
  }
  lcd.clear();
}

void loop(){
  char key = keypad.getKey();
  if(key != NO_KEY){
    switch (key){
      case 'A':
        op = false;
        flag = 0;
        F.clear();
        pos = 0;
        lcd.clear();
      break;
      case 'B':
        op = true;
        flag = 0;
        F.clear();
        pos = 0;
        lcd.clear();
      break;
      case 'C':
        C = true;
      break;
      case 'D':
        if(pos>0){
        F.remove(pos-1);
        pos--;
        lcd.clear();
        delay(50);
        lcd.setCursor(0, 1);
        lcd.print(F);
      break;
      default:
        pos++;
        lcd.setCursor(0, 1);
        F.concat(key);
        lcd.print(F);
      break;
      }
    }
  }
  if(op == false){
    if(flag == 0){
      if(C == false){
        lcd.setCursor(0, 0);
        lcd.print("matricula:");
        digitalWrite(green, HIGH);
        digitalWrite(red, HIGH);
      }
      else{
        aux=F;
        pos=0;
        F.clear();
        flag++;
        C = false;
        lcd.clear();
      }
    }
    else if(flag == 1){
      if(C == false){
        lcd.setCursor(0, 0);
        lcd.print("senha:");
        digitalWrite(green, HIGH);
        digitalWrite(red, HIGH);
      }
      else{
        Serial.println(aux);
        Serial.println(F);
        flag = 0;
        C = false;
        lcd.clear();

        MYSQL_DISPLAY("Enviando Dado...");
  
        if (conn.connectNonBlocking(server, server_port, user, password) != RESULT_FAIL){
          delay(500);
          runSelect();
          F.clear();
          conn.close(); 
        }
        else{
          MYSQL_DISPLAY("\nConnect failed. Trying again on next iteration.");
        }
      }
    }
  }
  else {
    if(flag == 0){
      if(C == false){
        lcd.setCursor(0, 0);
        lcd.print("Senha de admin:");
        digitalWrite(green, HIGH);
        digitalWrite(red, HIGH);
      }
      else{
        if(F.equals(admin)){
          F.clear();
          lcd.clear();
          flag++;
          C = false;
        }
        else{
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Senha incorreta!");
          digitalWrite(red, HIGH);
          delay(2000);
          C = false;
          lcd.clear();
        }
      }
    }
    else if(flag == 1){
      if(C == false){
        lcd.setCursor(0, 0);
        lcd.print("matricula:");
        digitalWrite(green, HIGH);
        digitalWrite(red, HIGH);
      }
      else{
        aux = F;
        pos = 0;
        F.clear();
        flag = 2;
        C = false;
        lcd.clear();
      }
    }
    else if(flag == 2){
      if(C == false){
        lcd.setCursor(0, 0);
        lcd.print("senha:");
      }
      else{
        Serial.println(aux);
        Serial.println(F);
        flag = 0;
        C = false;
        lcd.clear();

        MYSQL_DISPLAY("Conectando...");
  
        if (conn.connectNonBlocking(server, server_port, user, password) != RESULT_FAIL){
          delay(500);
          runInsert();
          F.clear();
          conn.close();
          op = false;
        }
        else{
          MYSQL_DISPLAY("\nConnect failed. Trying again on next iteration.");
        }
      }
    }
  }
}