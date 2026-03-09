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
    body { font-family: 'Segoe UI', Tahoma, sans-serif; background: #eee; text-align: center; margin: 0; padding: 10px; }
    .painel-box { background: white; padding: 15px; border-radius: 8px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); margin-bottom: 15px; }
    .timer-display { font-size: 2.5rem; font-weight: bold; color: #333; margin: 10px 0; font-family: monospace; }
    .input-group { display: flex; justify-content: center; gap: 5px; margin-bottom: 15px; }
    .input-wrapper { display: flex; flex-direction: column; }
    .input-wrapper label { font-size: 10px; color: #666; margin-bottom: 2px; }
    input[type=number] { padding: 8px; font-size: 16px; width: 60px; text-align: center; border: 1px solid #ccc; border-radius: 4px; }
    .btn-area { display: flex; justify-content: center; gap: 10px; }
    button { padding: 12px 20px; font-size: 16px; border: none; border-radius: 4px; cursor: pointer; color: white; transition: 0.3s; min-width: 100px; font-weight: bold; }
    .btn-start { background-color: #28a745; }
    .btn-pause { background-color: #ffc107; color: #333; }
    .btn-stop { background-color: #dc3545; }
    .btn-blue { background-color: #007BFF; }
    .btn-restart { background-color: #6c757d; }
    button:disabled { background-color: #ccc; cursor: not-allowed; }
    .grid-sensores { display: grid; grid-template-columns: repeat(auto-fit, minmax(80px, 1fr)); gap: 10px; }
    .card { background: #f9f9f9; padding: 5px; border-radius: 5px; border: 1px solid #ddd; }
    .valor { font-size: 1.2rem; font-weight: bold; color: #007BFF; }
    .grafico-container { position: relative; height: 250px; margin-top: 10px; }
    canvas { width: 100%; height: 100%; }
    
    /* Estilo da Caixa de Temperatura */
    .temp-container { background: #38bfc4; color: white; padding: 10px; border-radius: 8px; margin-top: 5px; font-weight: bold; font-size: 1.2rem; }
    
    .status-bar { display: flex; align-items: center; justify-content: center; gap: 20px; padding: 5px; margin-bottom: 10px; }
    .led-item { display: flex; align-items: center; gap: 8px; font-size: 13px; font-weight: bold; }
    .led { width: 12px; height: 12px; border-radius: 50%; background: gray; border: 1px solid #333; }
    .led-green { background: #28a745; box-shadow: 0 0 8px #28a745; }
    .led-red { background: #dc3545; box-shadow: 0 0 8px #dc3545; }
  </style>
</head>
<body>

  <h2>Monitor Industrial - LSA</h2>

  <div class="painel-box">
    <div class="status-bar">
      <div class="led-item"><div id="wifi-led" class="led"></div><span>CONEXÃO</span></div>
      <div class="led-item"><div id="sd-led" class="led"></div><span>CARTÃO SD</span></div>
    </div>
    <div class="temp-container">
      🌡️ Temperatura: <span id="valTemp">--</span>°C
    </div>
  </div>

  <div class="painel-box">
    <h3>⏱️ Controle de Experimento</h3>
    <div class="timer-display" id="display-tempo">PARADO</div>
    <div class="input-group" id="inputs-tempo">
      <div class="input-wrapper"><label>Dias</label><input type="number" id="dias" value="0"></div>
      <div class="input-wrapper"><label>Horas</label><input type="number" id="horas" value="0"></div>
      <div class="input-wrapper"><label>Mins</label><input type="number" id="mins" value="60"></div>
      <div class="input-wrapper"><label>Log (seg)</label><input type="number" id="intervalo" value="2"></div>
    </div>
    <div class="btn-area">
      <button id="btn-acao" class="btn-start" onclick="acaoPrincipal()">▶ INICIAR</button>
      <button id="btn-stop" class="btn-stop" onclick="pararComProtecao()" disabled>⏹ PARAR</button>
      <button class="btn btn-restart" onclick="if(confirm('Reiniciar hardware?')) fetch('/restart')">🔄 RESET</button>
    </div>
  </div>

  <div class="painel-box">
    <div class="grid-sensores" id="grid-sensores"></div>
  </div>

  <div class="painel-box">
    <div class="grafico-container"><canvas id="meuGrafico"></canvas></div>
  </div>

<script>
  let estadoAtual = "PARADO";
  const CORES = ['#FF0000', '#0000FF', '#00CC00', '#FF9900', '#9900CC', '#00CCCC'];
  const MAX_PONTOS = 60;
  let dadosGrafico = [[], [], [], [], [], []];
  let canvas = document.getElementById('meuGrafico');
  let ctx = canvas.getContext('2d');

  const grid = document.getElementById('grid-sensores');
  for(let i=0; i<6; i++){
    grid.innerHTML += `<div class="card" style="border-top: 3px solid ${CORES[i]}"><small>S${i+1}</small><br><span class="valor" id="val${i}">--</span></div>`;
  }

  function acaoPrincipal() {
    if (estadoAtual === "PARADO") {
      let d = parseInt(document.getElementById('dias').value) || 0;
      let h = parseInt(document.getElementById('horas').value) || 0;
      let m = parseInt(document.getElementById('mins').value) || 0;
      let inter = parseInt(document.getElementById('intervalo').value) || 2; 
      let totalMinutos = (d * 1440) + (h * 60) + m;
      let agora = new Date();
      let params = `?tempo=${totalMinutos}&intervalo=${inter}&ano=${agora.getFullYear()}&mes=${agora.getMonth()+1}&dia=${agora.getDate()}&h=${agora.getHours()}&m=${agora.getMinutes()}&s=${agora.getSeconds()}`;
      fetch('/start' + params);
    } else if (estadoAtual === "RODANDO") fetch('/pause'); else if (estadoAtual === "PAUSADO") fetch('/resume');
  }

  function pararComProtecao() { if (confirm("⚠️ ENCERRAR?")) fetch('/stop'); }

  setInterval(() => {
    fetch('/dados').then(r => {
      if(r.ok) { document.getElementById('wifi-led').className = 'led led-green'; return r.json(); }
      throw new Error();
    }).then(data => {
        // Atualiza as pressões
        for(let i=0; i<6; i++){
          let val = parseFloat(data['s'+(i+1)]);
          document.getElementById('val'+i).innerText = val.toFixed(2);
          dadosGrafico[i].push(val);
          if(dadosGrafico[i].length > MAX_PONTOS) dadosGrafico[i].shift();
        }

        // ATUALIZA A TEMPERATURA
        if(data.temp !== undefined) {
            document.getElementById('valTemp').innerText = data.temp.toFixed(1);
        }

        document.getElementById('sd-led').className = data.sdStatus ? "led led-green" : "led led-red";
        
        let display = document.getElementById('display-tempo');
        let btnAcao = document.getElementById('btn-acao');
        let btnStop = document.getElementById('btn-stop');
        
        if(data.pausado) estadoAtual = "PAUSADO"; 
        else if(data.rodando) estadoAtual = "RODANDO"; 
        else estadoAtual = "PARADO";

        if (estadoAtual === "RODANDO") {
          display.innerText = data.status; display.style.color = "#28a745";
          btnAcao.innerText = "⏸ PAUSAR"; btnAcao.className = "btn-pause"; btnStop.disabled = false;
        } else if (estadoAtual === "PAUSADO") {
          display.innerText = "PAUSADO"; display.style.color = "#ffc107";
          btnAcao.innerText = "▶ RETOMAR"; btnAcao.className = "btn-start"; btnStop.disabled = false;
        } else {
          display.innerText = "PRONTO"; display.style.color = "#333";
          btnAcao.innerText = "▶ INICIAR"; btnAcao.className = "btn-start"; btnStop.disabled = true;
        }
        desenharGrafico();
    }).catch(() => { document.getElementById('wifi-led').className = 'led led-red'; });
  }, 1000);

  function resizeCanvas() { canvas.width = canvas.parentElement.clientWidth; canvas.height = canvas.parentElement.clientHeight; }
  window.addEventListener('resize', resizeCanvas); resizeCanvas();
  function map(v, i_min, i_max, o_min, o_max) { return (v - i_min) * (o_max - o_min) / (i_max - i_min) + o_min; }

  function desenharGrafico() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.strokeStyle = '#eee';
    for(let p=0; p<=3.0; p+=0.5){
      let y = map(p, 0, 3.5, canvas.height, 0);
      ctx.beginPath(); ctx.moveTo(0,y); ctx.lineTo(canvas.width,y); ctx.stroke();
    }
    for(let s=0; s<6; s++){
      if(dadosGrafico[s].length < 2) continue;
      ctx.beginPath(); ctx.strokeStyle = CORES[s]; ctx.lineWidth = 2;
      for(let i=0; i<dadosGrafico[s].length; i++){
        let x = map(i, 0, MAX_PONTOS-1, 0, canvas.width);
        let y = map(dadosGrafico[s][i], 0, 3.5, canvas.height, 0);
        if(i==0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
      }
      ctx.stroke();
    }
  }
</script>
</body>
</html>
)rawliteral";
#endif