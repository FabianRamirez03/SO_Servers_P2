import tkinter as tk
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg


#import os
#os.environ['DISPLAY'] = ':0.0'

# Globales

font = "Arial"

main = tk.Tk(
    className="Visualizador. Evaluador de servidores con diferentes tipos procesos"
)

def close_app():
    if tk.messagebox.askokcancel("Salir", "¿Desea salir de la aplicación?"):
        main.destroy()
    


# Diagram logic --------------------------------------------------------------

def update_graphs():
    update_avg_execution_time_plot()
    update_execution_time_plot()
    main.after(5000, update_graphs)

def update_execution_time_plot():
    global execution_time_frame
    # Leer los archivos csv
    df_fifo = pd.read_csv('GUI/dummyData/FIFO.csv')
    df_heavy = pd.read_csv('GUI/dummyData/Heavy.csv')
    df_hilos = pd.read_csv('GUI/dummyData/hilos.csv')
    df_preheavy = pd.read_csv('GUI/dummyData/PreHeavy.csv')

    # Crear la figura y los subplots
    fig, ax = plt.subplots()

    # Agregar las series al gráfico
    ax.scatter(df_fifo['CantidadSolicitudes'], df_fifo['tiempoEjecucion'], label='FIFO')
    ax.scatter(df_heavy['CantidadSolicitudes'], df_heavy['tiempoEjecucion'], label='Heavy')
    ax.scatter(df_hilos['CantidadSolicitudes'], df_hilos['tiempoEjecucion'], label='hilos')
    ax.scatter(df_preheavy['CantidadSolicitudes'], df_preheavy['tiempoEjecucion'], label='PreHeavy')

    # Configurar el gráfico
    ax.set_xlabel('Cantidad de Solicitudes')
    ax.set_ylabel('Tiempo de Ejecución')
    ax.set_title('Tiempo de Ejecución - Cantidad de Solicitudes')
    ax.legend()

    # Crear la figura en el canvas de tkinter
    if hasattr(execution_time_frame, 'canvas'):
        execution_time_frame.canvas.get_tk_widget().pack_forget()
        execution_time_frame.canvas = None
    fig_canvas = FigureCanvasTkAgg(fig, master=execution_time_frame)
    fig_canvas.draw()
    fig_canvas.get_tk_widget().place(x=0, y=0, width=390, height=390)
    execution_time_frame.canvas = fig_canvas
    return

def update_avg_execution_time_plot():
	global average_time_frame
	df_fifo = pd.read_csv('GUI/dummyData/FIFO.csv')
	df_heavy = pd.read_csv('GUI/dummyData/Heavy.csv')
	df_hilos = pd.read_csv('GUI/dummyData/hilos.csv')
	df_preheavy = pd.read_csv('GUI/dummyData/PreHeavy.csv')

	# Calcular los tiempos promedio de cada cantidad de solicitudes
	df_fifo_avg = df_fifo.groupby('CantidadSolicitudes')['tiempoEjecucion'].mean().reset_index()
	df_heavy_avg = df_heavy.groupby('CantidadSolicitudes')['tiempoEjecucion'].mean().reset_index()
	df_hilos_avg = df_hilos.groupby('CantidadSolicitudes')['tiempoEjecucion'].mean().reset_index()
	df_preheavy_avg = df_preheavy.groupby('CantidadSolicitudes')['tiempoEjecucion'].mean().reset_index()

	# Crear la figura y los subplots
	fig, ax = plt.subplots()

	# Agregar las series al gráfico
	ax.plot(df_fifo_avg['CantidadSolicitudes'], df_fifo_avg['tiempoEjecucion'], label='FIFO')
	ax.plot(df_heavy_avg['CantidadSolicitudes'], df_heavy_avg['tiempoEjecucion'], label='Heavy')
	ax.plot(df_hilos_avg['CantidadSolicitudes'], df_hilos_avg['tiempoEjecucion'], label='hilos')
	ax.plot(df_preheavy_avg['CantidadSolicitudes'], df_preheavy_avg['tiempoEjecucion'], label='PreHeavy')

	# Configurar el gráfico
	ax.set_xlabel('Cantidad de Solicitudes')
	ax.set_ylabel('Tiempo Promedio de Ejecución')
	ax.set_title('Tiempo Promedio de Ejecución - Cantidad de Solicitudes')
	ax.legend()

	# Crear la figura en el canvas de tkinter
	if hasattr(average_time_frame, 'canvas'):
		average_time_frame.canvas.get_tk_widget().pack_forget()
		average_time_frame.canvas = None
	fig_canvas = FigureCanvasTkAgg(fig, master=average_time_frame)
	fig_canvas.draw()
	fig_canvas.get_tk_widget().place(x=0, y=0, width=390, height=390)
	average_time_frame.canvas = fig_canvas
        
def update_memory_plot():
	global memory_frame
	df_fifo = pd.read_csv('GUI/dummyData/FIFO.csv')
	df_heavy = pd.read_csv('GUI/dummyData/Heavy.csv')
	df_hilos = pd.read_csv('GUI/dummyData/hilos.csv')
	df_preheavy = pd.read_csv('GUI/dummyData/PreHeavy.csv')

	# Crear la figura y los subplots
	fig, ax = plt.subplots()

	# Agregar las series al gráfico
	ax.scatter(df_fifo['CantidadSolicitudes'], df_fifo['memoriaUtilizada'], label='FIFO')
	ax.scatter(df_heavy['CantidadSolicitudes'], df_heavy['memoriaUtilizada'], label='Heavy')
	ax.scatter(df_hilos['CantidadSolicitudes'], df_hilos['memoriaUtilizada'], label='hilos')
	ax.scatter(df_preheavy['CantidadSolicitudes'], df_preheavy['memoriaUtilizada'], label='PreHeavy')

	# Configurar el gráfico
	ax.set_xlabel('Cantidad de Solicitudes')
	ax.set_ylabel('Memoria')
	ax.set_title('Tiempo de Ejecución - memoria')
	ax.legend()

	# Crear la figura en el canvas de tkinter
	if hasattr(memory_frame, 'canvas'):
		memory_frame.canvas.get_tk_widget().pack_forget()
		memory_frame.canvas = None
	fig_canvas = FigureCanvasTkAgg(fig, master=memory_frame)
	fig_canvas.draw()
	fig_canvas.get_tk_widget().place(x=0, y=0, width=390, height=390)
	memory_frame.canvas = fig_canvas


# Main widget settings ---------------------------------------------------------

main.geometry("800x900")
main.resizable(False, False)

# Framing

main_frame = tk.Frame(main, bg="#E3E3E3", height=900, width=800)
main_frame.place(x=0, y=0)

header_frame = tk.Frame(main_frame, bg="white", height=92, width=792)
header_frame.place(x=4, y=4)


execution_time_frame = tk.Frame(main_frame, bg="white", height=396, width=394)
execution_time_frame.place(x=4, y=100)

average_time_frame = tk.Frame(main_frame, bg="white", height=396, width=394)
average_time_frame.place(x=402, y=100)

memory_frame = tk.Frame(main_frame, bg="white", height=396, width=394)
memory_frame.place(x=4, y=500)

custom_frame = tk.Frame(main_frame, bg="white", height=396, width=394)
custom_frame.place(x=402, y=500)


# Labels

# title 
title_label = tk.Label(header_frame, text='Evaluador de servidores con diferentes tipos procesos', font=(font, 21), bg="white")
title_label.place(x=15, y=30)

# Graphics

update_execution_time_plot()
update_avg_execution_time_plot()
update_memory_plot()


main.protocol("WM_DELETE_WINDOW", close_app)


main.mainloop()
