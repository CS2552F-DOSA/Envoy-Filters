#pragma once

#include <string>
#include <unordered_set>

#include "envoy/http/filter.h"
#include "envoy/upstream/cluster_manager.h"
#include "common/common/logger.h"

namespace Envoy {
namespace Http {

class DosaEngine {
public:
  DosaEngine(){};
  ~DosaEngine(){};

  int getCount() {
    return count_ ++;
  }

  bool isKeyInCache(std::string);
private:
  int count_ = 0;
  std::unordered_set<std::string> cache_;
};

/**
 * Configuration for the extauth filter.
 */
struct DosaConfig {
  Upstream::ClusterManager& cm_;
  std::string cluster_;
};

typedef std::shared_ptr<const DosaConfig> DosaConfigConstSharedPtr;

class HttpSampleDecoderFilter : Logger::Loggable<Logger::Id::filter>,
                                public Http::StreamFilter,
                                public Http::AsyncClient::Callbacks{
public:
  HttpSampleDecoderFilter(DosaConfigConstSharedPtr);
  ~HttpSampleDecoderFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  Http::FilterHeadersStatus decodeHeaders(HeaderMap&, bool) override;
  Http::FilterDataStatus decodeData(Buffer::Instance&, bool) override;
  Http::FilterTrailersStatus decodeTrailers(HeaderMap&) override;

  Http::FilterHeadersStatus encodeHeaders(HeaderMap&, bool) override;
  Http::FilterDataStatus encodeData(Buffer::Instance&, bool) override;
  Http::FilterTrailersStatus encodeTrailers(HeaderMap& ) override;

  void setDecoderFilterCallbacks(StreamDecoderFilterCallbacks&) override;
  void setEncoderFilterCallbacks(StreamEncoderFilterCallbacks&) override;

  // Http::AsyncClient::Callbacks
  void onSuccess(Http::MessagePtr&&) override;
  void onFailure(Http::AsyncClient::FailureReason) override;

private:
  const DosaConfigConstSharedPtr config_;
  static DosaEngine engine_;
  bool decodeCacheCheck_ = false;
  bool decodeDoNotChange_ = true;

  StreamDecoderFilterCallbacks* decoder_callbacks_{};
  StreamEncoderFilterCallbacks* encoder_callbacks_{};

  Http::AsyncClient::Request* test_request_{};
};

} // namespace Http
} // namespace Envoy
