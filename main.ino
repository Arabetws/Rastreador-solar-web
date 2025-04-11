#include <WiFi.h>
#include <ESP32Servo.h>
#include <WebServer.h>

const char* ssid = "Placa Solar";
const char* password = "12345678";  // senha precisa ter no mínimo 8 caracteres

WebServer server(80);

Servo Horizontal;       
Servo Vertical; 

int ServoHorizontal = 90;   // Estabelece valor fixo à ServoHorizontal
int ServoVertical = 90;     // Estabelece valor fixo à ServoVertical

int LimiteServoHorizontalMax = 180;  // Estabele os limites de rotação
int LimiteServoHorizontalMin = 65;   // Estabele os limites de rotação

int LimiteServoVerticalMax = 120;    // Estabele os limites de rotação
int LimiteServoVerticalMin = 15;     // Estabele os limites de rotação

int LDRDC = 48;             // Inicia LDRDC no pino A0
int LDRDB = 47;             // Inicia LDRDB no pino A1
int LDREC = 42;             // Inicia LDREC no pino A2
int LDREB = 41;             // Inicia LDREB no pino A3

bool sistemaLigado = true;
bool reiniciarSistema = false;


String desligarAte = "00:00";
String estacaoAno = "Verao";
int diaDesligar = 0;
unsigned long ultimaVerificacaoHora = 0;


String getHTML() {
  String mainColor = sistemaLigado ? "#00ff00" : "#ff0000";
  String btnText = sistemaLigado ? "DESLIGAR SISTEMA" : "LIGAR SISTEMA";
  
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>CONTROLE SOLAR MATRIX</title>
  <style>
    body {
      background-color: #000000;
      font-family: 'Courier New', monospace;
      text-align: center;
      margin: 0;
      padding: 20px;
    }
    .container {
      border: 2px solid )rawliteral" + mainColor + R"rawliteral(;
      border-radius: 10px;
      max-width: 400px;
      margin: 30px auto;
      padding: 20px;
      box-shadow: 0 0 15px )rawliteral" + mainColor + R"rawliteral(;
      background-color: rgba(0, 0, 0, 0.7);
    }
    h1 {
      color: )rawliteral" + mainColor + R"rawliteral(;
      text-shadow: 0 0 5px )rawliteral" + mainColor + R"rawliteral(;
      letter-spacing: 2px;
    }
    .status {
      font-size: 24px;
      margin: 20px 0;
      color: )rawliteral" + mainColor + R"rawliteral(;
      text-shadow: 0 0 3px )rawliteral" + mainColor + R"rawliteral(;
    }
    button {
      background-color: #000000;
      border: 2px solid;
      padding: 15px 30px;
      margin: 10px 0;
      font-size: 16px;
      font-family: 'Courier New', monospace;
      cursor: pointer;
      border-radius: 5px;
      width: 100%;
      transition: all 0.3s;
      text-transform: uppercase;
    }
    button:hover {
      color: #000000;
      box-shadow: 0 0 20px;
    }
    .main-btn {
      color: )rawliteral" + mainColor + R"rawliteral(;
      border-color: )rawliteral" + mainColor + R"rawliteral(;
    }
    .main-btn:hover {
      background-color: )rawliteral" + mainColor + R"rawliteral(;
    }
    .config-btn {
      color: )rawliteral" + mainColor + R"rawliteral(;
      border-color: )rawliteral" + mainColor + R"rawliteral(;
    }
    .config-btn:hover {
      background-color: )rawliteral" + mainColor + R"rawliteral(;
    }
    .restart-btn {
      color: )rawliteral" + mainColor + R"rawliteral(;
      border-color: )rawliteral" + mainColor + R"rawliteral(;
    }
    .restart-btn:hover {
      background-color: )rawliteral" + mainColor + R"rawliteral(;
    }
    .info {
      margin: 15px 0;
      color: #ffffff;
      font-size: 16px;
    }
    .clock {
      color: #ffffff;
      margin: 15px 0;
      font-size: 18px;
    }
    #restartText {
      display: none;
      color: )rawliteral" + mainColor + R"rawliteral(;
      margin-top: 10px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>CONTROLE SOLAR</h1>
    
    <div class="status">STATUS: )rawliteral" + (sistemaLigado ? "LIGADO" : "DESLIGADO") + R"rawliteral(</div>
    
    <div class="clock">
      <span>HORA: </span>
      <span id="datetime">Carregando...</span>
    </div>
    
    <form action="/toggle" method="POST">
      <button type="submit" class="main-btn">)rawliteral" + btnText + R"rawliteral(</button>
    </form>
    
    <form action="/config" method="GET">
      <button type="submit" class="config-btn">CONFIGURAÇÕES</button>
    </form>
    
    <form action="/restart" method="POST" onsubmit="document.getElementById('restartText').style.display='block'; this.querySelector('button').disabled=true; this.querySelector('button').textContent='REINICIANDO...'; return true;">
      <button type="submit" class="restart-btn">REINICIAR SISTEMA</button>
      <div id="restartText">Reiniciando controle solar...</div>
    </form>
    
    <div class="info">
      <div>ÂNGULO HORIZONTAL: )rawliteral" + String(ServoHorizontal) + R"rawliteral(°</div>
      <div>ÂNGULO VERTICAL: )rawliteral" + String(ServoVertical) + R"rawliteral(°</div>
    </div>
  </div>
  
  <script>
    function updateClock() {
      const now = new Date();
      const time = now.toLocaleTimeString('pt-BR', {hour12: false});
      document.getElementById('datetime').innerText = time;
      setTimeout(updateClock, 1000);
    }
    updateClock();
  </script>
</body>
</html>
)rawliteral";

  return html;
}

