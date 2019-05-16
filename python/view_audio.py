import matplotlib.pyplot as plt
import numpy as np
import wave
import sys

AFILTER_Acoef = [1.0, -4.0195761811158315, 6.1894064429206921, -4.4531989035441155,
                                1.4208429496218764, -0.14182547383030436,
                                0.0043511772334950787]
AFILTER_Bcoef = [0.2557411252042574, -0.51148225040851436,
                                -0.25574112520425807, 1.0229645008170318,
                                -0.25574112520425918, -0.51148225040851414,
                                0.25574112520425729]


def AFilter(input):
  AFILTER_conditions = np.zeros(6)
  # double[] output = new double[input.length];
  #       for (int i = 0; i < input.length; i++) {
  #           double x_i = input[i];
  #           //Filter sample:
  #           double y_i = x_i * bCoef[0] + conditions[0];
  #           //Store filtered sample:
  #           output[i] = y_i;
  #           //Adjust conditions:
  #           // all but the last condition:
  #           for (int j = 0; j < order - 1; j++)
  #               conditions[j] = x_i * bCoef[j + 1]
  #                       - y_i * aCoef[j + 1]
  #                       + conditions[j + 1];
  #           // last condition:
  #           conditions[order - 1] = x_i * bCoef[order]
  #                   - y_i * aCoef[order];
  #       }
  #       if (!keepConditions)
  #           conditions = new double[order]; //reset conditions
  #       return output;

  output = np.zeros(len(input), dtype=float)
  # print ("lenght input:", len(input))
  # print ("lenght output:", len(output))
  for i in range(0, len(input)-1):
    output[i] = input[i] * AFILTER_Bcoef[0] + AFILTER_conditions[0]
    # print(i, input[i], output[i], AFILTER_conditions)
    for j in range(0, 5):
      AFILTER_conditions[j] = input[i] * AFILTER_Bcoef[j + 1] - output[i] * AFILTER_Acoef[j + 1] + AFILTER_conditions[j + 1]
    AFILTER_conditions[5] = input[i] * AFILTER_Bcoef[6] - output[i] * AFILTER_Acoef[6]

  return output

STEPS = int(44100 / 1)
print ("STEPS:", STEPS)

signal = np.memmap("audio/pcm_w.raw", dtype='u1', mode='r') #u8
signal = signal.astype('f')-128
signal[signal < 0] /= 128.0
signal[signal >= 0] /= 127.0
print ("VALUES:",signal)
print ("lenght:", len(signal))

filtered = np.array([])

leqts = []
leqvalue = []
# AFilter(signal[0:1000])



for i in range(0, len(signal), STEPS):
  maxrange = min(STEPS, len(signal) - i)
  print(i, i+maxrange)
  f = AFilter(signal[i:i+maxrange])
  filtered = np.concatenate((filtered, f))
  sumsquare = 0.0
  leq = 0.0
  for j in range(0, maxrange):
    sumsquare += f[j] * f[j]
  leq = (10.0 * np.log10(sumsquare / maxrange)) + 93.97940008672037609572522210551
  leqts.append(i)
  leqvalue.append(leq)
  print(i, leq)

#spf = wave.open('audio/pcm_w.raw','r')

# #Extract Raw Audio from Wav File
# signal = spf.readframes(-1)
# signal = np.fromstring(signal, 'Int16')


# #If Stereo
# if spf.getnchannels() == 2:
#     print ('Just mono files')
#     sys.exit(0)

fig01 = plt.figure(1)
ax = fig01.add_subplot(311)
ax.set_title('Signal Wave...')
ax.plot(signal)

ax = fig01.add_subplot(312)
ax.set_title('AWeighted...')
print(filtered)
ax.plot(filtered)

ax = fig01.add_subplot(313)
ax.plot(leqts, leqvalue)
#plt.show()

plt.savefig('audio/pcm_w.png')

