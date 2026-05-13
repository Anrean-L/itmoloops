# ITMO Loops

A C++ application that parses a small music DSL and renders it into a mono `.wav` file.

The project implements a complete audio generation pipeline: parsing a textual score, building an internal track model, generating instrument signals, applying effects, mixing voices and writing the final WAV file.

```bash
./itmoloops score.txt music.wav
```

## Features

* Custom DSL for describing musical compositions
* Pattern system with nested pattern calls
* Polyphonic instruments
* WAV reading and writing
* 44.1 kHz, 16-bit mono output
* Attack-release envelope support
* Sample-based and oscillator-based instruments
* Per-instrument audio effects
* Modular architecture for adding new instruments and effects

## DSL Example

```
bpm 120

instrument sax sampler
    sample=./samples/sax.wav
    root=C5
    loop=8893,9229
    attack=0.001
    release=0.05
    effect echo delay=0.3 decay=0.2
end

pattern main resolution 8
    00 sax D5  2 50
    05 sax E5  2 50
    08 sax F5  2 50
    13 sax G5  2 50
    16 sax E5  4 50
    23 sax C5  2 50
    28 sax D5  8 50
end
```

## Implemented Instruments

### Sampler

Loads a WAV file and plays it with pitch shifting based on the target note.

Supported parameters:

* `sample` — path to a WAV file
* `root` — root note of the sample
* `loop` — optional loop range in samples
* `attack` — attack duration in seconds
* `release` — release duration in seconds

### Oscillators

The project also supports generated waveforms:

* `sine`
* `square`
* `triangle`

All oscillators support attack-release envelopes. The `square` oscillator also supports configurable duty cycle.

## Implemented Effects

### Gain

Multiplies the signal by a constant factor.

```text
effect gain gain=0.4
```

### Echo

Adds a delayed and attenuated copy of the signal.

```text
effect echo delay=0.3 decay=0.2
```

### Tremolo

Applies periodic amplitude modulation.

```text
effect tremolo freq=10 depth=0.5
```

## Patterns

Patterns are named blocks of musical commands. They can be reused inside other patterns, similarly to function calls.

```text
pattern phrase resolution 8
    00 piano C5 2 50
    04 piano E5 2 50
end

pattern main resolution 8
    00 @phrase
    08 @phrase
end
```

The `main` pattern is used as the entry point of the composition.

## Architecture

The rendering pipeline is split into several stages:

```text
DSL source
    - lexer/parser
    - internal track model
    - note scheduling
    - instrument rendering
    - effect processing
    - mixing
    - WAV writing
```

The implementation separates parsing, validation, audio generation and file output. Instruments and effects are represented through abstractions, so new sound sources or signal processors can be added without changing the whole rendering pipeline.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/itmoloops examples/score.txt output.wav
```

## Technologies

* C++
* CMake
* OOP
* RAII
* Polymorphism
* WAV file format
* Digital signal processing
