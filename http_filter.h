#pragma once

#include <string>
#include <unordered_set>

#include "envoy/server/filter_config.h"
#include "envoy/network/connection.h"
#include "envoy/network/filter.h"

#include "extensions/filters/network/common/redis/codec.h"

#include "http_filter.pb.h"

namespace Envoy {
namespace Filter {

namespace Redis = Extensions::NetworkFilters::Common::Redis;

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
public:
  DosaConfig(const dosa::Dosa& proto_config, Upstream::ClusterManager& cm);
  Upstream::ClusterManager& cm_;
  std::string cluster_;
};

typedef std::shared_ptr<const DosaConfig> DosaConfigConstSharedPtr;

class HttpSampleDecoderFilter : Logger::Loggable<Logger::Id::filter>,
                                public Network::Filter,
                                public Redis::DecoderCallbacks{
                                //public Network::ConnectionCallbacks{
public:
  HttpSampleDecoderFilter(DosaConfigConstSharedPtr, Redis::DecoderFactory&);
  ~HttpSampleDecoderFilter();

  // HTTP filter legacy
  // // StreamFilterBase
  // void onDestroy() override;

  // // StreamDecoderFilter
  // FilterHeadersStatus decodeHeaders(RequestHeaderMap&, bool) override;
  // FilterDataStatus decodeData(Buffer::Instance&, bool) override;
  // FilterTrailersStatus decodeTrailers(RequestTrailerMap&) override;

  // FilterHeadersStatus encodeHeaders(ResponseHeaderMap&, bool) override;
  // FilterDataStatus encodeData(Buffer::Instance&, bool) override;
  // FilterTrailersStatus encodeTrailers(ResponseTrailerMap& ) override;

  // FilterHeadersStatus encode100ContinueHeaders(ResponseHeaderMap&) override;
  // FilterMetadataStatus encodeMetadata(MetadataMap&) override;

  // void setDecoderFilterCallbacks(StreamDecoderFilterCallbacks&) override;
  // void setEncoderFilterCallbacks(StreamEncoderFilterCallbacks&) override;

  // // Http::AsyncClient::Callbacks
  // void onSuccess(const AsyncClient::Request&, ResponseMessagePtr&&) override;
  // void onFailure(const AsyncClient::Request&, AsyncClient::FailureReason) override;

  Network::FilterStatus onWrite(Buffer::Instance&, bool) override;
  Network::FilterStatus onData(Buffer::Instance&, bool) override;
  Network::FilterStatus onNewConnection() override;
  void initializeReadFilterCallbacks(Network::ReadFilterCallbacks&) override;
  // void initializeWriteFilterCallbacks(WriteFilterCallbacks&) override;

  // // Network::ConnectionCallbacks
  // void onEvent(Network::ConnectionEvent event) override;
  // void onAboveWriteBufferHighWatermark() override {}
  // void onBelowWriteBufferLowWatermark() override {}

  // Common::Redis::DecoderCallbacks
  void onRespValue(Redis::RespValuePtr&& value) override;

private:
  const DosaConfigConstSharedPtr config_;
  static DosaEngine engine_;
  int count_ = 0;

  Redis::DecoderPtr decoder_;
  // bool decodeCacheCheck_ = false;
  // bool decodeDoNotChange_ = true;

  Network::ReadFilterCallbacks* read_callbacks_{};

  // HTTP legacy
  // HeaderMap* copiedHeaders;
  // HeaderMap* copiedTrailers;

  // StreamDecoderFilterCallbacks* decoder_callbacks_{};
  // StreamEncoderFilterCallbacks* encoder_callbacks_{};

  // Http::AsyncClient::Request* test_request_{};
};

} // namespace Filter
} // namespace Envoy
