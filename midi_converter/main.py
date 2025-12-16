#!/usr/bin/env python3
# Best stable MIDI → ITMO Loops converter (clean sound, no surprises)

from __future__ import annotations
import argparse
from dataclasses import dataclass
from typing import Dict, List, Tuple
import mido

NOTE_NAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]


def midi_pitch_to_name(p: int) -> str:
    return f"{NOTE_NAMES[p % 12]}{p // 12 - 1}"


def vel_127_to_100(v: int) -> int:
    return max(20, min(80, int(v * 0.7)))


def ticks_to_units(t: int, tpq: int, res: int) -> int:
    return max(0, round(t * res / tpq))


def read_bpm(mid: mido.MidiFile) -> int:
    for tr in mid.tracks:
        for m in tr:
            if m.type == "set_tempo":
                return int(mido.tempo2bpm(m.tempo))
    return 120


@dataclass
class Note:
    start: int
    inst: str
    pitch: str
    dur: int
    vel: int
    midi: int


# ---------- MIDI parsing ----------

def extract(mid: mido.MidiFile, res: int) -> Tuple[List[Note], Dict[str, bool]]:
    tpq = mid.ticks_per_beat
    notes: List[Note] = []
    has_drums: Dict[str, bool] = {}

    for i, tr in enumerate(mid.tracks):
        name = f"inst{i}"
        abs_t = 0
        active: Dict[Tuple[int, int], Tuple[int, int]] = {}
        note_count = 0

        for m in tr:
            abs_t += m.time
            if getattr(m, "channel", -1) == 9:
                has_drums[name] = True

            if m.type == "note_on" and m.velocity > 0:
                active[(getattr(m, "channel", 0), m.note)] = (abs_t, m.velocity)

            elif m.type in ("note_off", "note_on"):
                key = (getattr(m, "channel", 0), m.note)
                if key not in active:
                    continue
                st, vel = active.pop(key)
                note_count += 1
                notes.append(Note(
                    start=ticks_to_units(st, tpq, res),
                    inst=name,
                    pitch=midi_pitch_to_name(m.note),
                    dur=max(1, ticks_to_units(abs_t - st, tpq, res)),
                    vel=vel_127_to_100(vel),
                    midi=m.note,
                ))

        # filter out empty / meta-only tracks
        if note_count == 0:
            has_drums[name] = True  # mark as ignorable

    notes.sort(key=lambda n: (n.start, n.inst))
    return notes, has_drums


# ---------- ITMO Loops ----------

def emit(midipath: str, bpm: int, res: int, notes: List[Note], has_drums: Dict[str, bool]) -> str:
    out: List[str] = []
    out += [f"# {midipath}", f"bpm {bpm}", ""]

    insts = sorted({n.inst for n in notes})

    # Stable, safe instruments only
    for inst in insts:
        if has_drums.get(inst):
            continue

        idx = int(inst[4:])
        if idx == 0:
            # bass
            out += [
                f"instrument {inst} sampler",
                "    sample=./samples/fbass.wav",
                "    root=C4",
                "    loop=4839,5844",
                "    effect gain gain=0.5",
                "end", ""
            ]
        elif idx == 1:
            # lead (safe + tastier)
            out += [
                f"instrument {inst} triangle",
                "    attack=0.01",
                "    release=0.14",
                "    effect gain gain=0.32",
                "    effect tremolo freq=6 depth=0.12",
                "    effect echo delay=0.19 decay=0.12",
                "end", ""
            ]
        else:
            # harmony (warmer) + optional soft pad layer
            out += [
                f"instrument {inst} sampler",
                "    sample=./samples/pianom.wav",
                "    root=C5",
                "    attack=0.001",
                "    release=0.12",
                "    effect gain gain=0.28",
                "end", ""
            ]

    # optional pad (strings) to make the main sound richer
    out += [
        "instrument pad sampler",
        "    sample=./samples/str.wav",
        "    root=C7",
        "    loop=60,16777",
        "    attack=0.025",
        "    release=0.3",
        "    effect gain gain=0.18",
        "end", ""
    ]

    # drums (fixed)
    out += [
        "instrument kick sampler",
        "    sample=./samples/kick.wav",
        "    root=C5",
        "    release=0.01",
        "    effect gain gain=0.4",
        "end", "",
        "instrument snare sampler",
        "    sample=./samples/snare.wav",
        "    root=C5",
        "    release=0.03",
        "    effect gain gain=0.35",
        "end", "",
        "instrument hh sampler",
        "    sample=./samples/hihc.wav",
        "    root=C5",
        "    release=0.01",
        "    effect gain gain=0.25",
        "end", "",
    ]

    out.append(f"pattern main resolution {res}")
    for n in notes:
        if has_drums.get(n.inst):
            out.append(f"    {n.start:02d} kick C5 1 {max(40, n.vel)}")
        else:
            out.append(f"    {n.start:02d} {n.inst} {n.pitch} {n.dur} {n.vel}")
            # add soft pad under longer harmony notes (taste, not mud)
            idx = int(n.inst[4:])
            if idx >= 2 and n.dur >= 4:
                pad_vel = max(10, int(n.vel * 0.35))
                pad_dur = n.dur + 2
                out.append(f"    {n.start:02d} pad {n.pitch} {pad_dur} {pad_vel}")
    out.append("end")

    return "\n".join(out)


# ---------- main ----------

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("midi")
    ap.add_argument("-o", "--out")
    ap.add_argument("--resolution", type=int, default=16)
    a = ap.parse_args()

    mid = mido.MidiFile(a.midi)
    txt = emit(a.midi, read_bpm(mid), a.resolution, *extract(mid, a.resolution))

    if a.out:
        open(a.out, "w", encoding="utf-8").write(txt)
    else:
        print(txt)


if __name__ == "__main__":
    main()
