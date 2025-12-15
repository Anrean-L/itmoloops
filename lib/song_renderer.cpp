#include "song_renderer.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numbers>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "config.hpp"
#include "wav_file.hpp"

namespace itmoloops {

namespace {

uint32_t UnitsToSamples(uint32_t units, uint32_t resolution, uint32_t bpm) {
    return (float)kSecondsInMinute / bpm / resolution * units * kFrequency;
}

template <typename T>
bool PairComparator(const std::pair<std::string, T>& pair,
                    const std::string& value) {
    return pair.first < value;
}

template <typename T>
size_t Find(const std::string& name,
            const std::vector<std::pair<std::string, T>>& mapping) {
    return std::lower_bound(mapping.begin(), mapping.end(), name,
                            PairComparator<T>) -
           mapping.begin();
}

}  // namespace

bool operator<(ScheduledNote& a, ScheduledNote& b) {
    return a.start_sample < b.start_sample;
}

float Envelope(uint32_t t, uint32_t note_len, uint32_t attack,
               uint32_t release) {
    if (t < attack && attack > 0.f) {
        return (float)t / attack;
    }
    if (t < note_len) {
        return 1.f;
    }
    uint32_t cur_release = t - note_len;
    if (cur_release < release) {
        return 1.f - (float)cur_release / release;
    }
    return 0.f;
}

float Instrument::ProcessSample(uint32_t sample) {
    float out = 0.f;
    uint32_t erased = 0;
    for (size_t i = 0; i < voices_.size(); ++i) {
        Voice& v = *voices_[i];
        if (v.note.end_sample + release_ <= sample) {
            ++erased;
            continue;
        }

        float env = Envelope(sample - v.note.start_sample,
                             v.note.end_sample - v.note.start_sample, attack_,
                             release_);
        out += GenerateVoiceSample(v, sample) * env * (v.note.note.velocity);
        if (erased > 0) {
            voices_[i - erased] = std::move(voices_[i]);
        }
    }
    voices_.resize(voices_.size() - erased);
    return out;
}

SamplerInstrument::SamplerInstrument(std::string sample_path,
                                     float root_frequency, uint32_t loop_start,
                                     uint32_t loop_end, float attack,
                                     float release)
    : Instrument(attack, release),
      root_frequency_(root_frequency),
      loop_start_(loop_start),
      loop_end_(loop_end) {
    WavReader wav_reader(sample_path);
    if (wav_reader.CheckValidity()) {
        const std::vector<int16_t>& raw_sample = wav_reader.GetData();
        sample_.reserve(raw_sample.size());
        for (int16_t value : raw_sample) {
            sample_.push_back((float)value / INT16_MAX);
        }
    }
}

float SamplerInstrument::GenerateVoiceSample(Voice& v, uint32_t sample) {
    if (root_frequency_ == 0.f) {
        return 0.f;
    }
    auto& sampler_v = (SamplerVoice&)(v);
    if (sampler_v.sample_pos >= sample_.size()) {
        return 0.f;
    }
    float out = sample_[(size_t)(sampler_v.sample_pos)];

    float ratio = sampler_v.note.note.frequency / root_frequency_;
    sampler_v.sample_pos += ratio;

    if (loop_end_ != 0 && sampler_v.sample_pos >= loop_end_) {
        sampler_v.sample_pos = loop_start_;
    }
    return out;
}

float SineInstrument::GenerateVoiceSample(Voice& v, uint32_t sample) {
    uint32_t local_sample = sample - v.note.start_sample;
    float phase = 2.f * std::numbers::pi * v.note.note.frequency *
                  local_sample / kFrequency;
    return std::sin(phase);
}

float SquareInstrument::GenerateVoiceSample(Voice& v, uint32_t sample) {
    uint32_t local_sample = sample - v.note.start_sample;

    float period = kFrequency / v.note.note.frequency;
    float pos = std::fmod(local_sample, period);

    return (pos < period * duty_ / 100.f) ? 1.f : -1.f;
}

float TriangleInstrument::GenerateVoiceSample(Voice& v, uint32_t sample) {
    uint32_t local_sample = sample - v.note.start_sample;

    float phase = v.note.note.frequency * local_sample / kFrequency;
    float frac = phase - std::floor(phase);

    return 4.f * std::abs(frac - .5f) - 1.f;
}

void Pattern::Expand(uint32_t bpm, std::vector<ScheduledNote>& out,
                     uint32_t offset,
                     std::vector<CompositionInstrument>& instruments,
                     std::vector<CompositionPattern>& patterns) {
    for (Event event : events_) {
        if (std::holds_alternative<InstrumentCall>(event.call)) {
            size_t instrument_idx =
                Find(std::get<InstrumentCall>(event.call).instrument_name,
                     instruments);

            if (instrument_idx >= instruments.size()) {
                continue;
            }

            uint32_t unit_end =
                event.unit_start +
                std::get<InstrumentCall>(event.call).note.duration;

            uint32_t start =
                offset + UnitsToSamples(event.unit_start, resolution_, bpm);
            uint32_t end = UnitsToSamples(unit_end, resolution_, bpm);

            out.emplace_back(start, end, instrument_idx,
                             std::get<InstrumentCall>(event.call).note);
        } else {
            size_t pattern_idx =
                Find(std::get<PatternCall>(event.call).pattern_name, patterns);

            if (pattern_idx >= patterns.size()) {
                continue;
            }
            uint32_t start_sample =
                UnitsToSamples(event.unit_start, resolution_, bpm);
            patterns[pattern_idx].second->Expand(
                bpm, out, offset + start_sample, instruments, patterns);
        }
    }
}

void Composition::PrepareData() {
    std::sort(patterns_.begin(), patterns_.end());
    std::sort(instruments_.begin(), instruments_.end());
    size_t start_idx = Find(kRootPattern, patterns_);
    patterns_[start_idx].second->Expand(bpm_, notes_, 0, instruments_,
                                        patterns_);
    std::sort(notes_.begin(), notes_.end());
}

std::vector<int16_t> Composition::CreateComposition() {
    PrepareData();
    std::vector<int16_t> data;
    uint32_t final_sample = 0;
    size_t notes_it = 0;
    for (uint32_t sample = 0; sample < final_sample || notes_it < notes_.size();
         ++sample) {
        while (notes_it < notes_.size() &&
               notes_[notes_it].start_sample <= sample) {
            ScheduledNote& note = notes_[notes_it];
            final_sample = std::max(final_sample, note.end_sample);
            instruments_[note.instrument_index].second->AddVoice(note);
            ++notes_it;
        }

        float out = 0.f;
        for (auto& [name, inst] : instruments_) {
            out += inst->ProcessSample(sample);
        }
        out = std::clamp(out, -1.f, 1.f);
        data.push_back(out * INT16_MAX);
    }
    return data;
}

}  // namespace itmoloops
