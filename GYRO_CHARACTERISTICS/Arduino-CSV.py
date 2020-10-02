# Implementación tomada del ejemplo de Liz Miller de Learn Robotics:
# https://www.learnrobotics.org/blog/arduino-data-logger-csv/


import serial  # import serial library to control serial comms

# Puertos USB del computador (Mac)
# port_1 = "/dev/cu.usbmodemFD121"    # el mas pegado al cargador
port_2 = '/dev/cu.usbmodemFA131'    # el mas pegado a los audifonos

baud = 115200  # velocidad de comunicacion serial (baudrate)
filename = "mov_eje_z - x(dps),y(dps),z(dps),t(us).csv"  # Nombre del archivo

ser = serial.Serial(port=port_2, baudrate=baud)
print("Conected to port:", port_2)
file = open(filename, 'a')    # crea un archivo adjuntando los valores a=apend

samples = 10050
print_labels = False
line = 0    # start at 0 because our header is 0 (not real data)
fail = False    # para revisar si hubo falla en el arudino

while line <= samples:
    if print_labels:
        if line == 0:
            print("Printing Column Headers")
        else:
            print("Line " + str(line) + ": writing...")
    getData = str(ser.readline())
    data = getData[2:][:-5]  # [2:][:-5] elimina el primer caracter y el ultimo
#    print(data)   # imprime la linea, se quita para que no imprima en terminal

    if "Failed" in data:
        fail = True
        break

    file = open(filename, "a")
    file.write(data + "\n")  # write data with a newline
    line = line+1

if fail:
    print("Failed")
else:
    print("Data collection complete!")
file.close()
