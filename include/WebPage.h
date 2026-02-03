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
    .painel-box { background: white; padding: 15px; border-radius: 8px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); margin-bottom: 15px; }
    .timer-display { font-size: 2.5rem; font-weight: bold; color: #333; margin: 10px 0; font-family: monospace; }
    .input-group { display: flex; justify-content: center; gap: 5px; margin-bottom: 15px; }
    .input-wrapper { display: flex; flex-direction: column; }
    .input-wrapper label { font-size: 10px; color: #666; margin-bottom: 2px; }
    input[type=number] { padding: 8px; font-size: 16px; width: 60px; text-align: center; border: 1px solid #ccc; border-radius: 4px; }
    .btn-area { display: flex; justify-content: center; gap: 10px; }
    button { padding: 12px 20px; font-size: 16px; border: none; border-radius: 4px; cursor: pointer; color: white; transition: 0.3s; min-width: 100px; }
    .btn-start { background-color: #28a745; }
    .btn-pause { background-color: #ffc107; color: #333; }
    .btn-stop { background-color: #dc3545; }
    .btn-blue { background-color: #007BFF; }
    button:disabled { background-color: #ccc; cursor: not-allowed; }
    .grid-sensores { display: grid; grid-template-columns: repeat(auto-fit, minmax(80px, 1fr)); gap: 10px; }
    .card { background: #f9f9f9; padding: 5px; border-radius: 5px; border: 1px solid #ddd; }
    .valor { font-size: 1.2rem; font-weight: bold; color: #007BFF; }
    .grafico-container { position: relative; height: 250px; }
    canvas { width: 100%; height: 100%; }
    
    /* Estilo do LED do SD */
    .sd-container { display: flex; align-items: center; justify-content: center; gap: 10px; padding: 5px; }
    #sd-led { width: 12px; height: 12px; border-radius: 50%; background: gray; border: 1px solid #333; }
  </style>
</head>
<body>

  <h2>Monitor Industrial V2</h2>

  <div class="painel-box">
    <div class="sd-container">
      <div id="sd-led"></div>
      <span id="sd-msg" style="font-size: 14px; font-weight: bold;">Verificando SD...</span>
    </div>
  </div>

  <div class="painel-box">
    <h3>⏱️ Controle de Experimento</h3>
    <div class="timer-display" id="display-tempo">PARADO</div>
    <div class="input-group" id="inputs-tempo">
      <div class="input-wrapper"><label>Dias</label><input type="number" id="dias" value="0" min="0"></div>
      <div class="input-wrapper"><label>Horas</label><input type="number" id="horas" value="0" min="0"></div>
      <div class="input-wrapper"><label>Mins</label><input type="number" id="mins" value="60" min="0"></div>
      <div class="input-wrapper"><label>Log (seg)</label><input type="number" id="intervalo" value="2" min="1"></div>
    </div>
    <div class="btn-area">
      <button id="btn-acao" class="btn-start" onclick="acaoPrincipal()">▶ INICIAR</button>
      <button id="btn-stop" class="btn-stop" onclick="pararComProtecao()" disabled>⏹ PARAR</button>
    </div>
  </div>

  <div class="painel-box">
    <div class="grid-sensores" id="grid-sensores"></div>
  </div>

  <div class="painel-box">
    <div class="grafico-container"><canvas id="meuGrafico"></canvas></div>
  </div>

  <div class="painel-box">
    <a href="/download"><button class="btn-blue">📥 Baixar CSV</button></a>
    <a href="/delete" onclick="return confirm('ATENÇÃO: Isso apagará PERMANENTEMENTE os dados do cartão SD. Tem certeza?')"><button class="btn-blue" style="background:#6c757d">🗑️ Limpar</button></a>
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
      let params = `?tempo=${totalMinutos}&intervalo=${inter}`;
      params += `&ano=${agora.getFullYear()}&mes=${agora.getMonth() + 1}&dia=${agora.getDate()}`;
      params += `&h=${agora.getHours()}&m=${agora.getMinutes()}&s=${agora.getSeconds()}`;
      fetch('/start' + params).then(r => console.log("Start OK"));
    } else if (estadoAtual === "RODANDO") { fetch('/pause'); }
      else if (estadoAtual === "PAUSADO") { fetch('/resume'); }
  }

  function pararComProtecao() {
    if (confirm("⚠️ ENCERRAR experimento?")) { fetch('/stop'); }
  }

  setInterval(() => {
    fetch('/dados').then(resp => resp.json()).then(data => {
        // Atualiza Sensores
        for(let i=0; i<6; i++){
          let val = parseFloat(data['s'+(i+1)]);
          document.getElementById('val'+i).innerText = val.toFixed(2);
          dadosGrafico[i].push(val);
          if(dadosGrafico[i].length > MAX_PONTOS) dadosGrafico[i].shift();
        }
        
        // ATUALIZAÇÃO DO LED DO SD
        const led = document.getElementById('sd-led');
        const msg = document.getElementById('sd-msg');
        if (data.sdStatus === true) {
          led.style.background = "#28a745";
          msg.innerText = "Cartão SD: PRONTO";
          msg.style.color = "#28a745";
        } else {
          led.style.background = "#dc3545";
          msg.innerText = "Cartão SD: NÃO DETECTADO";
          msg.style.color = "#dc3545";
        }

        let display = document.getElementById('display-tempo');
        let btnAcao = document.getElementById('btn-acao');
        let btnStop = document.getElementById('btn-stop');
        let inputs = document.getElementById('inputs-tempo');
        
        if(data.pausado === true) estadoAtual = "PAUSADO";
        else if(data.rodando === true) estadoAtual = "RODANDO";
        else estadoAtual = "PARADO";

        if (estadoAtual === "RODANDO") {
          display.innerText = data.status; display.style.color = "#28a745";
          btnAcao.innerText = "⏸ PAUSAR"; btnAcao.className = "btn-pause";
          btnStop.disabled = false; inputs.style.opacity = "0.5";
        } else if (estadoAtual === "PAUSADO") {
          display.innerText = "PAUSADO"; display.style.color = "#ffc107";
          btnAcao.innerText = "▶ RETOMAR"; btnAcao.className = "btn-start";
          btnStop.disabled = false;
        } else {
          display.innerText = "PRONTO"; display.style.color = "#333";
          btnAcao.innerText = "▶ INICIAR"; btnAcao.className = "btn-start";
          btnStop.disabled = true; inputs.style.opacity = "1";
        }
        desenharGrafico();
      }).catch(e => console.log(e));
  }, 1000);

  function resizeCanvas() { canvas.width = canvas.parentElement.clientWidth; canvas.height = canvas.parentElement.clientHeight; }
  window.addEventListener('resize', resizeCanvas);
  resizeCanvas();

  function map(x, in_min, in_max, out_min, out_max) { return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; }

  function desenharGrafico() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.strokeStyle = '#eee'; ctx.lineWidth = 1;
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