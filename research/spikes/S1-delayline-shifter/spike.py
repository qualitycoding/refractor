"""Spike S1 (R6): verify load-bearing DSP claims for a two-tap crossfaded
delay-line pitch shifter running in an internal fixed-clock domain.
C-020: output pitch ratio of a sine equals requested ratio within tolerance.
C-021: with regeneration through the shifter and ratio>1, the tail's dominant
       frequency ascends pass by pass (EQD 'pixie trails ascend above noon').
C-022: average wet lag scales as window/(2*f_int): lowering internal clock
       lengthens the lag (Tracking behaviour, V1 clock-pot mechanism).
Deterministic: no randomness. Run: python3 spike.py"""
import numpy as np, json

def shifter(x, ratio, fs_int, win, fb=0.0, clip=True):
    N = len(x); buf = np.zeros(win*4); y = np.zeros(N); w = len(buf)
    phase = 0.0; widx = 0
    rate = (1.0 - ratio)            # delay change per sample
    for n in range(N):
        # two taps half a window apart, triangular crossfade
        d1 = (phase % 1.0) * win; d2 = ((phase + 0.5) % 1.0) * win
        def rd(d):
            p = (widx - 1 - d) % w; i = int(np.floor(p)); f = p - i
            return buf[i] * (1 - f) + buf[(i + 1) % w] * f
        g1 = 1 - abs(2*((phase % 1.0)) - 1); g2 = 1 - g1
        out = g1 * rd(d1 + 1) + g2 * rd(d2 + 1)
        inp = x[n] + fb * out
        if clip: inp = np.tanh(inp)
        buf[widx] = inp; widx = (widx + 1) % w
        y[n] = out
        phase = (phase + rate / win) % 1.0
    return y

def domfreq(y, fs):
    Y = np.abs(np.fft.rfft(y * np.hanning(len(y)), 1 << 18)); f = np.fft.rfftfreq(1 << 18, 1/fs)
    return f[np.argmax(Y)]

fs = 32768.0; win = 1024; res = {}
t = np.arange(int(fs*1.0)) / fs
for semis in [-5, -2, 0.0, 3, 4, 12, -12]:
    r = 2 ** (semis/12); x = 0.5*np.sin(2*np.pi*440*t)
    y = shifter(x, r, fs, win)[int(0.2*fs):]
    fm = domfreq(y, fs); res[f"ratio_err_{semis}st"] = abs(fm/(440*r) - 1)
# C-021 ascending trail: burst then silence with feedback
r = 2**(4/12); x = np.zeros(int(fs*1.5)); x[:int(0.05*fs)] = 0.5*np.sin(2*np.pi*300*t[:int(0.05*fs)])
y = shifter(x, r, fs, win, fb=0.85)
seg = int(0.25*fs); tail = [domfreq(y[i:i+seg], fs) for i in range(int(0.05*fs), len(y)-seg, seg)]
res["trail_freqs"] = [round(v,1) for v in tail]
# C-022 lag vs clock: impulse-ish click, unity ratio; lag measured by cross-corr
for clk in [1.0, 0.5, 0.25]:
    f_int = fs*clk; x = np.zeros(int(f_int*0.5)); x[100] = 1.0
    y = shifter(x, 1.0, f_int, win, clip=False)
    lag_s = (np.argmax(np.abs(y)) - 100) / f_int
    res[f"lag_s_clock_{clk}"] = round(lag_s, 5)
print(json.dumps(res, indent=1))

# --- refinement (round 2): finer segmentation for C-021, energy-gated ---
def trail(ratio, fb=0.9, seg_s=0.04):
    x = np.zeros(int(fs*0.8)); n0=int(0.03*fs)
    x[:n0] = 0.5*np.sin(2*np.pi*300*t[:n0])
    y = shifter(x, ratio, fs, win, fb=fb); seg=int(seg_s*fs); out=[]
    for i in range(n0, len(y)-seg, seg):
        s = y[i:i+seg]
        if np.sqrt(np.mean(s**2)) > 1e-3: out.append(round(domfreq(s, fs),1))
    return out
res2 = {"trail_up_+4st": trail(2**(4/12)), "trail_down_-5st": trail(2**(-5/12))}
print(json.dumps(res2))
