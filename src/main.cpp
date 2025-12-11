#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

/**
 * PROJETO: Monitor Industrial com Gráfico Real-Time (Offline)
 * AUTOR: Eduardo dos Santos Rodrigues
 * MODO: Access Point (Cria rede Wi-Fi própria)
 * URL: http://192.168.4.1
 */

// --- Configurações de Wi-Fi ---
const char* ssid = "ESP32_Pressao"; 
const char* password = "12345678"; // SENHA CORRIGIDA (8 dígitos)

WebServer server(80);

// --- Hardware ---
const int NUM_SENSORES = 6;
const int PINOS_SENSORES[NUM_SENSORES] = {34, 35, 39, 36, 32, 33};
const float R_SHUNT = 150.0;
const float V_REF = 3.3;
const int ADC_RES = 4095;
const float PRESSAO_MIN = 0.5;
const float PRESSAO_MAX = 3.0;
const float ALPHA = 0.1;

float adc_filtrado[NUM_SENSORES];
float pressoes_finais[NUM_SENSORES];

// --- HTML/JS/CSS (Interface Gráfica) ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Monitor Industrial ESP32</title>
  <style>
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #eee; text-align: center; margin: 0; padding: 10px; }
    h2 { color: #333; margin-bottom: 5px; }
    
    /* Painel de Valores */
    .painel { display: grid; grid-template-columns: repeat(auto-fit, minmax(100px, 1fr)); gap: 10px; margin-bottom: 20px; }
    .card { background: white; padding: 10px; border-radius: 8px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
    .card h3 { margin: 0; font-size: 14px; color: #666; }
    .valor { font-size: 1.8rem; font-weight: bold; color: #007BFF; display: block; }
    .unidade { font-size: 0.8rem; color: #aaa; }

    /* Área do Gráfico */
    .grafico-container { background: white; padding: 10px; border-radius: 8px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); position: relative; height: 300px; }
    canvas { width: 100%; height: 100%; display: block; }
    
    /* Legenda */
    .legenda { margin-top: 5px; font-size: 12px; }
    .dot { height: 10px; width: 10px; display: inline-block; border-radius: 50%; margin-right: 3px; }

    /* Botões */
    .btn-area { margin-top: 20px; }
    button { padding: 12px 24px; font-size: 16px; border: none; border-radius: 4px; cursor: pointer; color: white; margin: 5px; }
    .btn-csv { background-color: #28a745; }
    .btn-csv:hover { background-color: #218838; }
    .btn-limpar { background-color: #dc3545; }
    
    .status { font-size: 12px; color: #666; margin-top: 10px; }
  </style>
</head>
<body>

  <h2>Monitor de Pressão</h2>
  
  <div class="painel" id="painel-sensores">
    </div>

  <div class="grafico-container">
    <canvas id="meuGrafico"></canvas>
  </div>
  
  <div class="legenda" id="legenda-grafico">
    </div>

  <div class="btn-area">
    <button class="btn-csv" onclick="baixarHistorico()">📥 Baixar CSV Completo</button>
    <button class="btn-limpar" onclick="limparHistorico()">🗑️ Limpar Dados</button>
  </div>
  
  <div class="status">Registros na memória: <span id="contador">0</span></div>

<script>
  // --- CONFIGURAÇÕES ---
  const CORES = ['#FF0000', '#0000FF', '#00CC00', '#FF9900', '#9900CC', '#00CCCC'];
  const MAX_PONTOS = 100; // Quantos pontos cabem na tela (largura do gráfico)
  
  // --- VARIÁVEIS GLOBAIS ---
  let historicoCSV = []; // Guarda TODOS os dados desde que abriu a página
  let dadosGrafico = [[], [], [], [], [], []]; // Guarda só os últimos 100 pontos para desenhar
  let canvas = document.getElementById('meuGrafico');
  let ctx = canvas.getContext('2d');

  // Inicializa Cards e Legenda
  const painel = document.getElementById('painel-sensores');
  const legenda = document.getElementById('legenda-grafico');
  
  for(let i=0; i<6; i++){
    // Cria Card
    painel.innerHTML += `
      <div class="card" style="border-top: 3px solid ${CORES[i]}">
        <h3>S${i+1}</h3>
        <span class="valor" id="val${i}">--</span>
        <span class="unidade">bar</span>
      </div>`;
      
    // Cria Legenda
    legenda.innerHTML += `<span style="margin-right:10px"><span class="dot" style="background:${CORES[i]}"></span>S${i+1}</span>`;
  }

  // Ajusta tamanho do canvas
  function resizeCanvas() {
    canvas.width = canvas.parentElement.clientWidth;
    canvas.height = canvas.parentElement.clientHeight;
  }
  window.addEventListener('resize', resizeCanvas);
  resizeCanvas();

  // --- FUNÇÃO DE DESENHO (LOOP) ---
  function desenharGrafico() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    // Desenha linhas de grade (0.5, 1.0, 1.5 ... 3.0)
    ctx.strokeStyle = '#e0e0e0';
    ctx.lineWidth = 1;
    ctx.font = "10px Arial";
    ctx.fillStyle = "#888";
    
    for(let p=0; p<=3.0; p+=0.5){
      let y = map(p, 0, 3.5, canvas.height, 0); // 3.5 para dar margem
      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(canvas.width, y);
      ctx.stroke();
      ctx.fillText(p.toFixed(1) + " bar", 5, y - 2);
    }

    // Desenha as 6 linhas dos sensores
    for(let s=0; s<6; s++){
      if(dadosGrafico[s].length < 2) continue;
      
      ctx.beginPath();
      ctx.strokeStyle = CORES[s];
      ctx.lineWidth = 2;
      
      for(let i=0; i<dadosGrafico[s].length; i++){
        let valor = dadosGrafico[s][i];
        let x = map(i, 0, MAX_PONTOS-1, 0, canvas.width);
        let y = map(valor, 0, 3.5, canvas.height, 0);
        
        if(i==0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      }
      ctx.stroke();
    }
  }
  
  // Função auxiliar de mapeamento
  function map(x, in_min, in_max, out_min, out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }

  // --- LOOP DE DADOS ---
  setInterval(() => {
    fetch('/dados')
      .then(resp => resp.json())
      .then(data => {
        let agora = new Date().toLocaleTimeString();
        let linhaCSV = [agora];
        
        // Processa os 6 sensores
        for(let i=0; i<6; i++){
          let val = parseFloat(data['s'+(i+1)]);
          
          // Atualiza Numero
          document.getElementById('val'+i).innerText = val.toFixed(2);
          
          // Atualiza Grafico (Remove antigo, põe novo)
          dadosGrafico[i].push(val);
          if(dadosGrafico[i].length > MAX_PONTOS) dadosGrafico[i].shift();
          
          linhaCSV.push(val.toFixed(2));
        }
        
        // Salva no histórico do navegador
        historicoCSV.push(linhaCSV);
        document.getElementById('contador').innerText = historicoCSV.length;
        
        desenharGrafico();
      })
      .catch(err => console.log("Erro conexão:", err));
  }, 500); // Atualiza a cada 500ms

  // --- EXPORTAR CSV ---
  function baixarHistorico() {
    if(historicoCSV.length === 0) { alert("Sem dados para salvar!"); return; }
    
    let csvContent = "data:text/csv;charset=utf-8,";
    csvContent += "Hora,Sensor1,Sensor2,Sensor3,Sensor4,Sensor5,Sensor6\n";
    
    historicoCSV.forEach(row => {
      csvContent += row.join(",") + "\n";
    });
    
    const encodedUri = encodeURI(csvContent);
    const link = document.createElement("a");
    link.setAttribute("href", encodedUri);
    let dataStr = new Date().toISOString().slice(0,19).replace(/:/g,"-");
    link.setAttribute("download", "Pressao_Log_" + dataStr + ".csv");
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  }
  
  function limparHistorico() {
    if(confirm("Tem certeza? Isso apaga os dados da memória do navegador.")){
      historicoCSV = [];
      dadosGrafico = [[],[],[],[],[],[]];
      document.getElementById('contador').innerText = 0;
    }
  }

</script>
</body>
</html>
)rawliteral";

void handleRoot() { server.send(200, "text/html", index_html); }

void handleDados() {
  String json = "{";
  for(int i=0; i<NUM_SENSORES; i++) {
    json += "\"s" + String(i+1) + "\": \"" + String(pressoes_finais[i], 2) + "\"";
    if(i < NUM_SENSORES - 1) json += ", ";
  }
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  analogSetAttenuation(ADC_11db);

  for(int i=0; i<NUM_SENSORES; i++) {
    pinMode(PINOS_SENSORES[i], INPUT);
    adc_filtrado[i] = analogRead(PINOS_SENSORES[i]);
  }

  // CONFIGURAÇÃO DO AP CORRIGIDA
  WiFi.softAP(ssid, password);
  
  Serial.println("\n--- SERVIDOR INICIADO ---");
  Serial.print("SSID: "); Serial.println(ssid);
  Serial.print("IP:   "); Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/dados", handleDados);
  server.begin();
}

void loop() {
  server.handleClient();

  for(int i=0; i<NUM_SENSORES; i++) {
    int raw = analogRead(PINOS_SENSORES[i]);
    adc_filtrado[i] = (ALPHA * raw) + ((1.0 - ALPHA) * adc_filtrado[i]);

    float tensao = (adc_filtrado[i] * V_REF) / ADC_RES;
    float corrente_mA = (tensao / R_SHUNT) * 1000.0;
    
    if (corrente_mA < 3.5) pressoes_finais[i] = 0.0; 
    else if (corrente_mA > 21.0) pressoes_finais[i] = PRESSAO_MAX; 
    else pressoes_finais[i] = PRESSAO_MIN + (corrente_mA - 4.0) * (PRESSAO_MAX - PRESSAO_MIN) / 16.0;
  }
  delay(5);
}