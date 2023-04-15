#include <iostream>

#include "param.h"

struct DelayPatch
{
    params::CollectedRange range;
    params::Param sampleDelay, feedback;
    void setup(params::Collector &collector, const std::string &stableNamePrefix)
    {
        auto g = params::CaptureCollectedRangeGuard(collector, range);
        sampleDelay.collectTo(collector)
            .withStableName(stableNamePrefix + "_sampleDelay")
            .withRange(1.f, 16000.f, 2000.f);
        feedback.collectTo(collector)
            .withStableName(stableNamePrefix + "_feedback")
            .withRange(0.f, 1.f, 0.2f);
    }
};
struct OscPatch
{
    params::CollectedRange range;

    params::Param sqrSawMix, pulseWidth;
    void setup(params::Collector &collector, const std::string &stableNamePrefix)
    {
        auto g = params::CaptureCollectedRangeGuard(collector, range);
        sqrSawMix.collectTo(collector)
            .withStableName(stableNamePrefix + "_sqrSawMix")
            .withRange(0.f, 1.f, 0.f);
        pulseWidth.collectTo(collector)
            .withStableName(stableNamePrefix + "_pulseWidth")
            .withRange(0.f, 1.f, 0.5f);
    }
};

struct FilteredAREnvPatch
{
    params::Collector collector;
    params::CollectedRange range;
    params::CollectedRange globalRange, voiceRange;

    // "Global" (or not-per-voice) items
    params::Param mainVolume;
    DelayPatch delay;

    // "Per Voice" items
    OscPatch osc[2];
    params::Param cutoff, resonance, a, r, filterMode;

    void setup()
    {
        auto g = params::CaptureCollectedRangeGuard(collector, range);

        {
            auto q = params::CaptureCollectedRangeGuard(collector, globalRange);
            mainVolume.collectTo(collector).withStableName("main_volume").withRange(0.f, 1.f, 0.8f);
            delay.setup(collector, "delay");
        }

        auto vg = params::CaptureCollectedRangeGuard(collector, voiceRange);
        osc[0].setup(collector, "osc0");
        osc[1].setup(collector, "osc1");
        cutoff.collectTo(collector).withStableName("filter_cutoff").withRange(-60.f, 70.f, 0.f);
        resonance.collectTo(collector).withStableName("filter_resonance").withRange(0.f, 1.f, 0.7f);
        a.collectTo(collector).withStableName("ar_attack").withRange(-5.f, 3.f, -2.f);
        r.collectTo(collector).withStableName("ar_release").withRange(-5.f, 3.f, 1.f);
        filterMode.collectTo(collector).withStableName("filter_mode").withRange(0, 3, 0);
    }
};


int main(int argc, char **argv)
{
    std::cout << "Yo" << std::endl;

    FilteredAREnvPatch ss;
    ss.setup();
    std::cout << "Filtered patch has " << ss.collector.paramWeakPtrs.size() << " params"
              << std::endl;

    // Two ways to address. We can do this
    for (int i = 0; i < 2; ++i)
        std::cout << ss.osc[i].sqrSawMix << std::endl;
    std::cout << ss.cutoff << std::endl;

    // Or we can do this
    std::cout << "All params" << std::endl;
    for (const auto *wr : ss.collector.paramWeakPtrs)
    {
        std::cout << *wr << std::endl;
    }

    // Or we can do this
    std::cout << "All Params in Range" << std::endl;
    for (const auto &rti : ss.range)
    {
        std::cout << *ss.collector[rti] << " " << (ss.globalRange.contains(rti) ? "global" : "")
                  << " " << (ss.voiceRange.contains(rti) ? "voice" : "") << std::endl;
    }

    // Or we can do this
    std::cout << "OSC1 params" << std::endl;
    std::cout << ss.osc[1].range << std::endl;
    for (auto p = ss.osc[1].range.from; p < ss.osc[1].range.to; ++p)
    {
        std::cout << *(ss.collector.paramWeakPtrs[p]) << std::endl;
    }

    std::cout << "Sub-osc range onto pdata" << std::endl;
    std::array<params::Param::pdata_t, 2> oscData;
    assert(ss.collector.extractOnto(oscData, ss.osc[1].range));
    for (const auto &f : oscData)
    {
        std::cout << "  - " << std::get<float>(f) << std::endl;
    }

    std::cout << "Sub-osc range onto float *" << std::endl;
    std::array<float *, 2> oscFP;
    assert(ss.collector.extractOnto(oscFP, ss.osc[1].range));
    for (const auto &f : oscFP)
    {
        std::cout << "  - " << (f ? std::to_string(*f) : "nullptr") << std::endl;
    }

    std::cout << "Specified range onto float *" << std::endl;
    assert(ss.collector.extractOnto(oscFP, {4, 6}));
    for (const auto &f : oscFP)
    {
        std::cout << "  - " << (f ? std::to_string(*f) : "nullptr") << std::endl;
    }

    return 0;
}
