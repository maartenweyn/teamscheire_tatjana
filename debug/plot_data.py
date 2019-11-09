import matplotlib.pyplot as plt
import numpy as np

import pandas as pd


#bank, raw, avg, corr = np.loadtxt('sounddata.csv', delimiter=',', unpack=True)

df = pd.read_csv('sounddata.csv', header=None, names=['id','raw','avg','corr','filtered','sum'], error_bad_lines=False)

fig = plt.figure()

plt.plot(df['raw'], label='Sound')
#plt.plot(df['id'], label='id')
plt.plot(df['avg'], label='avg')
plt.plot(df['corr'], label='corr')
plt.plot(df['filtered'], label='filtered')

plt.ylabel('y')
plt.title('Sound')
plt.legend()
plt.show()
