# VERSI PYTHON 3.9.2

import os

try:
    import cv2
    import time
    from ultralytics import YOLO
    import requests
    from datetime import date
    from datetime import datetime
    import os
    import numpy as np

except ImportError:
    print("Library tidak ditemukan, menginstal sekarang...")
    os.system(
        "pip install openpyxl==3.1.5 numpy==2.0.2  tqdm==4.67.0 pickle-mixin==1.0.2 opencv-python==4.10.0.84 ultralytics==8.3.32 requests==2.32.3 pyserial==3.5")

    import cv2
    import time
    from ultralytics import YOLO
    import requests

    from datetime import date
    from datetime import datetime
    import os
    import numpy as np


######################## DEKLARASI ########################

# Mendapatkan folder saat ini
current_directory = os.path.dirname(
    os.path.abspath(__file__))

print("Tanggal :", str(date.today()))
print("Folder saat ini:", current_directory)


url = "http://192.168.126.37"  # IP ESP32-CAM


warna_bgr = [
    ("merah", (0, 0, 255)),
    ("kuning", (0, 255, 255)),
    ("hijau", (0, 255, 0)),
    ("biru", (255, 0, 0)),
    ("ungu", (128, 0, 128)),      # Sama di RGB dan BGR karena G=0
    ("putih", (255, 255, 255)),
    ("orange", (0, 165, 255)),
    ("cyan", (255, 255, 0)),
    ("magenta", (255, 0, 255))    # Sama di RGB dan BGR karena G=0
]

timer = time.time()



# Load model
model = YOLO(os.path.join(current_directory, "D:\ABHINAYA FILE\KULIAH 8 SKRIPSI\Project Skripsi Hardware/last.pt"))
timer_deteksi = 2 # durasi saat objek terdeteksi 5 detik delay sebelum mengambil tindakan

print("DETEKSI LABEL : ", model.names)

kamus = [{"shoving_sungkur" : "Weathering And Raveling"},
         {"lubang":"Pothole"},
         {"retak_tepi":"Edge Cracking"},
         {"tambalan":"Patching And Utility Cut Patching"},
         {"retak_buaya":"Alligator Cracking"}]



#tinggi esp-cam dari tanah 75 cm 
kalibrasi_x_asli = 26 #cm ukuran asli lobang
kalibrasi_y_asli = 16 #cm ukuran asli lobang
kalibrasi_x = 188 #ukuran pixel
kalibrasi_y = 144 #ukuran pixel


#timer
status = [0,0,0,0,0,0,0,0,0,0,0,0]

# list dictionary label per frame dan koordinatnya
object_per_frame = []

label_per_frame = ""
old_label_per_frame = ""


def proses_get_gambar():
    response = requests.get(url, timeout=5)
    img_array = np.frombuffer(response.content, np.uint8)
    frame = cv2.imdecode(img_array, cv2.IMREAD_COLOR)
    return frame



def iou(boxA, boxB):
    # Menghitung Intersection over Union (IoU) 
    # untuk penghitung timpang tindih
    xA = max(boxA[0], boxB[0])
    yA = max(boxA[1], boxB[1])
    xB = min(boxA[2], boxB[2])
    yB = min(boxA[3], boxB[3])
    
    interArea = max(0, xB - xA) * max(0, yB - yA)
    boxAArea = (boxA[2] - boxA[0]) * (boxA[3] - boxA[1])
    boxBArea = (boxB[2] - boxB[0]) * (boxB[3] - boxB[1])
    
    iou_value = interArea / float(boxAArea + boxBArea - interArea + 1e-6)
    return iou_value

def non_max_suppression(objs, iou_threshold=0.5):
    # untuk penghitung timpang tindih
    if not objs:
        return []
    
    objs = sorted(objs, key=lambda x: x["confidence"], reverse=True)
    hasil = []
    
    while objs:
        objek_terpilih = objs.pop(0)
        hasil.append(objek_terpilih)
        objs = [
            o for o in objs
            if iou(objek_terpilih["box"], o["box"]) < iou_threshold
        ]
    return hasil

