#pragma once

#include <string>

#include "envoy/server/http_filter_config.h"

#include "http_filter.pb.h"

namespace Envoy {
namespace Http {


/**
 * Configuration for the extauth filter.
 */
struct DosaConfig {
  Upstream::ClusterManager& cm_;
  std::string cluster_;
};

typedef std::shared_ptr<const DosaConfig> DosaConfigConstSharedPtr;

class HttpSampleDecoderFilter : public Logger::Loggable<Logger::Id::filter>,
                                public Http::StreamFilter {
public:
  HttpSampleDecoderFilter(HttpSampleDecoderFilterConfigSharedPtr, DosaEngine&);
  ~HttpSampleDecoderFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  Http::FilterHeadersStatus decodeHeaders(Http::RequestHeaderMap&, bool) override;
  Http::FilterDataStatus decodeData(Buffer::Instance&, bool) override;

  Http::FilterHeadersStatus encodeHeaders(Http::ResponseHeaderMap&, bool) override;
  Http::FilterDataStatus encodeData(Buffer::Instance&, bool) override;

  void setDecoderFilterCallbacks(StreamDecoderFilterCallbacks&) override;
private:
  const HttpSampleDecoderFilterConfigSharedPtr config_;
  DosaEngine& engine_;

};

} // namespace Http
} // namespace Envoy
