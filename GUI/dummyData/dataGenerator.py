import random
import csv
import math


with open('GUI/dummyData/FIFO.csv', 'w', newline='') as file:
    writer = csv.writer(file)

    # Escribimos la cabecera
    writer.writerow(['CantidadSolicitudes', 'tiempoEjecucion', 'memoriaUtilizada'])

    # Generamos 100 filas de datos aleatorios
    for i in range(100):
        cantidad_solicitudes = random.randint(1, 100)
        tiempo_ejecucion = cantidad_solicitudes * 10 + random.randint(-10, 10)
        memoria_utilizada = math.log(tiempo_ejecucion, 2) + random.randint(-2, 2)

        # Escribimos la fila de datos en el archivo CSV
        writer.writerow([cantidad_solicitudes, tiempo_ejecucion, memoria_utilizada])



with open('GUI/dummyData/Heavy.csv', 'w', newline='') as file:
    writer = csv.writer(file)

    # Escribimos la cabecera
    writer.writerow(['CantidadSolicitudes', 'tiempoEjecucion', 'memoriaUtilizada'])

    # Generamos 100 filas de datos aleatorios
    for i in range(100):
        cantidad_solicitudes = random.randint(1, 100)
        tiempo_ejecucion = cantidad_solicitudes * 5 + random.randint(-5, 5)
        memoria_utilizada = math.log(tiempo_ejecucion, 4) + random.randint(-1, 1)

        # Escribimos la fila de datos en el archivo CSV
        writer.writerow([cantidad_solicitudes, tiempo_ejecucion, memoria_utilizada])

with open('GUI/dummyData/hilos.csv', 'w', newline='') as file:
    writer = csv.writer(file)

    # Escribimos la cabecera
    writer.writerow(['CantidadSolicitudes', 'tiempoEjecucion', 'memoriaUtilizada'])

    # Generamos 100 filas de datos aleatorios
    for i in range(100):
        cantidad_solicitudes = random.randint(1, 100)
        tiempo_ejecucion = cantidad_solicitudes * 2 + random.randint(-2, 2)
        memoria_utilizada = math.log(tiempo_ejecucion, 2) + random.randint(-10, 10)

        # Escribimos la fila de datos en el archivo CSV
        writer.writerow([cantidad_solicitudes, tiempo_ejecucion, memoria_utilizada])


with open('GUI/dummyData/PreHeavy.csv', 'w', newline='') as file:
    writer = csv.writer(file)

    # Escribimos la cabecera
    writer.writerow(['CantidadSolicitudes', 'tiempoEjecucion', 'memoriaUtilizada'])

    # Generamos 100 filas de datos aleatorios
    for i in range(100):
        cantidad_solicitudes = random.randint(1, 100)
        tiempo_ejecucion = cantidad_solicitudes * 8 + random.randint(-8, 8)
        memoria_utilizada = math.log(tiempo_ejecucion, 5) + random.randint(-8, 8)

        # Escribimos la fila de datos en el archivo CSV
        writer.writerow([cantidad_solicitudes, tiempo_ejecucion, memoria_utilizada])