def proses_deteksi( frame):
    global object_per_frame,label_per_frame

    anti_timpang_tindih = 1 # untuk mengabaikan objek yang terdeteksi tumpang tindih

   

    results = model(frame)
    
    for r in results:
        object_per_frame = []
        label_per_frame = ""
        boxes = (r.boxes.xyxy).tolist()
        for box, conf, cls in zip(boxes, r.boxes.conf, r.boxes.cls):
            x1, y1, x2, y2 = map(int, box)
            label = model.names[int(cls)]
            koordinat = (x1 + (x2 - x1) // 2, y1 + (y2 - y1) // 2)

            object_per_frame.append({
                "label": label,
                "koordinat": koordinat,
                "box": [x1, y1, x2, y2],
                "confidence": float(conf),
                "cls": int(cls)
            })

        if anti_timpang_tindih:
            object_per_frame = non_max_suppression(object_per_frame, iou_threshold=0.5)

        # Gambar ke frame
        for obj in object_per_frame:
            x1, y1, x2, y2 = obj["box"]
            label = obj["label"]
            cls = obj["cls"]
            conf = obj["confidence"]
            label_per_frame += " " + label
            label = terjemahkan(label)

            cv2.putText(frame, f"{label} {str(round(conf*100,2))} %", (x1, y1 - 50),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.9, (255, 255, 55), 2)
            cv2.rectangle(frame, (x1, y1), (x2, y2), warna_bgr[cls][1], 2)

    return frame


def terjemahkan(kata):
    for item in kamus:
        if kata in item:
            return item[kata]
    return kata  # jika tidak ditemukan, kembalikan kata aslinya


def proses_kirim(frame,panjang,lebar,label):
    print("MENGIRIM DATA : ", label)
    nama_file = os.path.join(current_directory,"logo.jpg")
    cv2.imwrite(nama_file, frame)
    print("gambar disiman ... ")

    #cek kamus 

    label = terjemahkan(label)

 

    url = "http://localhost/abhinaya/api/api.php"
    payload = {'data': '{"panjang":'+'"'+str(panjang)+'"'+',"lebar":'+'"'+str(lebar)+'"'+',"jenis_kerusakkan":'+'"'+str(label)+'"'+"}"}
    print(url)
    print(payload)

    files=[
    ('foto',('logo.jpg',open(nama_file,'rb'),'image/jpeg'))
    ]
    headers = {}

    response = requests.request("POST", url, headers=headers, data=payload, files=files)

    print(response.text)



def proses_logika():
    global timer, kalibrasi_x_asli,kalibrasi_y_asli,kalibrasi_x,kalibrasi_y,label_per_frame,old_label_per_frame

    while True:
        if time.time() - timer > timer_deteksi :    
            try :
                print("Menghubungi esp32 cam ... ")
                frame = proses_get_gambar()
                frame = proses_deteksi(frame)
                a = 0
                


                if label_per_frame != old_label_per_frame :
                    for a in object_per_frame :
                        print(a)
                        _x =  np.abs(a["box"][0] - a["box"][2])
                        _y = np.abs(a["box"][1] - a["box"][3])
                        hasil_x = kalibrasi_x_asli/kalibrasi_x * _x
                        hasil_y = kalibrasi_y_asli/kalibrasi_y * _y
                           # cm ke m
    
                        hasil_y = round(hasil_y/100,2)
                        hasil_x = round(hasil_x/100,2)
                        
                        print("Hasil P, L : ", hasil_y,hasil_x)
                        cv2.putText(frame, f"L : {hasil_x}m P : {hasil_y}m", (a["box"][0], a["box"][1]- 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.9, (255, 255, 55), 2)
                        proses_kirim(frame,hasil_y,hasil_x,a["label"])
                        old_label_per_frame = label_per_frame

                        
                        a =1
                        
                        
                else :
                    print("DATA SAMA SEPERTI SEBELUMNYA, SKIPP .... ")

                cv2.imshow("Kerusakan Jalan ", frame)
                
                if(a == 1):
                    time.sleep(5)  
                    

            except :
                pass
            timer = time.time()

    
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

   
    cv2.destroyAllWindows()


    
if __name__ == "__main__":
    timer = time.time()
    proses_logika()
            



