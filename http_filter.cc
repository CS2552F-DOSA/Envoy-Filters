#include <string>

#include "http_filter.h"

#include "envoy/server/filter_config.h"

#include "absl/strings/str_join.h"

namespace Envoy {
namespace Http {

HttpSampleDecoderFilterConfig::HttpSampleDecoderFilterConfig(
    const dosa::Dosa&){}

HttpSampleDecoderFilter::HttpSampleDecoderFilter(
    HttpSampleDecoderFilterConfigSharedPtr config,
    DosaEngine& engine)
    : config_(config), engine_(engine){}

HttpSampleDecoderFilter::~HttpSampleDecoderFilter() {}

void HttpSampleDecoderFilter::onDestroy() {}

FilterHeadersStatus HttpSampleDecoderFilter::decodeHeaders(RequestHeaderMap& headers, bool) {
  if(false) {
    auto parts = StringUtil::splitToken(headers.Host()->value().getStringView(), ":");
    ASSERT(!parts.empty() && parts.size() <= 2);
    headers.setHost(parts.size() == 2
            ? absl::StrJoin(parts, "_test:")
            : absl::StrCat(headers.Host()->value().getStringView(), "_test"));
  }
  return FilterHeadersStatus::Continue;
}

FilterDataStatus HttpSampleDecoderFilter::decodeData(Buffer::Instance&, bool) {
  return FilterDataStatus::Continue;
}

FilterTrailersStatus HttpSampleDecoderFilter::decodeTrailers(ResponseHeaderMap&) {
  return FilterTrailersStatus::Continue;
}

void HttpSampleDecoderFilter::setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) {
  decoder_callbacks_ = &callbacks;
}

} // namespace Http
} // namespace Envoy
