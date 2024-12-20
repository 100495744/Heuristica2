#! / u s r / b i n / env python
# −∗− c o d i n g : u t f −8 −∗−

from constraint import *
import sys

#Varianbles y grupos que usaremos
Aviones, Normales, Jumbo, Restrictos = [], [], [], []
Variables_Aviones = []
Dominio_Angar = []
Num_Aviones = 0

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
    #Llenamos la lista con toda la información de cada avión
    Aviones.append(x.split("-"))
    Aviones[-1][4] = Aviones[-1][4][0:1]
    Num_Aviones += 1

#Llenamos los subgrupos de los aviones
for i in range(1, len(Aviones) +1):
    for j in range(1, FranjaHoraria +1):
        #cada variable tendrá la estructura de #avion-#franja
        avion = str(i) + "-" + str(j)
        Variables_Aviones.append(avion)

        if Aviones[i-1][1] == "STD":
            Normales.append(avion)
        elif Aviones[i-1][1] == "JMB":
            Jumbo.append(avion)

        if Aviones[i-1][2] == "T":
            Restrictos.append(avion)

#creamos el grupo con el Dominio de las posiciones.
for r in range(0, rows):
    for j in range(0, columns):
        position = "(" + str(r) + "," + str(j) + ")"
        Dominio_Angar.append(position)

#inicializamos el problema y añadimos las variables con su dominio
problem = Problem()
problem.addVariables(Variables_Aviones, Dominio_Angar)





#Usamos funciones dentro de funciones para coger información del nombre de la variable
def MaximoAvionesJumbo(*args):
    def Calculo(*args2):
        x = 0
        y= 0
        #iteramos por cada posición posible
        for time in Dominio_Angar:
            for avionsito in range(len(args2)):
                #si la posición de la variable coincide con la posición que estamos viendo
                if args2[avionsito] == time:
                    #Vemos si el avión es de tipo JMB
                    if Aviones[int(args[0][avionsito][0]) -1][1] == "JMB":
                        x += 1
                        y += 1
                    else:
                        x += 1
            #Vemos si hay más de dos aviones y si hay más de 1 JMB junto
            if x > 2 or y > 1:
                return False
            x = 0
            y = 0
        return True
    return Calculo

def CompletarTareas(*args):

    def VerdaderoCompletarTareas(*args2):
        tipo1 = 0
        tipo2 = 0
        posibletipo1 = 0        # tipo1 || tipo1 + posibleTipo1 || posibleTipo2 -
        posibletipo2 = 0
        mintipo1 = int(Aviones[int(args[0][0][0]) - 1][3])
        mintipo2 = int(Aviones[int(args[0][0][0]) - 1][4])
        #contamos las veces que pasa por un hangar especial o normal
        for i in range( len(args2)):

            if STD.count( args2[i] ) > 0:
                tipo1 += 1
            elif SPC.count(args2[i]) > 0:
                posibletipo2 += 1
                posibletipo1 += 1
        #revisamos los dos casos posibles si es consigue rellenar el cupo
        if tipo1 < mintipo1:
            if tipo1 + posibletipo1 < mintipo1:
                return False
            elif posibletipo2 - (mintipo1 - tipo1) < mintipo2:
            #Cogemos de tipo2 la diferencia necesaria que hubiera hecho falta para completar la parte 1
                return False
        elif posibletipo2 < mintipo2:
            return False
        tipo1 = 0
        posibletipo2 =0
        posibletipo1 =0
        return True
    return VerdaderoCompletarTareas

def RestriccionOrden(*args):
    def VerdaderoRestriccionOrden(*args2):
        if len(args[0]) > 0:
            #cogemos el valor minimo de tareas de tipo2 para este avión
            mintipo2 = int(Aviones[int(args[0][0][0]) - 1][4])
            tipo2= 0
            #vamos por orden contando las tareas de tipo2 que hace
            for i in range(len(args2)):
                if SPC.count(args2[i]):
                    tipo2 += 1
                #si para cuando llegue a un hangar normal no ha completado todas sus tareas de tipo2 es falso
                if STD.count(args2[i]):
                    if mintipo2 > tipo2:
                        return False
            return True
        return True
    return VerdaderoRestriccionOrden

def RestriccionEspacio (*args):
    def VerdaderaRestriccion(*args2):
        for i in range(0, len(args2)):
            #calculamos el valor de las coordenadas adyacentes
            up = "(" + args2[i][1] + "," + str((int(args2[i][3]) + 1)) + ")"
            down ="("+ args2[i][1] + "," + str((int(args2[i][3]) - 1))+ ")"
            left ="("+ str((int(args2[i][1])) - 1) + "," + args2[i][3]+ ")"
            right ="("+ str((int(args2[i][1])) + 1) + "," + args2[i][3]+ ")"
            round = 0
            #vemos si estamos en una esquina o lateral
            if args2[i][1] == "0" or args2[i][1] == str(rows - 1):
                if args2[i][3] == "0" or args2[i][3] == str(columns - 1):
                    round = 2
                else:
                    round = 3
            else:
                round = 4

            #vemos si es un jumbo tiene que estar libre en todos los adyacentes
            if Aviones[int(args[0][i][0]) -1][1] == "JMB":
                round = 1
            #Si entre todas la variabels coincide alguna de las coordenadas calculadas se quita un espacio
            if args2.count(up) > 0:
                round -= 1
            if args2.count(down) > 0:
                round -= 1
            if args2.count(left) > 0:
                round -= 1
            if args2.count(right) > 0:
                round -= 1

            if round <= 0:
                return False
        return True
    return VerdaderaRestriccion


#para estas restricciones se hacen grupos de distintos aviones en la misma franja
for j in range(1, FranjaHoraria +1):
    listavariables = []
    for i in Variables_Aviones:
        if i[2] == str(j):
            listavariables.append(i)
    problem.addConstraint(MaximoAvionesJumbo(listavariables), listavariables)
    problem.addConstraint(RestriccionEspacio(listavariables), listavariables)


#para estas restricciones hacemos grupos del mismo avión en distintas franjas
for j in range(1, len(Aviones) + 1):
    listavariables = []
    for i in Variables_Aviones:
        if i[0] == str(j):
            listavariables.append(i)
    problem.addConstraint(CompletarTareas(listavariables), listavariables)
    problem.addConstraint(RestriccionOrden(listavariables), listavariables)




solutions = problem.getSolutions()

#codigo para escribir el output
outputfile = sys.argv[1] + ".csv"
output = open(outputfile, "a")
output.write("N. Sol: " + str(len(solutions)) + "\n")
for i in range(0,6):
    output.write("Solucion " + str(i+1) +":" + "\n")
    for j in range(1,Num_Aviones +1 ):
        linea_avion = "\t" + "-".join(Aviones[j -1]) + ": "
        output.write(linea_avion)
        linea_sol = ""
        for avion in solutions[i].keys():
            if avion[0] == str(j):
                solucion = solutions[i][avion]
                if PRK.count(solucion) > 0:
                    linea_sol += "PRK:" + "".join(solucion) + ", "
                elif SPC.count(solucion) > 0:
                    linea_sol += "SPC" + "".join(solucion) + ", "
                else:
                    linea_sol += "STD" + "".join(solucion) + ", "
        output.write(str(linea_sol) + "\n")

