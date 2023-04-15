#include <iostream>

#include "param.h"

struct OscPatch
{
    params::CollectedRange range;

    params::Param sqrSawMix, pulseWidth;
    void setup(params::Collector &collector, const std::string stableNamePrefix) {
        auto g = params::CaptureCollectedRangeGuard(collector, range);
        sqrSawMix.collectTo(collector).withStableName(stableNamePrefix + "_sqrSawMix")
            .withRange(0.f, 1.f, 0.f);
        pulseWidth.collectTo(collector).withStableName(stableNamePrefix + "_pulseWidth").withRange(0.f,1.f,0.5f);
    }
};

struct FilteredAREnvPatch
{
    params::Collector collector;
    params::CollectedRange range;

    OscPatch osc[2];
    params::Param cutoff, resonance, a, r, filterMode;

    void setup()
    {
        auto g = params::CaptureCollectedRangeGuard(collector, range);
        osc[0].setup(collector, "osc0");
        osc[1].setup(collector, "osc1");
        cutoff.collectTo(collector).withStableName("filter_cutoff")
                .withRange(-60.f, 70.f, 0.f);
        resonance.collectTo(collector).withStableName("filter_resonance")
                     .withRange(0.f, 1.f, 0.7f);
        a.collectTo(collector).withStableName("ar_attack")
                        .withRange(-5.f, 3.f, -2.f);
        r.collectTo(collector).withStableName("ar_release")
                .withRange(-5.f, 3.f, 1.f);
        filterMode.collectTo(collector).withStableName("filter_mode")
                .withRange(0, 3, 0);
    }
};

struct DelayedPatch
{
    params::Collector collector;

    OscPatch osc;
    params::Param feedback, delay;

    void setup()
    {
        osc.setup(collector, "generator");
    }
};

int main(int argc, char **argv)
{
   std::cout << "Yo" << std::endl;

   FilteredAREnvPatch ss;
   ss.setup();
   std::cout << "Filtered patch has " << ss.collector.paramWeakPtrs.size() << " params" << std::endl;

   // Two ways to address. We can do this
   for (int i=0; i<2; ++i)
        std::cout << ss.osc[i].sqrSawMix << std::endl;
   std::cout << ss.cutoff << std::endl;

   // Or we can do this
   std::cout << "All params" << std::endl;
   for (const auto *wr : ss.collector.paramWeakPtrs)
   {
        std::cout << *wr << std::endl;
   }

   // Or we can do this
   std::cout << "OSC1 params" << std::endl;
   std::cout << ss.osc[1].range << std::endl;
   for (auto p=ss.osc[1].range.from; p < ss.osc[1].range.to; ++p)
   {
        std::cout << *(ss.collector.paramWeakPtrs[p]) << std::endl;
   }

   std::array<params::Param::pdata_t, 2> oscData;
   assert(ss.collector.extractOnto(oscData, ss.osc[1].range));
   for (const auto &f : oscData)
   {
        std::cout << "  - " << std::get<float>(f) << std::endl;
   }

   return 0;
}
