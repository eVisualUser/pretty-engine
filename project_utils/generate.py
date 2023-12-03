import utils

import os

import tkinter as tk

root = tk.Tk()
root.title("Generate")
root.geometry("800x600")

frame = tk.Frame(root)

text_entry_path = tk.Entry(frame)
text_entry_path.pack()


def button_action_bat_to_bash():
    utils.convert_bat_to_bash(text_entry_path.get())


button_batToBash = tk.Button(frame, text="Convert Bat files to Bash", command=button_action_bat_to_bash)
button_batToBash.pack()

frame.pack()

root.mainloop()
