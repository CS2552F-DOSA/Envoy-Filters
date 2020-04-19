#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>

#include "envoy/server/filter_config.h"

#include "http_filter.pb.h"

namespace Envoy {
namespace Http {

class DosaEngine {
public:
  DosaEngine(){};
  ~DosaEngine(){};

  int getCount() {
    return count_ ++;
  }
  void set_id_with_timestamp(const std::string& id, const int64_t timestamp) {
    this->id_to_timestamp_[id] = timestamp;
  }

  pair<bool, int64_t> get_timestamp_from_id(const std::string& id) {
    if (id_to_timestamp_.count(id)) {
      return std::make_pair(true, this->id_to_timestamp_[id]);
    } else {
      return std::make_pair(false, 0);
    }
  }
private:
  int count_ = 0;
  std::unordered_map<std::string, int64_t> id_to_timestamp_;
};

/**
 * Configuration for the extauth filter.
 */
struct DosaConfig {
public:
  DosaConfig(const dosa::Dosa& proto_config, Upstream::ClusterManager& cm);
  Upstream::ClusterManager& cm_;
  std::string cluster_;
};

typedef std::shared_ptr<const DosaConfig> DosaConfigConstSharedPtr;

class HttpSampleDecoderFilter : Logger::Loggable<Logger::Id::filter>,
                                public StreamFilter,
                                public AsyncClient::Callbacks{
public:
  HttpSampleDecoderFilter(DosaConfigConstSharedPtr);
  ~HttpSampleDecoderFilter();

  // StreamFilterBase
  void onDestroy() override;

  // StreamDecoderFilter
  FilterHeadersStatus decodeHeaders(RequestHeaderMap&, bool) override;
  FilterDataStatus decodeData(Buffer::Instance&, bool) override;
  FilterTrailersStatus decodeTrailers(RequestTrailerMap&) override;

  FilterHeadersStatus encodeHeaders(ResponseHeaderMap&, bool) override;
  FilterDataStatus encodeData(Buffer::Instance&, bool) override;
  FilterTrailersStatus encodeTrailers(ResponseTrailerMap& ) override;

  FilterHeadersStatus encode100ContinueHeaders(ResponseHeaderMap&) override;
  FilterMetadataStatus encodeMetadata(MetadataMap&) override;

  void setDecoderFilterCallbacks(StreamDecoderFilterCallbacks&) override;
  void setEncoderFilterCallbacks(StreamEncoderFilterCallbacks&) override;

  // Http::AsyncClient::Callbacks
  void onSuccess(const AsyncClient::Request&, ResponseMessagePtr&&) override;
  void onFailure(const AsyncClient::Request&, AsyncClient::FailureReason) override;

private:
  const DosaConfigConstSharedPtr config_;
  static DosaEngine engine_;
  int count_ = 0;
  // bool decodeCacheCheck_ = false;
  // bool decodeDoNotChange_ = true;

  // HeaderMap* copiedHeaders;
  // HeaderMap* copiedTrailers;

  StreamDecoderFilterCallbacks* decoder_callbacks_{};
  StreamEncoderFilterCallbacks* encoder_callbacks_{};

  // Http::AsyncClient::Request* test_request_{};
};

} // namespace Http
} // namespace Envoy
