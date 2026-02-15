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

    # Cabeçalho atualizado com coluna "Luz"
    try:
        if not os.path.exists(caminho_csv):
            with open(caminho_csv, mode='w', newline='', encoding='utf-8') as file:
                writer = csv.writer(file, delimiter=DELIMITADOR_CSV)
                writer.writerow(["Data", "Hora", "S1", "S2", "S3", "S4", "S5", "S6", "Tempo_Exp", "Modo", "Luz"])
    except Exception as e:
        print(f"Erro ao criar arquivo: {e}")
        return

    while app_aberto:
        try:
            r = requests.get(f"{URL_SENSOR}/dados", timeout=3)
            
            if r.status_code == 200:
                dados = r.json()
                id_atual = dados.get('count', 0)
                
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
                            dados.get('status','--'), "AUTOMATICO", "sim"
                        ])
                        ultimo_id_salvo = id_atual 
                
                time.sleep(0.5)

        except (requests.exceptions.ConnectionError, requests.exceptions.Timeout):
            # DETECÇÃO DE QUEDA DE ENERGIA: Registra "não" no CSV do PC
            print("Aguardando volta do sensor (ESP32)... Registrando falta de luz.")
            try:
                with open(caminho_csv, mode='a', newline='', encoding='utf-8') as file:
                    writer = csv.writer(file, delimiter=DELIMITADOR_CSV)
                    agora = datetime.now()
                    writer.writerow([
                        agora.strftime("%d/%m/%Y"), agora.strftime("%H:%M:%S"),
                        "0,00", "0,00", "0,00", "0,00", "0,00", "0,00",
                        "--", "OFFLINE", "NAO"
                    ])
            except:
                pass
            time.sleep(5)
            
        except Exception as e:
            print(f"Erro inesperado: {e}")
            time.sleep(2)

if __name__ == '__main__':
    t = threading.Thread(target=tarefa_de_fundo)
    t.daemon = True
    t.start()
    
    webview.create_window('Monitor Pressão Industrial - ESTÁVEL', URL_SENSOR, width=1024, height=768)
    webview.start()