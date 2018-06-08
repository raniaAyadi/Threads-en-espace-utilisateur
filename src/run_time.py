import numpy as np
import os
import matplotlib.pyplot as plt

def runtime(name, size1, size2, step, step2):
    l = np.arange(0, size1, step)
    l2 = np.arange(0, size2, step2)
    thread = []
    pthread = []

    for j in l2:
        os.system('rm time.txt')

        for i in l:
            a = "(time ./tst/" + name + " " + str(i) + " " + str(j) + ") 2> tmp.txt 1>/dev/null && awk \'/real/ {print substr($2,3,5)}\' tmp.txt >> time.txt"
            os.system(a)

        with open('time.txt') as f:
            thread = f.readlines()
        thread = [x.strip() for x in thread]


        os.system('rm time.txt')

        for i in l:
            a = "(time ./pthread/"+name+" "+str(i)+" "+str(j)+") 2> tmp.txt 1>/dev/null && awk \'/real/ {print substr($2,3,5)}\' tmp.txt >> time.txt"
            os.system(a)

        with open('time.txt') as f:
            pthread = f.readlines()
        pthread = [x.strip() for x in pthread]


        plt.figure(name+"_runtime_"+str(j))
        plt.title(name+"_runtime_"+str(j))
        plt.plot(l,thread,'b',label="thread.h")
        plt.plot(l,pthread,'r',label="pthread")
        plt.xlabel('Number of threads')
        plt.ylabel('Execution Time')
        plt.legend()
        #plt.savefig("./graphs/"+name+".png",format= 'png')
        plt.show()


print("\nTest1 (thread.h): ")
os.system('time ./tst/01-main 1>/dev/null')
print("\nTest1 (pthread): ")
os.system('time ./pthread/01-main 1>/dev/null')

print("\nTest2 (thread.h): ")
os.system('time ./tst/02-switch 1>/dev/null')
print("\nTest2 (pthread): ")
os.system('time ./pthread/02-switch 1>/dev/null')

print("\nTest11 (thread.h): ")
os.system('time ./tst/11-join 1>/dev/null')
print("\nTest11 (pthread): ")
os.system('time ./pthread/11-join 1>/dev/null')

print("\nTest12 (thread.h): ")
os.system('time ./tst/12-join-main 1>/dev/null')
print("\nTest12 (pthread): ")
os.system('time ./pthread/12-join-main 1>/dev/null')

print("\nLancement du test 21 pour 100000 threads max\n")
runtime("21-create-many",10**4,1,100,100)

print("\nLancement du test 22 pour 10**4 threads max\n")
runtime("22-create-many-recursive",3*10**3,1,100,100) #pthread plante pour ./Test22 4000

print("\nLancement du test 23 pour 10000 threads max\n")
runtime("23-create-many-once",10**4,1,100,100)

print("\nLancement du test 31 pour 10000 threads max avec 0/20/40/60 yield \n")
runtime("31-switch-many",10**4,80,100,20)

print("\nLancement du test 32 pour 1000 threads max\n")
runtime("32-switch-many-join",10**3,10**4,10,1000)


print("\nLancement du test 51 pour X=20 max\n")
l = np.arange(0,20,1)
l2 = np.arange(0,16,1)
thread = []
pthread = []
os.system('rm time.txt')

for i in l:
    a = "(time ./tst/51-fibonacci "+str(i)+") 2> tmp.txt 1>/dev/null && awk \'/real/ {print substr($2,3,5)}\' tmp.txt >> time.txt"
    os.system(a)

with open('time.txt') as f:
    thread = f.readlines()
thread = [x.strip() for x in thread]


os.system('rm time.txt')

for i in l2:
    a = "(time ./pthread/51-fibonacci "+str(i)+") 2> tmp.txt 1>/dev/null && awk \'/real/ {print substr($2,3,5)}\' tmp.txt >> time.txt"
    os.system(a)

with open('time.txt') as f:
    pthread = f.readlines()
pthread = [x.strip() for x in pthread]


plt.figure("51-fibonacci_runtime")
plt.title("51-fibonacci_runtime")
plt.plot(l,thread,'b',label="thread.h")
plt.plot(l2,pthread,'r',label="pthread")
plt.xlabel('Integer X')
plt.ylabel('Execution Time')
plt.legend()

plt.show()


print("\n Lancement du test 70 avec 10**3 taille max du tableau\n")
l = np.arange(0,10**3,100)
l2 = np.arange(0,1,100)
thread = []
pthread = []
for j in l2:
    os.system('rm time.txt')

    for i in l:
        a = "(time ./tst/70-sum-array " +str(i)+" "+str(j)+") 2> tmp.txt 1>/dev/null && awk \'/real/ {print substr($2,3,5)}\' tmp.txt >> time.txt"
        os.system(a)

    with open('time.txt') as f:
        thread = f.readlines()
    thread = [x.strip() for x in thread]


    os.system('rm time.txt')

    for i in l:
        a = "(time ./pthread/70-sum-array " +str(i)+" "+str(j)+") 2> tmp.txt 1>/dev/null && awk \'/real/ {print substr($2,3,5)}\' tmp.txt >> time.txt"
        os.system(a)

    with open('time.txt') as f:
        pthread = f.readlines()
    pthread = [x.strip() for x in pthread]


    plt.figure("70-sum-array_runtime_"+str(j))
    plt.title("70-sum-array_runtime_"+str(j))
    plt.plot(l,thread,'b',label="thread.h")
    plt.plot(l,pthread,'r',label="pthread")
    plt.xlabel('Size of Array')
    plt.ylabel('Execution Time')
    plt.legend()
    #plt.savefig(name+".png",format= 'png')
    plt.show()
os.system('rm time.txt')
os.system('rm mem_use.txt')
os.system('rm tmp.txt')
