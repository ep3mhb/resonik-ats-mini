"""Waterfall pictures of each digital mode for the manuals' appendix A
("Learning the digital modes").

Each mode's audio is computed (as the radio hears it in USB, with noise),
then drawn as a waterfall from 300 to 2700 Hz in the colours of the Decodes
tab of Web Control, newest at the top. The pictures are simulations, and the
captions say so.

Usage: python3 tools/wf_examples.py [out_dir]   (default: docs/manual-fa/img)
Writes wf-ft8.jpg, wf-ft4.jpg, wf-cw.jpg, wf-rtty.jpg, wf-dwd.jpg,
wf-navtex.jpg, wf-psk.jpg and wf-fax.jpg; both manuals use them.
Needs numpy, scipy, Pillow and the Inter font (fonts-inter).
"""
import os
import sys

import numpy as np
from scipy.ndimage import gaussian_filter1d
from PIL import Image, ImageDraw, ImageFont

FS = 8000
LO, HI = 300.0, 2700.0
W, H = 1120, 400          # Waterfall in pixels (drawn at 2x for print)
rng = np.random.default_rng(7)
FONT = "/usr/share/fonts/opentype/inter/Inter-Medium.otf"
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "..", "docs", "manual-fa", "img")


def noise(n, level=1.0):
    x = rng.normal(0, 1, n)
    X = np.fft.rfft(x)
    f = np.fft.rfftfreq(n, 1 / FS)
    m = np.clip((f - 200) / 120, 0, 1) * np.clip((2950 - f) / 200, 0, 1)
    return np.fft.irfft(X * m, n) * level


def fsk(freqs_per_sample, amp=1.0, ramp=0.004):
    ph = 2 * np.pi * np.cumsum(freqs_per_sample) / FS
    y = np.sin(ph) * amp
    return y


def place(buf, sig, t0, env=None):
    i = int(t0 * FS)
    j = min(len(buf), i + len(sig))
    if env is not None:
        sig = sig * env
    buf[i:j] += sig[: j - i]


def keyed_env(n, ramp=0.006):
    e = np.ones(n)
    r = int(ramp * FS)
    if r and n > 2 * r:
        w = 0.5 - 0.5 * np.cos(np.linspace(0, np.pi, r))
        e[:r] = w
        e[-r:] = w[::-1]
    return e


def amp_for(snr_db):
    # Noise power in 2500 Hz is about 2500/4000 of the unit white noise
    pn = 2500 / (FS / 2)
    return np.sqrt(2 * pn * 10 ** (snr_db / 10))


# ---------------- the modes ----------------

def ft8_signal(f0, snr, sym=0.16, nsym=79, spacing=6.25, ntone=8, costas=(3, 1, 4, 0, 6, 5, 2)):
    tones = rng.integers(0, ntone, nsym)
    if costas:
        for p in (0, 36, 72):
            if p + 7 <= nsym:
                tones[p:p + 7] = costas
    spb = int(sym * FS)
    f = np.repeat(f0 + tones * spacing, spb).astype(float)
    f = gaussian_filter1d(f, spb * 0.25)
    y = fsk(f, amp_for(snr))
    return y * keyed_env(len(y), 0.02)


