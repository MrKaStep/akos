import pylab
from matplotlib import mlab

xlist = mlab.frange(1<<18, 1 << 26, 1 << 18)
inp = open("times", "r")

ylist = []

for l in inp:
    ylist.append(int(l.split()[0]))

pylab.plot(xlist, ylist)
pylab.show()
