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

def tarefa_de_fundo():
    global app_aberto
    caminho_csv = None  
    ultimo_id_salvo = -1 

    while app_aberto:
        try:
            r = requests.get(f"{URL_SENSOR}/dados", timeout=3)
            
            if r.status_code == 200:
                dados = r.json()
                id_atual = dados.get('count', 0)
                rodando = dados.get("rodando", False)
                
                # CRIAÇÃO: Se o experimento iniciou e não temos arquivo aberto para esta rodada
                if rodando and (caminho_csv is None or id_atual < ultimo_id_salvo):
                    usuario = os.environ.get('USERNAME') or os.environ.get('USER')
                    caminho_desktop = f"C:\\Users\\{usuario}\\Desktop\\DADOS_MONITOR"
                    os.makedirs(caminho_desktop, exist_ok=True)
                    
                    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
                    caminho_csv = os.path.join(caminho_desktop, f"Experimento_{timestamp}.csv")
                    
                    with open(caminho_csv, mode='w', newline='', encoding='utf-8') as file:
                        writer = csv.writer(file, delimiter=DELIMITADOR_CSV)
                        writer.writerow(["Data", "Hora", "S1", "S2", "S3", "S4", "S5", "S6", "Temp", "Tempo_Exp", "Modo", "Luz"])
                    
                    ultimo_id_salvo = -1 
                    print(f"Novo experimento iniciado! Arquivo: {caminho_csv}")

                # GRAVAÇÃO: Salva os dados se estiver rodando e for uma leitura nova
                if rodando and id_atual != ultimo_id_salvo and caminho_csv is not None:
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
                            str(dados.get('temp','0')).replace('.',','),
                            dados.get('status','--'), 
                            "AUTOMATICO", "sim"
                        ])
                        ultimo_id_salvo = id_atual 

                # RESET: Se o experimento parou, limpa a referência para o próximo Iniciar
                if not rodando:
                    caminho_csv = None
                    ultimo_id_salvo = -1
                
                time.sleep(0.5)

        except (requests.exceptions.ConnectionError, requests.exceptions.Timeout):
            # Se a luz cair durante o experimento, registra a falha no arquivo atual
            if caminho_csv is not None:
                try:
                    with open(caminho_csv, mode='a', newline='', encoding='utf-8') as file:
                        writer = csv.writer(file, delimiter=DELIMITADOR_CSV)
                        agora = datetime.now()
                        writer.writerow([
                            agora.strftime("%d/%m/%Y"), agora.strftime("%H:%M:%S"),
                            "0,00", "0,00", "0,00", "0,00", "0,00", "0,00", "0,00",
                            "--", "OFFLINE", "NAO"
                        ])
                except: pass
            time.sleep(5)
            
        except Exception as e:
            time.sleep(2)

if __name__ == '__main__':
    t = threading.Thread(target=tarefa_de_fundo)
    t.daemon = True
    t.start()
    
    webview.create_window('Monitor Pressão Industrial - LSA', URL_SENSOR, width=1024, height=768)
    webview.start()