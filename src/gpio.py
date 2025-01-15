import socket
import threading
import time
import pygame

client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_address = ('localhost', 50000)

gpio = 0
pressed_keys = set()

def send_udp():
    global gpio
    while True:
        client_socket.sendto(str(gpio).encode(), server_address)
        time.sleep(0.1)

def handle_keys():
    global gpio
    keys = pygame.key.get_pressed()

    if keys[pygame.K_w]:
        pressed_keys.add('w')
    else:
        pressed_keys.discard('w')

    if keys[pygame.K_a]:
        pressed_keys.add('a')
    else:
        pressed_keys.discard('a')

    if keys[pygame.K_s]:
        pressed_keys.add('s')
    else:
        pressed_keys.discard('s')

    if keys[pygame.K_d]:
        pressed_keys.add('d')
    else:
        pressed_keys.discard('d')

    gpio = 0
    if 'w' in pressed_keys:
        gpio += 1
    if 'a' in pressed_keys:
        gpio += 2
    if 's' in pressed_keys:
        gpio += 4
    if 'd' in pressed_keys:
        gpio += 8

def stop_listener():
    client_socket.sendto("EXIT".encode(), server_address)
    data, server = client_socket.recvfrom(1024)
    client_socket.close()

if __name__ == '__main__':

    pygame.init()
    screen = pygame.display.set_mode((100, 100))

    udp_thread = threading.Thread(target=send_udp)
    udp_thread.daemon = True
    udp_thread.start()

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        handle_keys()

        if pygame.key.get_pressed()[pygame.K_ESCAPE]:
            running = False

        time.sleep(0.01)

    stop_listener()
    pygame.quit()
