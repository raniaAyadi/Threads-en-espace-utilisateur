# -*- coding: utf-8 -*-
import os
import time
import numpy as np
import matplotlib
import matplotlib.pyplot as plt

threadPath = './build/tst/'
pthreadPath = './build-pthread/'
postponePath = './build-postpone/'

graphPath = './graphs/'

threadColor = 'b'
pthreadColor = 'g'
postponeColor = 'r'

threadLabel = 'thread.h'
postponeLabel = 'thread.h - postpone thread exec'
pthreadLabel = 'pthread'

ylabel = "Durée (ms)"

legendLocation = "upper left"



font = {'family' : 'normal',
        'weight' : 'normal',
        'size'   : 8}

matplotlib.rc('font', **font)



def time_execution(binary, parameters):
    command = binary + " " + " ".join(parameters)

    startTime = time.time()

    os.system(command)

    endTime = time.time()

    return (endTime - startTime) * 1000

def bar_graph():
    binariesNoParam = [
        '01-main',
        '02-switch',
        '11-join',
        '12-join-main'
    ]

    threadDuration = [time_execution(threadPath + x, []) for x in binariesNoParam]
    pthreadDuration = [time_execution(pthreadPath + x, []) for x in binariesNoParam]
    postponeDuration = [time_execution(postponePath + x, []) for x in binariesNoParam]

    # data to plot
    n_groups = 4

    # create plot
    fig, ax = plt.subplots()
    index = np.arange(n_groups)
    bar_width = 0.25
    opacity = 0.8

    rects1 = plt.bar(index, threadDuration, bar_width,
                     alpha=opacity,
                     color=threadColor,
                     label=threadLabel)

    rects2 = plt.bar(index + bar_width, pthreadDuration, bar_width,
                     alpha=opacity,
                     color=pthreadColor,
                     label=pthreadLabel)

    rects3 = plt.bar(index + bar_width*2, postponeDuration, bar_width, alpha=opacity, color=postponeColor, label=postponeLabel)

    plt.ylabel(ylabel)
    plt.title("Durée d'éxecutions des programmes sans paramètre")
    plt.xticks(index + bar_width, binariesNoParam)
    plt.legend()

    plt.tight_layout()
    plt.savefig(graphPath + "noparam.png", format='png')
    plt.close()


def plot_line_graph(xlabel, title, x, threadDuration, postponeDuration, pthreadDuration):
    plt.title(title)
    plt.plot(x, threadDuration, threadColor, label=threadLabel)
    plt.plot(x, pthreadDuration, pthreadColor, label=pthreadLabel)
    plt.plot(x, postponeDuration, postponeColor, label=postponeLabel)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.legend(loc=legendLocation)


def line_graph(xlabel, title, binary, min, max, step, additionalParam = None):
    testedValues = range(min, max, step)

    if additionalParam != None:
        threadDuration = [time_execution(threadPath + binary, [str(x), str(additionalParam)]) for x in testedValues]
        postponeDuration = [time_execution(postponePath + binary, [str(x), str(additionalParam)]) for x in testedValues]
        pthreadDuration = [time_execution(pthreadPath + binary, [str(x), str(additionalParam)]) for x in testedValues]
    else:
        threadDuration = [time_execution(threadPath + binary, [str(x)]) for x in testedValues]
        postponeDuration = [time_execution(postponePath + binary, [str(x)]) for x in testedValues]
        pthreadDuration = [time_execution(pthreadPath + binary, [str(x)]) for x in testedValues]

    plot_line_graph(xlabel, title, testedValues, threadDuration, postponeDuration, pthreadDuration)

    plt.savefig(graphPath + binary + ".png", format='png')
    plt.close()



# Compile with pthread
os.system('cd build && cmake -DCMAKE_BUILD_TYPE="pthread" .. && make')
os.system('mkdir build-pthread')
os.system('cp build/tst/*-* build-pthread/')

# Compile with thread.h postpone thread execution
os.system('cd build && cmake -DCMAKE_BUILD_TYPE="postpone" .. && make')
os.system('mkdir build-postpone')
os.system('cp build/tst/*-* build-postpone/')

# Compile with thread.h
os.system('cd build && cmake -DCMAKE_BUILD_TYPE="Default" .. && make')



# First tests, no parameter:
bar_graph()


# Tests with number of threads as parameter:
binariesNbThreads = [
    ['21-create-many', 5, 10000, 50],
    ['22-create-many-recursive', 5, 2000, 10],
    ['23-create-many-once', 5, 10000, 50],
    ['61-mutex', 5, 2000, 50]
]

for x in binariesNbThreads:
    line_graph("Nombre de threads", "Durée d'exécution de " + x[0], *x)


#Tests with number of threads and number of yields as parameter:
binariesNbThreadsNbYields = [
    ['31-switch-many', 5, 2000, 50, 1000],
    ['32-switch-many-join', 5, 3500, 50, 1000]
]

for x in binariesNbThreadsNbYields:
    line_graph("Nombre de threads", "Durée d'éxecution de " + x[0] + " avec 1000 yields", *x)


# Tests with a value as parameter:
binariesValue = [
    ['51-fibonacci', 5, 18, 1],
    ['70-sum-array', 5, 3000, 25],
    ['71-sort-array', 5, 1000, 3],
    ['72-map-array', 5, 600, 2]
]

for x in binariesValue:
    line_graph("Valeur", "Durée d'exécution de " + x[0], *x)




os.system("rm -rf build-pthread")
os.system("rm -rf build-postpone")
