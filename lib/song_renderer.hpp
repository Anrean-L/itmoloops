#pragma once

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

namespace itmoloops {

struct Note {
    uint32_t duration;
    float frequency;
    float velocity;
};

float Envelope(uint32_t t, uint32_t note_len, uint32_t attack,
               uint32_t release);

struct ScheduledNote {
    uint32_t start_sample;
    uint32_t end_sample;
    size_t instrument_index;
    Note note;
};

class Effect {
   public:
    virtual ~Effect() = default;
    virtual float Process(float sound, uint32_t sample) = 0;
};

class GainEffect : public Effect {
   public:
    explicit GainEffect(float gain) : gain_(gain) {}

    float Process(float sound, uint32_t) override { return sound * gain_; }

   private:
    float gain_;
};

class TremoloEffect : public Effect {
   public:
    explicit TremoloEffect(float freq, float depth)
        : freq_(freq), depth_(depth) {}

    float Process(float sound, uint32_t sample) override {
        float t = sample / (float)kFrequency;
        float mod =
            1.f - depth_ + depth_ * std::sin(2 * std::numbers::pi * freq_ * t);
        return sound * mod;
    }

   private:
    float freq_;
    float depth_;
};

class EchoEffect : public Effect {
   public:
    EchoEffect(float delay, float decay)
        : buffer_(delay * kFrequency, 0.f), decay_(decay) {}

    float Process(float sound, uint32_t sample) override {
        if (buffer_.size() == 0) {
            return sound;
        }
        float y = sound + decay_ * buffer_[pos_];
        buffer_[pos_] = sound;
        pos_ = (pos_ + 1) % buffer_.size();
        return y;
    }

   private:
    std::vector<float> buffer_;
    size_t pos_ = 0;
    float decay_;
};

using VectorEffects = std::vector<std::unique_ptr<Effect>>;

class Instrument {
   public:
    float ProcessSample(uint32_t sample);

    Instrument(float attack, float release, VectorEffects effects)
        : attack_(attack * kFrequency),
          release_(release * kFrequency),
          effects_(std::move(effects)) {}

    Instrument(const Instrument&) = delete;
    Instrument& operator=(const Instrument&) = delete;
    virtual ~Instrument() = default;

    void AddVoice(const ScheduledNote& note);

   protected:
    struct Voice {
        ScheduledNote note;
        explicit Voice(const ScheduledNote& note) : note(note) {}
        virtual ~Voice() = default;
    };

    VectorEffects effects_;
    std::vector<std::unique_ptr<Voice>> voices_;
    uint32_t attack_;
    uint32_t release_;

    virtual std::unique_ptr<Voice> CreateVoice(const ScheduledNote& note) {
        return std::make_unique<Voice>(note);
    }

    virtual float GenerateVoiceSample(Voice& v, uint32_t sample) = 0;
};

class SamplerInstrument : public Instrument {
   public:
    SamplerInstrument(std::string sample_path, float root, uint32_t loop_start,
                      uint32_t loop_end, float attack, float release,
                      VectorEffects effects);

   protected:
    struct SamplerVoice : Voice {
        float sample_pos = 0;
        explicit SamplerVoice(const ScheduledNote& note) : Voice(note) {}
    };

    std::unique_ptr<Voice> CreateVoice(const ScheduledNote& note) override {
        return std::make_unique<SamplerVoice>(note);
    }
    float GenerateVoiceSample(Voice& v, uint32_t sample) override;

   private:
    std::vector<float> sample_;
    float root_frequency_;
    uint32_t loop_start_;
    uint32_t loop_end_;
};

class SquareInstrument : public Instrument {
   public:
    SquareInstrument(uint8_t duty, float attack, float release,
                     VectorEffects effects)
        : Instrument(attack, release, std::move(effects)), duty_(duty) {}

   protected:
    float GenerateVoiceSample(Voice& v, uint32_t sample) override;

   private:
    uint8_t duty_;
};

class SineInstrument : public Instrument {
   public:
    SineInstrument(float attack, float release, VectorEffects effects)
        : Instrument(attack, release, std::move(effects)) {}

   protected:
    float GenerateVoiceSample(Voice& v, uint32_t sample) override;
};

class TriangleInstrument : public Instrument {
   public:
    TriangleInstrument(float attack, float release, VectorEffects effects)
        : Instrument(attack, release, std::move(effects)) {}

   protected:
    float GenerateVoiceSample(Voice& v, uint32_t sample) override;
};

class Pattern {
   public:
    explicit Pattern(uint32_t resolution) : resolution_(resolution) {}
    using CompositionPattern = std::pair<std::string, std::unique_ptr<Pattern>>;
    using CompositionInstrument =
        std::pair<std::string, std::unique_ptr<Instrument>>;
    void Expand(uint32_t bpm, std::vector<ScheduledNote>& out, uint32_t offset,
                std::vector<CompositionInstrument>& instruments,
                std::vector<CompositionPattern>& patterns);

    void AddNote(uint32_t unit_start, Note note, std::string inst) {
        events_.emplace_back(unit_start, InstrumentCall{note, std::move(inst)});
    }

    void AddCall(uint32_t unit_start, std::string pattern_name) {
        events_.emplace_back(unit_start, PatternCall{std::move(pattern_name)});
    }

   private:
    struct InstrumentCall {
        Note note;
        std::string instrument_name;
    };

    struct PatternCall {
        std::string pattern_name;
    };

    struct Event {
        uint32_t unit_start;
        std::variant<InstrumentCall, PatternCall> call;
    };
    uint32_t resolution_;
    std::vector<Event> events_;
};

class Composition {
   public:
    void SetBpm(uint32_t new_bpm) { bpm_ = new_bpm; }

    void AddInstrument(std::string name, std::unique_ptr<Instrument> inst) {
        instruments_.emplace_back(std::move(name), std::move(inst));
    }

    void AddPattern(std::string name, std::unique_ptr<Pattern> pattern) {
        patterns_.emplace_back(std::move(name), std::move(pattern));
    }

    std::vector<int16_t> CreateComposition();

   private:
    uint32_t bpm_ = 60;
    using CompositionPattern = std::pair<std::string, std::unique_ptr<Pattern>>;
    using CompositionInstrument =
        std::pair<std::string, std::unique_ptr<Instrument>>;
    std::vector<CompositionPattern> patterns_;
    std::vector<CompositionInstrument> instruments_;
    std::vector<ScheduledNote> notes_;

    void PrepareData();
};

}  // namespace itmoloops
