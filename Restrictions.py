#! / u s r / b i n / env python
# −∗− c o d i n g : u t f −8 −∗−

from constraint import *
import sys


Aviones, Normales, Jumbo, Restrictos = [], [], [], []
Variables_Aviones = []
Dominio_Angar = []

if len(sys.argv) != 2:
        print("Error in number of argumets")

file = open(sys.argv[1], "r")

    #we read the input file
FranjaHoraria = int(file.readline()[9:])
rowColumns = file.readline()
rows = int(rowColumns[:1])
columns = int(rowColumns[2:])
STD = file.readline()[4:].split()
SPC = file.readline()[4:].split()
PRK = file.readline()[4:].split()
for x in file:
    Aviones.append(x.split("-"))

    #we fill the rest of the subgroups created
for av in Aviones:
        if av[1] == "STD":
            Normales.append(av)
        elif av[1] == "JMB":
            Jumbo.append(av)

        if av[2] == "T":
            Restrictos.append(av)


for i in range(1, len(Aviones) +1):
    for j in range(1, FranjaHoraria +1):
        avion = str(i) + "-" + str(j)
        Variables_Aviones.append(avion)

for r in range(0, rows):
    for j in range(0, columns):
        position = "(" + str(r) + "," + str(j) + ")"
        Dominio_Angar.append(position)

problem = Problem()
problem.addVariables(Variables_Aviones, Dominio_Angar)

