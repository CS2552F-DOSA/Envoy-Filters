#pragma once

#include <string>

#include "envoy/server/filter_config.h"

#include "http_filter.pb.h"

namespace Envoy {
namespace Http {

class HttpSampleDecoderFilterConfig {
public:
  HttpSampleDecoderFilterConfig(const dosa::Dosa& proto_config);

private:
};

typedef std::shared_ptr<HttpSampleDecoderFilterConfig> HttpSampleDecoderFilterConfigSharedPtr;

class HttpSampleDecoderFilter : public StreamDecoderFilter {
public:
  HttpSampleDecoderFilter(HttpSampleDecoderFilterConfigSharedPtr);
  ~HttpSampleDecoderFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  FilterHeadersStatus decodeHeaders(RequestHeaderMap&, bool) override;
  FilterDataStatus decodeData(Buffer::Instance&, bool) override;
  FilterTrailersStatus decodeTrailers(RequestTrailerMap&) override;
  void setDecoderFilterCallbacks(StreamDecoderFilterCallbacks&) override;


private:
  const HttpSampleDecoderFilterConfigSharedPtr config_;
  StreamDecoderFilterCallbacks* decoder_callbacks_;

  // const LowerCaseString headerKey() const;
  // const std::string headerValue() const;
};

} // namespace Http
} // namespace Envoy
