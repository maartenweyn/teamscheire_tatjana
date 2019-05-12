import matplotlib.pyplot as plt
import numpy as np
import wave
import sys


signal = np.memmap("../audio/pcm_w.raw", dtype='h', mode='r')
print ("VALUES:",signal)


# spf = wave.open('../audio/pcm_w.raw','r')

# #Extract Raw Audio from Wav File
# signal = spf.readframes(-1)
# signal = np.fromstring(signal, 'Int16')


# #If Stereo
# if spf.getnchannels() == 2:
#     print ('Just mono files')
#     sys.exit(0)

plt.figure(1)
plt.title('Signal Wave...')
plt.plot(signal)
plt.show()