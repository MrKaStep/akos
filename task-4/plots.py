import pylab
from matplotlib import mlab


xlist = mlab.frange(1<<18, 1 << 26, 1 << 18)
inp = open("times", "r")

ylist = []

for i in range(5):
    ylist.append([])
    for j in range(256):
        ylist[i].append(int(inp.readline().split()[0]))

pylab.plot(xlist, ylist[0], label="Linear")
pylab.plot(xlist, ylist[1], label="Step 333")
pylab.plot(xlist, ylist[2], label="Step 2^10")
pylab.plot(xlist, ylist[3], label="Step 2^15")
pylab.plot(xlist, ylist[4], label="Random")

pylab.xlabel("Array size")
pylab.ylabel("Time in ns")

pylab.legend()

pylab.savefig("plot.png")
pylab.show()
