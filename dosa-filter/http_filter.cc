#include <string>

#include "http_filter.h"

#include "envoy/server/filter_config.h"

namespace Envoy {
namespace Http {

HttpSampleDecoderFilterConfig::HttpSampleDecoderFilterConfig(
    const sample::Decoder&){}

HttpSampleDecoderFilter::HttpSampleDecoderFilter(HttpSampleDecoderFilterConfigSharedPtr config)
    : config_(config) {}

HttpSampleDecoderFilter::~HttpSampleDecoderFilter() {}

void HttpSampleDecoderFilter::onDestroy() {}

FilterHeadersStatus HttpSampleDecoderFilter::decodeHeaders(RequestHeaderMap&, bool) {
  // headers.setHost();
  return FilterHeadersStatus::Continue;
}

FilterDataStatus HttpSampleDecoderFilter::decodeData(Buffer::Instance&, bool) {
  return FilterDataStatus::Continue;
}

FilterTrailersStatus HttpSampleDecoderFilter::decodeTrailers(RequestTrailerMap&) {
  return FilterTrailersStatus::Continue;
}

void HttpSampleDecoderFilter::setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) {
  decoder_callbacks_ = &callbacks;
}

} // namespace Http
} // namespace Envoy
