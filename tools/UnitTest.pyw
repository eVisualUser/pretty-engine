import subprocess
import tkinter as tk


def Execute():
    subprocess.run("cmake -S ../ -B ../build -G Ninja")
    subprocess.run("ninja test -C ../build")


root = tk.Tk()

root.minsize(380, 480)
root.title("Unit Tests")

button_execute = tk.Button(root, text="Test", command=Execute())
button_execute.pack()

root.mainloop()
