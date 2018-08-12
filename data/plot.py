from numpy import *
import  matplotlib.pyplot as plt

a = loadtxt("grepped.txt",delimiter=",");
plt.xlabel('num of threads')
plt.ylabel('prob of race')
plt.plot(a[0:,0], 1- 1/(a[0:,1]+1),'x')
plt.show()


plt.xlabel('num of threads')
plt.ylabel('stupidness')
plt.plot(a[0:,0], a[0:,1],'x')
plt.show()
