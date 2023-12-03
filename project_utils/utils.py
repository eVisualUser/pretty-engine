
import os
import subprocess


def convert_bat_to_bash(folder):
    files = os.listdir(folder)
    
    for file in files:
        if file.endswith(".bat"):
            new_file = file.replace('.bat', '.bash')
            with open(os.path.join(folder, file), 'r') as read_file:
                content = read_file.read()
            with open(os.path.join(folder, new_file), 'w') as write_file:
                write_file.write(content)


def execute_command(command):
    subprocess.run(command, shell=True)
