#!/usr/bin/python3

import numpy as np
from matplotlib import pyplot as plt
import csv
from numpy import genfromtxt

data = genfromtxt("alllists_unique_selector_stats.txt")
length = int(data.shape[0] / 3)
print(length)

listnumbers = list()
selectors = list()
unique_selectors = list()

for index in range(length):
    listnumbers.append(int(data[index * 3]))
    selectors.append(int(data[index * 3 + 1]))
    unique_selectors.append(int(data[index * 3 + 2]))

    
plt.plot(range(600), range(600))
plt.plot(selectors, unique_selectors, 'bo')
plt.xlabel("number of selectors")
plt.ylabel("number of unique selectors")
plt.savefig("alllists_unique_selectors.png")
plt.show()
