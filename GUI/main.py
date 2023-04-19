import tkinter as tk
from tkinter import scrolledtext, messagebox

main = tk.Tk(
    className="Visualizador. Evaluador de servidores con diferentes tipos procesos"
)

def close_app():
    main.destroy()


# Diagram logic --------------------------------------------------------------




# Main widget settings ---------------------------------------------------------

main.geometry("800x900")
main.resizable(False, False)

# Framing

main_frame = tk.Frame(main, bg="white", height=900, width=800)
main_frame.place(x=0, y=0)

header_frame = tk.Frame(main_frame, bg="red", height=100, width=800)
header_frame.place(x=0, y=0)


execution_time_frame = tk.Frame(main_frame, bg="salmon", height=400, width=400)
execution_time_frame.place(x=0, y=100)

average_time_frame = tk.Frame(main_frame, bg="darkturquoise", height=400, width=400)
average_time_frame.place(x=400, y=100)

memory_frame = tk.Frame(main_frame, bg="lime", height=400, width=400)
memory_frame.place(x=0, y=500)

custom_frame = tk.Frame(main_frame, bg="orangered", height=400, width=400)
custom_frame.place(x=400, y=500)


main.protocol("WM_DELETE_WINDOW", close_app)


main.mainloop()
