//
// Created by Paul Walker on 4/15/23.
//

#ifndef PARAM_PROTO_PARAM_H
#define PARAM_PROTO_PARAM_H

#include <string>
#include <variant>
#include <cstdint>
#include <memory>
#include <functional>
#include <ostream>
#include <vector>
#include <cassert>
#include <array>

namespace params
{
struct Collector;

struct Param {
    typedef std::variant<int32_t, float, bool> pdata_t;
    pdata_t val{0.f}, val_max{1.f}, val_min{0.f}, val_def{0.f};
    std::string stableName;
    std::size_t idHash;

    typedef int64_t rti_t;
    rti_t runtimeIndex{-1};


    friend std::ostream &operator<<(std::ostream &os, const Param &z)
    {
        os << "param[sn=" << z.stableName << ",rti=" << z.runtimeIndex << ",vt=" << z.val.index() << "]";
        return os;
    }

    Param() {
        formatter = nullptr; // TODO set this up to a default formatter
    }

    Param &collectTo(Collector &coll);

    Param &withStableName(const std::string &withStableName) {
        stableName = withStableName;
        idHash = std::hash<std::string>()(stableName);
        return *this;
    }

    struct ExtensionSupport
    {
        virtual ~ExtensionSupport() = default;
        virtual float extendedValue(float f) const = 0;
    };
    struct Formatter
    {
        virtual ~Formatter() = default;
        virtual std::string valueToString(const Param &, pdata_t value) const = 0;
        virtual pdata_t stringToValue(const Param &, const std::string &s) const = 0;
    };

    Param &withRange(const pdata_t &min, const pdata_t &max, const pdata_t &def)
    {
        val_max = max;
        val_min = min;
        val_def = def;
        val = val_def;
        assert(val_def >= val_min);
        assert(val_def <= val_max);
        assert(val_def.index() == val_min.index());
        assert(val_def.index() == val_max.index());
        assert(val_min <= val_max);
        return *this;
    }
    Param &withExtensionSupport(const std::shared_ptr<ExtensionSupport> &e)
    {
        extender = e;
        return *this;
    }
    Param &withFormatter(const std::shared_ptr<Formatter> &f)
    {
        formatter = f;
        return *this;
    }
    Param &withTemposync() {
        return *this;
    }
    Param &withAbsolute() {
        return *this;
    }
    Param &withDeform(int numDeformTypes) {
        return *this;
    }

    float getValue01() const;
    void setValue01(float f);

    std::shared_ptr<ExtensionSupport> extender;
    bool canExtend() const { return extender != nullptr; }
    bool isExtended() const;
    bool canTemposync() const;
    bool isTemposynced() const;
    bool canAbsolute() const;
    bool isAbsoluted() const;
    bool canDeform() const;
    int numDeformTypes() const;
    int deformType() const;


    std::shared_ptr<Formatter> formatter;

};

struct CollectedRange
{
    Param::rti_t from{-1}, to{-1};

    friend std::ostream &operator<<(std::ostream &os, const CollectedRange &z)
    {
        os << "range[from=" << z.from << ",to=" << z.to << "]";
        return os;
    }
};

struct Collector
{
    Param::rti_t nextRTI{0};
    std::vector<Param *> paramWeakPtrs;

    template <size_t N>
    bool extractOnto(std::array<Param::pdata_t, N> &arr, const CollectedRange &r)
    {
        auto pts = r.to - r.from;
        assert(pts <= N);
        if (pts > N)
            return false;
        for (auto i=0; i<pts; ++i)
        {
            arr[i] = paramWeakPtrs[i + r.from]->val;
        }
        return true;
    }
};

struct CaptureCollectedRangeGuard
{
    Collector &collector;
    CollectedRange &range;
    CaptureCollectedRangeGuard(Collector &c, CollectedRange &r) :
    collector(c), range(r)
    {
        range.from = collector.nextRTI;
    }
    ~CaptureCollectedRangeGuard()
    {
        range.to = collector.nextRTI;
    }
};

inline Param &Param::collectTo(Collector &coll) {
    runtimeIndex = coll.nextRTI;
    coll.nextRTI++;
    coll.paramWeakPtrs.push_back(this);
    return *this;
}
}

#endif // PARAM_PROTO_PARAM_H
