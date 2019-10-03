#!/usr/bin/python3

from matplotlib import pyplot as plt
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

    
plt.plot(range(500), range(500))
plt.plot(selectors, unique_selectors, 'bo')
plt.xlim(left=0)
plt.ylim(bottom=0)
plt.xlabel("number of selectors")
plt.ylabel("number of unique selectors")
plt.savefig("alllists_unique_selectors.png")
plt.show()
