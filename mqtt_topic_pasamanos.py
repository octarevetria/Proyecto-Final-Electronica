import paho.mqtt.client as mqtt
import json
import time

#this code is used to forward the messages from the receiver broker to the sender broker
receiver_broker_address = "mqtt.autolink.com.uy"
receiver_topic = "#"
receiver_username = "pruebas_tesis_ORT"
receiver_password = "123456"

sender_broker_address = "demo.thingsboard.io"
sender_topic = "v1/devices/me/telemetry"
texto = "{""lat:51.001000,lon:52.001000,sat:33,trk:2,bat:66""}"
sender_username = "Vm8OAEIC2OTn2ESgekJi"
sender_password = "Vm8OAEIC2OTn2ESgekJi"

def on_connect_sender(client, userdata, flags, rc):
    print("Connected to sender with result code "+str(rc))


def on_connect_receiver(client, userdata, flags, rc):
    print("Connected to receiver with result code "+str(rc))
    client.subscribe(receiver_topic)

def on_message_receiver(client, userdata, msg):

    #resend the message to the sender broker and print if successful
    payload = str(msg.payload)
    payload = payload[2::]
    payload = payload[:-1]
    if payload.find('lat') is not -1:
        result = sender_client.publish(sender_topic, payload)
        print("Message sent to sender broker with result: "+str(result.rc))
        print("Mande esto a ThingsBoard: ", payload)
    
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

# loop start
receiver_client.loop_start()
sender_client.loop_start()

# loop forever

while True:
    time.sleep(1)