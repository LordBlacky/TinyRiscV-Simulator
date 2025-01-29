#
# TinyRiscV-Simulator 2024
# ===========================
#
# Project: https://github.com/LordBlacky/TinyRiscV-Simulator
#
#

import keyboard
import socket


client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_address = ('localhost', 50000)

while True:
    keys = 0
    if keyboard.is_pressed('a'):
        keys += 1
    if keyboard.is_pressed('s'):
        keys += 2
    if keyboard.is_pressed('d'):
        keys += 4
    if keyboard.is_pressed('f'):
        keys += 8

    if keyboard.is_pressed('esc'):
        break

    client_socket.sendto(str(keys).encode(), server_address)
    data, server = client_socket.recvfrom(1024)

client_socket.sendto("EXIT".encode(), server_address)
data, server = client_socket.recvfrom(1024)
client_socket.close()
