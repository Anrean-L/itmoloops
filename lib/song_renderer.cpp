#include "song_renderer.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <string>
#include <utility>
#include <vector>

#include "config.hpp"
#include "wav_file.hpp"

namespace itmoloops {

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
        if (v.note.end_sample + release_ < sample) {
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
        sample_.resize(raw_sample.size());
        for (size_t i = 0; i < raw_sample.size(); ++i) {
            sample_[i] = (float)raw_sample[i] / INT16_MAX;
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

}  // namespace itmoloops
