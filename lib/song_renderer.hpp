#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

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

class Instrument {
   public:
    float ProcessSample(uint32_t sample);
    Instrument(float attack, float release)
        : attack_(attack), release_(release) {}

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
    std::vector<std::unique_ptr<Voice>> voices;

    virtual std::unique_ptr<Voice> CreateVoice(const ScheduledNote& note) {
        return std::make_unique<Voice>(note);
    }

    virtual float GenerateVoiceSample(Voice& v, uint32_t sample) = 0;
    float attack_;
    float release_;
};

class SamplerInstrument : public Instrument {
   public:
    SamplerInstrument(std::string sample_path, std::string root,
                      uint32_t loop_start, uint32_t loop_end, float attack,
                      float release);

   protected:
    struct SamplerVoice : Voice {
        uint32_t sample_pos = 0;
        explicit SamplerVoice(const ScheduledNote& note) : Voice(note) {}
    };

    std::unique_ptr<Voice> CreateVoice(const ScheduledNote& note) override {
        return std::make_unique<SamplerVoice>(note);
    }
    float GenerateVoiceSample(Voice& v, uint32_t sample) override;

   private:
    std::vector<float> sample_;
    float root_frequency_;
    uint32_t loop_start_ = 0;
    uint32_t loop_end_ = 0;
};

class SquareInstrument : public Instrument {
   public:
    SquareInstrument(uint8_t duty, float attack, float release)
        : Instrument(attack, release), duty_(duty) {}

   protected:
    float GenerateVoiceSample(Voice& v, uint32_t sample) override;

   private:
    uint8_t duty_;
};

class SineInstrument : public Instrument {
   public:
    SineInstrument(float attack, float release) : Instrument(attack, release) {}

   protected:
    float GenerateVoiceSample(Voice& v, uint32_t sample) override;
};

class TriangleInstrument : public Instrument {
   public:
    TriangleInstrument(float attack, float release)
        : Instrument(attack, release) {}

   protected:
    float GenerateVoiceSample(Voice& v, uint32_t sample) override;
};

class Pattern {
   public:
    void Expand(uint32_t bpm, std::vector<ScheduledNote>& out);

   private:
    struct InstrumentCall {
        Note note;
        std::string instrument;
    };

    struct PatternCall {
        std::string pattern;
    };

    struct Event {
        uint32_t unit_start;
        std::variant<InstrumentCall, PatternCall> call;
    };
    uint32_t resolution;
    std::vector<Event> events_;
};

class Composition {
   public:
    bool CreateComposition();

   private:
    uint32_t bpm;
    using CompositionPattern = std::pair<std::string, std::unique_ptr<Pattern>>;
    using CompositionInstrument =
        std::pair<std::string, std::unique_ptr<Instrument>>;
    std::vector<CompositionPattern> patterns;
    std::vector<CompositionInstrument> instruments;
    std::vector<ScheduledNote> notes;
    bool PrepareData();
};

}  // namespace itmoloops
