#include <string>

#include "http_filter.h"

#include "envoy/server/filter_config.h"

namespace Envoy {
namespace Http {

HttpSampleDecoderFilterConfig::HttpSampleDecoderFilterConfig(
    const sample::Decoder& proto_config)
    : key_(proto_config.key()), val_(proto_config.val()) {}

HttpSampleDecoderFilter::HttpSampleDecoderFilter(HttpSampleDecoderFilterConfigSharedPtr config)
    : config_(config) {}

HttpSampleDecoderFilter::~HttpSampleDecoderFilter() {}

void HttpSampleDecoderFilter::onDestroy() {}

FilterHeadersStatus HttpSampleDecoderFilter::encodeHeaders(RequestHeaderMap& headers, bool) {
  return FilterHeadersStatus::Continue;
}

FilterDataStatus HttpSampleDecoderFilter::encodeData(Buffer::Instance&, bool) {
  return FilterDataStatus::Continue;
}

FilterTrailersStatus HttpSampleDecoderFilter::encodeTrailers(RequestTrailerMap&) {
  return FilterTrailersStatus::Continue;
}

} // namespace Http
} // namespace Envoy
