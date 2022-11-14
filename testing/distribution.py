import subprocess
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

BYTES = 1000000
CMD = f"od -vAn -N{BYTES} -tu1 < /dev/qrandom0"

result = subprocess.run(['sudo', 'bash', '-c', CMD], stdout=subprocess.PIPE).stdout.decode('utf-8')
numbers = [int(x) for line in result.split('\n') for x in line.split(' ') if x]

x = np.arange(0,256,1)
y = np.zeros(256,dtype=int)

for number in numbers:
    y[number] += 1

fig, ax = plt.subplots()

# Be sure to only pick integer tick locations.
for axis in [ax.xaxis, ax.yaxis]:
    axis.set_major_locator(ticker.MaxNLocator(integer=True))

ax.bar(x,y,width=1)

# Just for appearance's sake
ax.margins(0.05)
ax.axis('tight')
fig.tight_layout()

plt.show()