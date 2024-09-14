import paho.mqtt.client as mqtt
import json
import time

receiver_broker_address = "mqtt.autolink.com.uy"
receiver_topic = "#"
receiver_username = "pruebas_tesis_ORT"
receiver_password = "123456"

sender_broker_address = "demo.thingsboard.io"
sender_topic = "v1/devices/me/telemetry"
sender_username = "Vm8OAEIC2OTn2ESgekJi"
sender_password = "Vm8OAEIC2OTn2ESgekJi"

def on_connect_sender(client, userdata, flags, rc):
    print("Codigo resultante de conectarme al sender: "+str(rc))

def on_connect_receiver(client, userdata, flags, rc):
    print("Codgio resultande de conectarme al receptor: "+str(rc))
    client.subscribe(receiver_topic)

def on_message_receiver(client, userdata, msg):

    payload = str(msg.payload)
    payload = payload[2::]
    payload = payload[:-1]
    if payload.find('trk') is not -1:
        resultado = sender_client.publish(sender_topic, payload)
        print("Resultado de enviar al broker: "+str(resultado.rc))
        print("Quiero enviar esto a ThingsBoard: ", payload)
    
#sender broker client
sender_client = mqtt.Client(client_id = "sender")
sender_client.username_pw_set(sender_username, sender_password)
sender_client.connect(sender_broker_address, 1883, 60)
sender_client.on_connect = on_connect_sender

#receiver broker client
receiver_client = mqtt.Client()
receiver_client.username_pw_set(receiver_username, receiver_password)
receiver_client.on_connect = on_connect_receiver
receiver_client.on_message = on_message_receiver
receiver_client.connect(receiver_broker_address, 1883, 60)

# comienzo del loop
receiver_client.loop_start()
sender_client.loop_start()

# loop sin fin
while True:
    time.sleep(1)