def ft8_band(T=60.0):
    buf = noise(int(T * FS))
    even = [(520, -8), (1140, -15), (1610, 2), (2240, -19), (900, -21)]
    odd = [(780, -4), (1330, -12), (1905, -17), (2420, -9)]
    for slot in range(int(T // 15)):
        for f0, snr in (even if slot % 2 == 0 else odd):
            place(buf, ft8_signal(f0 + rng.uniform(-3, 3), snr + rng.uniform(-2, 2)), slot * 15 + 0.5 + rng.uniform(0, 0.4))
    return buf, T


def ft4_band(T=30.0):
    buf = noise(int(T * FS))
    even = [(700, -6), (1420, -10), (2050, 0)]
    odd = [(1000, -3), (1750, -12)]
    for slot in range(int(T // 7.5)):
        for f0, snr in (even if slot % 2 == 0 else odd):
            sig = ft8_signal(f0, snr + rng.uniform(-2, 2), sym=0.048, nsym=103, spacing=20.833, ntone=4, costas=None)
            place(buf, sig, slot * 7.5 + 0.5 + rng.uniform(0, 0.3))
    return buf, T


MORSE = {'A': '.-', 'B': '-...', 'C': '-.-.', 'D': '-..', 'E': '.', 'F': '..-.', 'G': '--.', 'H': '....', 'I': '..',
         'J': '.---', 'K': '-.-', 'L': '.-..', 'M': '--', 'N': '-.', 'O': '---', 'P': '.--.', 'Q': '--.-', 'R': '.-.',
         'S': '...', 'T': '-', 'U': '..-', 'V': '...-', 'W': '.--', 'X': '-..-', 'Y': '-.--', 'Z': '--..',
         '0': '-----', '1': '.----', '2': '..---', '3': '...--', '4': '....-', '5': '.....', '6': '-....',
         '7': '--...', '8': '---..', '9': '----.', '/': '-..-.'}


def morse_env(text, wpm):
    u = 1.2 / wpm
    seq = []
    for wi, word in enumerate(text.split(' ')):
        if wi:
            seq.append((0, 7))
        for ci, ch in enumerate(word):
            if ci:
                seq.append((0, 3))
            for ei, el in enumerate(MORSE[ch]):
                if ei:
                    seq.append((0, 1))
                seq.append((1, 1 if el == '.' else 3))
    n = int(sum(d for _, d in seq) * u * FS)
    e = np.zeros(n)
    i = 0
    for on, d in seq:
        k = int(d * u * FS)
        if on:
            e[i:i + k] = keyed_env(k, 0.005)
        i += k
    return e


def cw_band(T=8.0):
    buf = noise(int(T * FS))
    for f0, wpm, txt, snr, t0 in ((700, 18, 'CQ CQ DE K1ABC K1ABC K', 6, 0.3),
                                  (1150, 28, 'TU 5NN 14 TU', -2, 1.0),
                                  (1830, 12, 'R R TNX FER CALL', 2, 0.2)):
        e = morse_env(txt, wpm)
        y = np.sin(2 * np.pi * f0 * np.arange(len(e)) / FS) * amp_for(snr) * e
        place(buf, y, t0)
    return buf, T


ITA2 = {'A': 3, 'B': 25, 'C': 14, 'D': 9, 'E': 1, 'F': 13, 'G': 26, 'H': 20, 'I': 6, 'J': 11, 'K': 15, 'L': 18,
        'M': 28, 'N': 12, 'O': 24, 'P': 22, 'Q': 23, 'R': 10, 'S': 5, 'T': 16, 'U': 7, 'V': 30, 'W': 19, 'X': 29,
        'Y': 21, 'Z': 17, ' ': 4, '\n': 2}
FIGS = {'1': 'Q', '2': 'W', '3': 'E', '4': 'R', '5': 'T', '6': 'Y', '7': 'U', '8': 'I', '9': 'O', '0': 'P', '-': 'A', '/': 'X', '.': 'M'}


def rtty_bits(text):
    bits = []
    figs = False
    for ch in text:
        if ch in FIGS:
            if not figs:
                bits += [0] + [(27 >> k) & 1 for k in range(5)] + [1, 1]
                figs = True
            code = ITA2[FIGS[ch]]
        else:
            if figs and ch not in ' \n':
                bits += [0] + [(31 >> k) & 1 for k in range(5)] + [1, 1]
                figs = False
            code = ITA2[ch]
        bits += [0] + [(code >> k) & 1 for k in range(5)] + [1, 1]   # 1.5 stop bits rounded to 2 halves below
    return bits


def rtty_signal(text, baud, shift, centre, snr):
    bits = rtty_bits(text)
    spb = FS / baud
    f = np.concatenate([np.full(int(round(spb * (0.75 if (i % 8 == 7) else 1.0))), centre + (shift / 2 if b else -shift / 2))
                        for i, b in enumerate(bits)])
    f = gaussian_filter1d(f.astype(float), spb * 0.08)
    return fsk(f, amp_for(snr))


def rtty_band(T=8.0):
    buf = noise(int(T * FS))
    place(buf, rtty_signal('CQ TEST K1ABC K1ABC CQ TEST K1ABC K1ABC CQ TEST', 45.45, 170, 1500, 8), 0.2)
    place(buf, rtty_signal('G4XYZ 599 14 14 G4XYZ', 45.45, 170, 2215, -1), 2.5)
    return buf, T


def dwd_band(T=8.0):
    buf = noise(int(T * FS))
    txt = 'RYRYRYRYRYRYRYRY\nFQEN70 EDZW 221100\nWEATHER REPORT NORTH SEA AND BALTIC SEA'
    place(buf, rtty_signal(txt, 50, 450, 1500, 6), 0.0)
    return buf, T


def navtex_band(T=8.0):
    buf = noise(int(T * FS))
    n = int(T * 100)
    bits = []
    # phasing first (alternating pattern), then characters of 4 marks in 7
    for k in range(n // 7):
        if k < 30:
            bits += [1, 1, 1, 1, 0, 0, 0] if k % 2 else [1, 0, 1, 0, 1, 1, 0]
        else:
            c = [1, 1, 1, 1, 0, 0, 0]
            rng.shuffle(c)
            bits += c
    spb = FS / 100
    f = np.repeat(np.array([1585 if b else 1415 for b in bits], float), int(spb))
    f = gaussian_filter1d(f, spb * 0.08)
    sig = fsk(f, amp_for(4))
    place(buf, sig, 0.6)
    return buf, T


VARICODE = {' ': '1', 'a': '1011', 'b': '1011111', 'c': '101111', 'd': '101101', 'e': '11', 'f': '111101',
            'g': '1011011', 'h': '101011', 'i': '1101', 'j': '111101011', 'k': '10111111', 'l': '11011',
            'm': '111011', 'n': '1111', 'o': '111', 'p': '111111', 'q': '110111111', 'r': '10101', 's': '10111',
            't': '101', 'u': '110111', 'v': '1111011', 'w': '1101011', 'x': '11011111', 'y': '1011101',
            'z': '111010101', '0': '10110111', '1': '10111101', '2': '11101101', '3': '11111111',
            '4': '101110111', '5': '101011011', '6': '101101011', '7': '110101101', '8': '110101011',
            '9': '110110111'}


def psk_signal(text, f0, snr, idle_before=1.5, idle_after=1.0):
    T = 1 / 31.25
    bits = '0' * int(idle_before / T) + ''.join(VARICODE[c] + '00' for c in text) + '0' * int(idle_after / T)
    spb = int(round(T * FS))
    out = np.zeros(len(bits) * spb)
    sign = 1.0
    t = np.arange(spb) / spb
    for i, b in enumerate(bits):
        if b == '0':   # phase reversal, shaped through zero
            seg = sign * np.cos(np.pi * t)
            sign = -sign
        else:
            seg = np.full(spb, sign)
        out[i * spb:(i + 1) * spb] = seg
    car = np.sin(2 * np.pi * f0 * np.arange(len(out)) / FS)
    return out * car * amp_for(snr)


def psk_band(T=8.0):
    buf = noise(int(T * FS))
    place(buf, psk_signal('cq cq cq de k1abc k1abc pse k', 1000, 4, 1.2, 0.5), 0.2)
    place(buf, psk_signal('tnx for the call name is bob', 1460, -4, 0.3, 0.2), 1.6)
    place(buf, psk_signal('', 1780, -6, 7.5, 0), 0.0)
    return buf, T


def fax_band(T=24.0):
    buf = noise(int(T * FS))
    spl = int(0.5 * FS)            # 120 lines a minute
    parts = []
    # start tone: black and white alternating at 300 Hz for 4 s
    n = int(4.0 * FS)
    sq = (np.sign(np.sin(2 * np.pi * 300 * np.arange(n) / FS)) + 1) / 2
    parts.append(sq)
    # phasing: white for 25 ms, then black, each line, 6 s
    for _ in range(12):
        line = np.zeros(spl)
        line[:int(0.025 * FS)] = 1
        parts.append(line)
    # the chart: white paper with dark isobars, coast and lettering
    rows = int((T - 10.0) / 0.5)
    x = np.linspace(0, 1, spl)
    for r in range(rows):
        b = np.ones(spl)
        for k in range(7):
            pos = (0.12 * k + 0.05 + 0.04 * np.sin(0.35 * r + k)) % 1
            b[np.abs(x - pos) < 0.004] = 0
        if 6 < r < 18:
            b[(x > 0.62) & (x < 0.62 + 0.01 * (r - 6))] = 0.1
        parts.append(b)
    bright = np.concatenate(parts)
    f = 1500 + 800 * bright
    f = gaussian_filter1d(f.astype(float), 3)
    place(buf, fsk(f, amp_for(12)), 0.0)
    return buf, T


# ---------------- drawing ----------------

def colour(v):
    v = np.clip(v, 0, 255)
    r = np.minimum(255, v * 1.6)
    g = np.minimum(255, v * 1.2)
    b = 80 + v * 0.6
    return np.stack([r, g, b], -1).astype(np.uint8)


def waterfall(x, T, nfft, hop, rng_db=34.0, boost=0.0):
    win = np.hanning(nfft)
    frames = 1 + (len(x) - nfft) // hop
    S = np.empty((frames, nfft // 2 + 1))
    for i in range(frames):
        seg = x[i * hop:i * hop + nfft] * win
        S[i] = np.abs(np.fft.rfft(seg)) ** 2
    f = np.fft.rfftfreq(nfft, 1 / FS)
    sel = (f >= LO) & (f <= HI)
    S = S[:, sel]
    f = f[sel]
    from scipy.ndimage import uniform_filter
    S = uniform_filter(S, size=(2, 3))     # a little averaging, as a receiver's waterfall does
    db = 10 * np.log10(S + 1e-12)
    floor = np.percentile(db, 80) + boost
    v = 26 + (db - floor) / rng_db * 229
    # columns: average the bins that fall in each pixel, rows: nearest
    cols = np.interp(np.linspace(LO, HI, W), f, np.arange(len(f)))
    ci = np.clip(np.round(cols).astype(int), 0, len(f) - 1)
    ri = np.clip(np.round(np.linspace(0, frames - 1, H)).astype(int), 0, frames - 1)
    img = v[ri][:, ci]
    img = img[::-1]               # newest at the top, as on the Decodes tab
    return colour(img)


def frame(rgb, T, ticks_s, name):
    pad_l, pad_b, pad_t, pad_r = 92, 64, 10, 18
    im = Image.new('RGB', (W + pad_l + pad_r, H + pad_t + pad_b), (255, 255, 255))
    im.paste(Image.fromarray(rgb), (pad_l, pad_t))
    d = ImageDraw.Draw(im)
    fnt = ImageFont.truetype(FONT, 26)
    d.rectangle([pad_l - 1, pad_t - 1, pad_l + W, pad_t + H], outline=(7, 58, 75), width=2)
    for hz in (500, 1000, 1500, 2000, 2500):
        x = pad_l + (hz - LO) / (HI - LO) * W
        d.line([x, pad_t + H, x, pad_t + H + 10], fill=(7, 58, 75), width=2)
        t = str(hz)
        d.text((x - d.textlength(t, font=fnt) / 2, pad_t + H + 14), t, fill=(7, 58, 75), font=fnt)
    d.text((pad_l + W - d.textlength('Hz', font=fnt), pad_t + H + 14 + 0), '', font=fnt)
    t = 'Hz'
    d.text((pad_l + W + pad_r - d.textlength(t, font=fnt) - 2, pad_t + H + 14), t, fill=(7, 58, 75), font=fnt)
    for s in ticks_s:
        y = pad_t + (s / T) * H
        d.line([pad_l - 10, y, pad_l, y], fill=(7, 58, 75), width=2)
        lab = '%g s' % s
        d.text((pad_l - 14 - d.textlength(lab, font=fnt), y - 15), lab, fill=(7, 58, 75), font=fnt)
    im.save(os.path.join(OUT, name + '.jpg'), quality=86, subsampling=0, optimize=True)
    return im


if __name__ == '__main__':
    os.makedirs(OUT, exist_ok=True)
    jobs = [
        ('wf-ft8', ft8_band, 4096, 1000, (0, 15, 30, 45, 60)),
        ('wf-ft4', ft4_band, 2048, 400, (0, 7.5, 15, 22.5, 30)),
        ('wf-cw', cw_band, 512, 96, (0, 2, 4, 6, 8)),
        ('wf-rtty', rtty_band, 1024, 160, (0, 2, 4, 6, 8)),
        ('wf-dwd', dwd_band, 1024, 160, (0, 2, 4, 6, 8)),
        ('wf-navtex', navtex_band, 1024, 160, (0, 2, 4, 6, 8)),
        ('wf-psk', psk_band, 2048, 160, (0, 2, 4, 6, 8)),
        ('wf-fax', fax_band, 1024, 320, (0, 6, 12, 18, 24)),
    ]
    for name, fn, nfft, hop, ticks in jobs:
        x, T = fn()
        rgb = waterfall(x, T, nfft, hop)
        frame(rgb, T, ticks, name)
        print(name, 'ok')
