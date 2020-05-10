#pragma once

#include <string>
#include <unordered_set>
#include <mutex>
#include <condition_variable> // std::condition_variable

#include "envoy/server/filter_config.h"

#include "http_filter.pb.h"

namespace Envoy {
namespace Http {

enum FilterType { Get, Post };
enum FilterState { Null, GetDupSent, PostSent, GetCompass, PostCacheUpdate };

class DosaEngine {
public:
  DosaEngine(){};
  ~DosaEngine(){};

  // Set file id and its "prod envoy" timestamp.
  void set_id_with_timestamp(std::string& id, const int64_t timestamp) {
    this->id_to_timestamp_[id] = timestamp;
  }

  void set_id_with_cache(std::string& id, const std::string file) {
    this->id_to_cache_[id] = file;
  }

  // Return the pair for the id. If the id is in this->id_to_timestamp_, then return the <true, timestamp>, if not return <false, 0>.
  std::pair<bool, int64_t> get_timestamp_from_id(std::string& id) {
    if (this->id_to_timestamp_.find(id) != this->id_to_timestamp_.end()) {
      return std::make_pair(true, this->id_to_timestamp_[id]);
    } else {
      return std::make_pair(false, 0);
    }
  }

  // Return the pair for the id. If the id is in this->id_to_timestamp_, then return the <true, timestamp>, if not return <false, 0>.
  std::pair<bool, std::string> get_cache_from_id(std::string& id) {
    if (this->id_to_cache_.find(id) != this->id_to_cache_.end()) {
      return std::make_pair(true, this->id_to_cache_[id]);
    } else {
      return std::make_pair(false, "");
    }
  }

private:
  // Map for maintaining id -> timestamp.
  std::unordered_map<std::string, int64_t> id_to_timestamp_;
  std::unordered_map<std::string, std::string> id_to_cache_;
};

/**
 * Configuration for the extauth filter.
 */
struct DosaConfig {
public:
  DosaConfig(const dosa::Dosa& proto_config, Upstream::ClusterManager& cm);
  Upstream::ClusterManager& cm_;
  std::string cluster_;
  std::string test_cluster_;
  std::string prod_cluster_;
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

  // Filter state Set
  FilterState filter_state_;
  FilterType filter_type_;

  StreamDecoderFilterCallbacks* decoder_callbacks_{};
  StreamEncoderFilterCallbacks* encoder_callbacks_{};

  Http::AsyncClient::Request* test_request_{};
  std::string url_;
  std::string request_body_;
  std::string test_response_body_;
  std::string test_reponse_time_;
  std::string cluster_;
  bool flag_ = false;
};

} // namespace Http
} // namespace Envoy
