#pragma once
#include "common.hpp"
#include <cmath>
#include <map>
#include <pdh.h>
#include <pdhmsg.h>
#include <vector>
namespace monitoring::win {
// One instance belongs to one sampling thread. First/invalid samples stay N/A.
class Counter {
  PDH_HQUERY query_ = nullptr;
  PDH_HCOUNTER counter_ = nullptr;
  std::wstring path_;
  bool primed_ = false;

public:
  explicit Counter(const wchar_t *path) : path_(path) {}
  ~Counter() {
    if (query_)
      PdhCloseQuery(query_);
  }
  Counter(const Counter &) = delete;
  Counter &operator=(const Counter &) = delete;
  std::map<std::wstring, double> sample() {
    std::map<std::wstring, double> result;
    if (!query_) {
      if (PdhOpenQueryW(nullptr, 0, &query_) != ERROR_SUCCESS) {
        query_ = nullptr;
        return result;
      }
      if (PdhAddEnglishCounterW(query_, path_.c_str(), 0, &counter_) !=
          ERROR_SUCCESS) {
        PdhCloseQuery(query_);
        query_ = nullptr;
        return result;
      }
    }
    if (PdhCollectQueryData(query_) != ERROR_SUCCESS) {
      primed_ = false;
      return result;
    }
    if (!primed_) {
      primed_ = true;
      return result;
    }
    DWORD bytes = 0, count = 0;
    const DWORD flags = PDH_FMT_DOUBLE | PDH_FMT_NOCAP100;
    auto status =
        PdhGetFormattedCounterArrayW(counter_, flags, &bytes, &count, nullptr);
    if (status != PDH_MORE_DATA || bytes == 0 || bytes > 16 * 1024 * 1024)
      return result;
    std::vector<unsigned char> storage(bytes);
    auto values =
        reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W *>(storage.data());
    status =
        PdhGetFormattedCounterArrayW(counter_, flags, &bytes, &count, values);
    if (status != ERROR_SUCCESS)
      return result;
    for (DWORD i = 0; i < count; ++i) {
      const auto &value = values[i].FmtValue;
      if ((value.CStatus == PDH_CSTATUS_VALID_DATA ||
           value.CStatus == PDH_CSTATUS_NEW_DATA) &&
          std::isfinite(value.doubleValue) && value.doubleValue >= 0)
        result[values[i].szName] = value.doubleValue;
    }
    return result;
  }
};
} // namespace monitoring::win