String getConfigHTML() {
  String textColor = sistemaLigado ? "#00ff00" : "#ff0000";

  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>CONFIGURAÇÕES</title>
  <style>
    body {
      background-color: #000000;
      font-family: 'Courier New', monospace;
      text-align: center;
      margin: 0;
      padding: 20px;
      color: )rawliteral" + textColor + R"rawliteral(;
    }
    .container {
      border: 2px solid )rawliteral" + textColor + R"rawliteral(;
      border-radius: 10px;
      max-width: 400px;
      margin: 30px auto;
      padding: 20px;
      box-shadow: 0 0 15px )rawliteral" + textColor + R"rawliteral(;
      background-color: rgba(0, 0, 0, 0.7);
    }
    h1 {
      color: )rawliteral" + textColor + R"rawliteral(;
      text-shadow: 0 0 5px )rawliteral" + textColor + R"rawliteral(;
      letter-spacing: 2px;
    }
    .form-group {
      margin: 20px 0;
      text-align: left;
    }
    label {
      display: block;
      margin-bottom: 8px;
      color: )rawliteral" + textColor + R"rawliteral(;
    }
    input, select {
      background-color: #000000;
      color: )rawliteral" + textColor + R"rawliteral(;
      border: 1px solid )rawliteral" + textColor + R"rawliteral(;
      padding: 10px;
      width: 100%;
      font-family: 'Courier New', monospace;
      font-size: 16px;
      border-radius: 5px;
    }
    button {
      background-color: #000000;
      color: )rawliteral" + textColor + R"rawliteral(;
      border: 2px solid )rawliteral" + textColor + R"rawliteral(;
      padding: 10px 20px;
      margin: 10px 5px;
      font-size: 16px;
      font-family: 'Courier New', monospace;
      cursor: pointer;
      border-radius: 5px;
      transition: all 0.3s;
    }
    button:hover {
      background-color: )rawliteral" + textColor + R"rawliteral(;
      color: #000000;
      box-shadow: 0 0 15px )rawliteral" + textColor + R"rawliteral(;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>CONFIGURAÇÕES</h1>
    
    <form action="/saveconfig" method="POST">
    
      <div class="form-group">
        <label for="estacaoAno">Estação do ano:</label>
        <select id="estacaoAno" name="estacaoAno">
          <option value="Verao")rawliteral" + (estacaoAno == "Verao" ? " selected" : "") + R"rawliteral(>Verão</option>
          <option value="Outono")rawliteral" + (estacaoAno == "Outono" ? " selected" : "") + R"rawliteral(>Outono</option>
          <option value="Inverno")rawliteral" + (estacaoAno == "Inverno" ? " selected" : "") + R"rawliteral(>Inverno</option>
          <option value="Primavera")rawliteral" + (estacaoAno == "Primavera" ? " selected" : "") + R"rawliteral(>Primavera</option>
        </select>
      </div>      
      <button type="submit">SALVAR</button>
      <button type="button" onclick="window.location.href='/'">VOLTAR</button>
    </form>
  </div>
</body>
</html>
)rawliteral";

  return html;
}

