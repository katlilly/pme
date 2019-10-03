#!/usr/bin/python3

from matplotlib import pyplot as plt
from numpy import genfromtxt
import numpy

data = genfromtxt("num_unique_widths_vs_mean_columns.txt", delimiter=',')
print(data.shape)

listnumbers = data[:,0]
print(listnumbers.shape)

lengths = data[:,1]

mean_num_columns = data[:,2]

num_unique_column_widths = data[:,3]

ind_short = numpy.where(lengths < 1000)
shortuniq = num_unique_column_widths[ind_short]
shortmeancols = mean_num_columns[ind_short]

ind_long = numpy.where(lengths >= 1000)
longuniq = num_unique_column_widths[ind_long]
longmeancols = mean_num_columns[ind_long]


widths = list()
for i in range(1,11) :
    widths.append(i)

eightbitlimit = list()
eightbitlimit.append(8)
eightbitlimit.append(8)
eightbitlimit.append(4)
eightbitlimit.append(4)
eightbitlimit.append(2)
eightbitlimit.append(2)
eightbitlimit.append(2)
eightbitlimit.append(2)
eightbitlimit.append(2)
eightbitlimit.append(2)


sixteenbitlimit = list()
sixteenbitlimit.append(16)
sixteenbitlimit.append(16)
sixteenbitlimit.append(8)
sixteenbitlimit.append(8)
sixteenbitlimit.append(5)
sixteenbitlimit.append(5)
sixteenbitlimit.append(5)
sixteenbitlimit.append(5)
sixteenbitlimit.append(4)
sixteenbitlimit.append(4)


thirtytwo = list()
thirtytwo.append(32)
thirtytwo.append(32)
thirtytwo.append(16)
thirtytwo.append(16)
thirtytwo.append(10)
thirtytwo.append(10)
thirtytwo.append(10)
thirtytwo.append(10)
thirtytwo.append(8)
thirtytwo.append(8)


print(widths)

#plt.plot(num_unique_column_widths, mean_num_columns, 'bo')
plt.plot(shortuniq, shortmeancols, 'o')
plt.plot(longuniq, longmeancols, 'o')

plt.plot(widths, eightbitlimit)
plt.plot(widths, sixteenbitlimit)
plt.plot(widths, thirtytwo)
plt.legend(['short lists (<1000)', 'long lists', '8bit limit','16bit limit', '32bit limit'])
plt.xlabel('number of unique column widths')
plt.ylabel('mean number of columns per word')
plt.show()
