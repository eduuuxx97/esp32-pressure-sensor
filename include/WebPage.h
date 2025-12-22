#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <Arduino.h>

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

    /* Área de Controle (SD Card) */
    .painel-controle { margin-top: 20px; background: #fff; padding: 15px; border-radius: 8px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
    .btn-area { display: flex; flex-wrap: wrap; justify-content: center; gap: 10px; margin-top: 10px; }
    
    button { padding: 12px 24px; font-size: 14px; border: none; border-radius: 4px; cursor: pointer; color: white; transition: 0.3s; }
    .btn-download { background-color: #28a745; }
    .btn-download:hover { background-color: #218838; }
    .btn-delete { background-color: #dc3545; }
    .btn-delete:hover { background-color: #c82333; }
    
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
  
  <div class="legenda" id="legenda-grafico"></div>

  <div class="painel-controle">
    <h3>📂 Datalogger (Cartão SD)</h3>
    <div class="btn-area">
      <a href="/download" style="text-decoration: none;">
        <button class="btn-download">📥 Baixar Histórico Completo (.csv)</button>
      </a>
      
      <a href="/delete" onclick="return confirm('ATENÇÃO: Isso apagará todos os dados do cartão SD permanentemente. Continuar?')" style="text-decoration: none;">
        <button class="btn-delete">🗑️ Limpar Memória do Cartão</button>
      </a>
    </div>
    <div class="status">O sistema grava automaticamente a cada 2 segundos.</div>
  </div>

<script>
  // --- CONFIGURAÇÕES ---
  const CORES = ['#FF0000', '#0000FF', '#00CC00', '#FF9900', '#9900CC', '#00CCCC'];
  const MAX_PONTOS = 100; // Largura do gráfico
  
  // --- VARIÁVEIS ---
  let dadosGrafico = [[], [], [], [], [], []]; 
  let canvas = document.getElementById('meuGrafico');
  let ctx = canvas.getContext('2d');

  // Inicializa Cards e Legenda
  const painel = document.getElementById('painel-sensores');
  const legenda = document.getElementById('legenda-grafico');
  
  for(let i=0; i<6; i++){
    painel.innerHTML += `
      <div class="card" style="border-top: 3px solid ${CORES[i]}">
        <h3>S${i+1}</h3>
        <span class="valor" id="val${i}">--</span>
        <span class="unidade">bar</span>
      </div>`;
      
    legenda.innerHTML += `<span style="margin-right:10px"><span class="dot" style="background:${CORES[i]}"></span>S${i+1}</span>`;
  }

  // Ajusta tamanho do canvas
  function resizeCanvas() {
    canvas.width = canvas.parentElement.clientWidth;
    canvas.height = canvas.parentElement.clientHeight;
  }
  window.addEventListener('resize', resizeCanvas);
  resizeCanvas();

  // --- FUNÇÃO DE DESENHO ---
  function desenharGrafico() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    // Grade
    ctx.strokeStyle = '#e0e0e0';
    ctx.lineWidth = 1;
    ctx.font = "10px Arial";
    ctx.fillStyle = "#888";
    
    for(let p=0; p<=3.0; p+=0.5){
      let y = map(p, 0, 3.5, canvas.height, 0); 
      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(canvas.width, y);
      ctx.stroke();
      ctx.fillText(p.toFixed(1), 5, y - 2);
    }

    // Linhas
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
  
  function map(x, in_min, in_max, out_min, out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }

  // --- LOOP DE DADOS ---
  setInterval(() => {
    fetch('/dados')
      .then(resp => resp.json())
      .then(data => {
        for(let i=0; i<6; i++){
          let val = parseFloat(data['s'+(i+1)]);
          
          // Atualiza Numero
          document.getElementById('val'+i).innerText = val.toFixed(2);
          
          // Atualiza Grafico
          dadosGrafico[i].push(val);
          if(dadosGrafico[i].length > MAX_PONTOS) dadosGrafico[i].shift();
        }
        desenharGrafico();
      })
      .catch(err => console.log("Erro conexão:", err));
  }, 500); // 500ms

</script>
</body>
</html>
)rawliteral";

#endif