void handleRoot() {
  server.send(200, "text/html", getHTML());
}

void handleConfig() {
  server.send(200, "text/html", getConfigHTML());
}

void handleSaveConfig() {
  estacaoAno = server.arg("estacaoAno");
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleToggle() {
  sistemaLigado = !sistemaLigado;
  
  if (!sistemaLigado) {
    // Posição de repouso quando desliga
    Horizontal.write(90);
    Vertical.write(45);
  }
  
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleRestart() {
  reiniciarSistema = true;
  server.sendHeader("Location", "/");
  server.send(303);
}
void setup() {
  Serial.begin(115200);
  delay(1000);

  Horizontal.attach(45);     // Inicia servo Horizontal na porta D3
  Vertical.attach(46);       // Inicia servo Vertical na porta D5

  Horizontal.write(180);    // Inicia servo Horizontal na posição 180
  Vertical.write(45);       // Inicia servo Horizontal na posição 45
  delay(3000); 

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("IP Address: ");
  Serial.println(IP);

  // Configura rotas do servidor web
  server.on("/toggle", HTTP_POST, handleToggle);
  
  server.begin();
  Serial.println("Servidor HTTP iniciado");

  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/saveconfig", HTTP_POST, handleSaveConfig);
  server.on("/toggle", HTTP_POST, handleToggle);
  server.on("/restart", HTTP_POST, handleRestart);
}

void loop() {
  server.handleClient();

  if(sistemaLigado){
    String currentTime = getCurrentTime();

    if(reiniciarSistema){
      reiniciarSistema = false;
    }
      int LDC = analogRead(LDRDC);      // Leitura Analógica do LDR Direita Cima
      int LEC = analogRead(LDREC);      // Leitura Analógica do LDR Esquerda Cima
      int LDB = analogRead(LDRDB);      // Leitura Analógica do LDR Direita Baixo
      int LEB = analogRead(LDREB);      // Leitura Analógica do LDR Esquerda Baixo
    
      int tol = 50;
    
      int ValorSup = (LDC + LEC) / 2;   // Média da leitura dos LDR superior
      int ValorInf = (LDB + LEB) / 2;   // Média da leitura dos LDR inferior
      int ValorDir = (LDC + LDB) / 2;   // Média da leitura dos LDR da direita
      int ValorEsq = (LEC + LEB) / 2;   // Média da leitura dos LDR da esquerda
    
      int DifSupInf = ValorSup - ValorInf;      // Diferença entre LED superior e inferior
      int DifDirEsq = ValorDir - ValorEsq;      // Diferença entre LED direita e esquerda
    
      // Realiza a leitura e executa os movimentos referente ao Servo Vertical
      if (-1 * tol > DifSupInf || DifSupInf > tol)  {
        if (ValorSup > ValorInf)  {
          ServoVertical = ++ServoVertical;
          if (ServoVertical > LimiteServoVerticalMax)  {
            ServoVertical = LimiteServoVerticalMax;
          }
        }
        else if (ValorSup < ValorInf)  {
          ServoVertical = --ServoVertical;
          if (ServoVertical < LimiteServoVerticalMin)  {
            ServoVertical = LimiteServoVerticalMin;
          }
        }
        Vertical.write(ServoVertical);
      }
    
      // Realiza a leitura e executa os movimentos referente ao Servo Horizontal
      if (-1 * tol > DifDirEsq || DifDirEsq > tol) {
        if (ValorDir > ValorEsq)  {
          ServoHorizontal = --ServoHorizontal;
          if (ServoHorizontal < LimiteServoHorizontalMin)  {
            ServoHorizontal = LimiteServoHorizontalMin;
          }
        }
        else if (ValorDir < ValorEsq)  {
          ServoHorizontal = ++ServoHorizontal;
          if (ServoHorizontal > LimiteServoHorizontalMax)  {
            ServoHorizontal = LimiteServoHorizontalMax;
          }
        }
        else if (ValorDir = ValorEsq)  {
        }
        Horizontal.write(ServoHorizontal);
      }
    
      delay(100);   // Aguarda 0,1 segundo
      
  }
}


String getCurrentTime() {
 //RTC ou NTP
  return "00:00";
}
