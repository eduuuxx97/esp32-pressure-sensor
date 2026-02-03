import webview
import threading
import time
import requests
import csv
import os
from datetime import datetime

URL_SENSOR = "http://192.168.4.1"
DELIMITADOR_CSV = ";"
app_aberto = True

def obter_caminho_real():
    usuario = os.environ.get('USERNAME') or os.environ.get('USER')
    caminho_desktop = f"C:\\Users\\{usuario}\\Desktop\\DADOS_MONITOR"
    os.makedirs(caminho_desktop, exist_ok=True)
    return os.path.join(caminho_desktop, f"Historico_PC_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv")

def tarefa_de_fundo():
    global app_aberto
    caminho_csv = obter_caminho_real()
    ultimo_id_salvo = -1 

    try:
        with open(caminho_csv, mode='w', newline='', encoding='utf-8') as file:
            writer = csv.writer(file, delimiter=DELIMITADOR_CSV)
            writer.writerow(["Data", "Hora", "S1", "S2", "S3", "S4", "S5", "S6", "Tempo_Exp", "Modo"])
    except: return

    while app_aberto:
        try:
            r = requests.get(f"{URL_SENSOR}/dados", timeout=2)
            if r.status_code == 200:
                dados = r.json()
                id_atual = dados.get('count', 0)
                
                # SÓ GRAVA SE O CONTADOR DO ESP32 MUDOU
                if dados.get("rodando", False) and id_atual != ultimo_id_salvo:
                    with open(caminho_csv, mode='a', newline='', encoding='utf-8') as file:
                        writer = csv.writer(file, delimiter=DELIMITADOR_CSV)
                        agora = datetime.now()
                        writer.writerow([
                            agora.strftime("%d/%m/%Y"), agora.strftime("%H:%M:%S"),
                            str(dados.get('s1','0')).replace('.',','), 
                            str(dados.get('s2','0')).replace('.',','), 
                            str(dados.get('s3','0')).replace('.',','), 
                            str(dados.get('s4','0')).replace('.',','), 
                            str(dados.get('s5','0')).replace('.',','), 
                            str(dados.get('s6','0')).replace('.',','),
                            dados.get('status','--'), "AUTOMATICO"
                        ])
                        ultimo_id_salvo = id_atual 
        except: pass
        time.sleep(0.2)

if __name__ == '__main__':
    t = threading.Thread(target=tarefa_de_fundo); t.daemon = True; t.start()
    webview.create_window('Monitor Pressão Industrial', URL_SENSOR, width=1024, height=768)
    webview.start()