import subprocess
import tkinter as tk


root = tk.Tk()

index = 0


def Execute():
    global index
    index = index + 1
    subprocess.run("cmake -S ../ -B ../build -G Ninja")
    subprocess.run("ninja -C ../build")
    testRunResult = subprocess.run("ninja test -C ../build")
    if (testRunResult.returncode != 0):
        label = tk.Label(root, text=str(index) + ": Failed tests")
        label.pack()
    else:
        label = tk.Label(root, text=str(index) + ": Succeeded tests")
        label.pack()


root.minsize(380, 480)
root.title("Unit Tests")

button_execute = tk.Button(root, text="Test", command=Execute)
button_execute.pack()

root.mainloop()
