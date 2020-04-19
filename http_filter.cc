#include <string>

#include "http_filter.h"

#include "chrono"
#include "envoy/server/filter_config.h"

// #include "absl/strings/str_join.h"
// #include "absl/strings/string_view.h"
// #include "absl/strings/str_split.h"

namespace Envoy {
namespace Http {

static LowerCaseString Method{":method"};

DosaConfig::DosaConfig(const dosa::Dosa& proto_config, Upstream::ClusterManager& cm):
  cm_(cm), cluster_(proto_config.cluster()) {}

DosaEngine HttpSampleDecoderFilter::engine_ = DosaEngine();

HttpSampleDecoderFilter::HttpSampleDecoderFilter(
    DosaConfigConstSharedPtr config)
    : config_(config){}

HttpSampleDecoderFilter::~HttpSampleDecoderFilter() {}

void HttpSampleDecoderFilter::onDestroy() {}

FilterHeadersStatus HttpSampleDecoderFilter::decodeHeaders(RequestHeaderMap& headers, bool) {
  ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders: {}", *decoder_callbacks_, headers);
  ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders: {}", *decoder_callbacks_, count_++);

  

  if (headers.get(Method)->value() == "GET") {

    return FilterHeadersStatus::Continue;

  } else if (headers.get(Method)->value() == "POST") {
    headers.setHost(config_->cluster_);

    // get url.
    std::string url = std::string(headers.EnvoyOriginalUrl()->value().getStringView());

    // Currently use url as id.
    std::string id = url;

    // get timestamp for the id.
    const auto p1 = std::chrono::system_clock::now();
    int64_t file_time_stamp = std::chrono::duration_cast<std::chrono::nanoseconds>(p1.time_since_epoch()).count();
    // std::cout << "file timestamp: " << filetime_stamp << '\n';
    ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders: {}", *decoder_callbacks_, file_time_stamp);

    // store id, timestamp for the file.
    this->engine_.set_id_with_timestamp(id, file_time_stamp);

    return FilterHeadersStatus::Continue;
  }

  return FilterHeadersStatus::Continue;

  // if(copiedHeaders){
  //   copiedHeaders = Http::HeaderMapPtr{new Http::HeaderMapImpl(*headers)};
  //   decoder_callbacks_->continueDecoding();
  // }

  // if(decodeCacheCheck_){
  //   // The decodeData checked the cache
  //   if(decodeDoNotChange_){
  //     return FilterHeadersStatus::Continue;
  //   } else {
  //     // // Change the header to test database
  //     // std::string host = headers->Host()->value().c_str();
  //     // ASSERT(!host.empty());
  //     // host += "_test";
  //     // headers->Host()->value(host);
  //     return FilterHeadersStatus::Continue;
  //   }
  // } else {
  //   // We do not check the cache.
  //   return FilterHeadersStatus::StopIteration;
  // }
}

FilterDataStatus HttpSampleDecoderFilter::decodeData(Buffer::Instance&, bool) {
  // if(!decodeCacheCheck_){
  //   // TODO: Decode the data
  //   std::string key = "TODO:";s

  //   // Note: The order is important
  //   // decodeDoNotChange_ = !engine_.isKeyInCache(key);
  //   decodeDoNotChange_ = true;
  //   decodeCacheCheck_ = true;

  //   decoder_callbacks_->continueDecoding();
  // }

  // if(decodeDoNotChange_){
  //   if(copiedHeaders && copiedTrailers){
  //     // TODO: shadow the request to the test server
  //     Http::MessagePtr request(new Http::RequestMessageImpl(copiedHeaders));
  //     request->body().reset(new Buffer::OwnedImpl(*callbacks_->decodingBuffer()));
  //   } else {
  //     return FilterDataStatus::StopIterationAndBuffer;
  //   }
  // }

  return FilterDataStatus::Continue;
}

FilterTrailersStatus HttpSampleDecoderFilter::decodeTrailers(RequestTrailerMap&){
  // if(copiedTrailers){
  //   copiedTrailers = Http::HeaderMapPtr{new Http::HeaderMapImpl(*trailers)};
  //   decoder_callbacks_->continueDecoding();
  // }
  return FilterTrailersStatus::Continue;
}

Http::FilterHeadersStatus HttpSampleDecoderFilter::encodeHeaders(ResponseHeaderMap& headers, bool){
  ENVOY_STREAM_LOG(info, "Dosa::encodeHeaders: {}", *encoder_callbacks_, headers);
  ENVOY_STREAM_LOG(info, "Dosa::encodeHeaders: {}", *encoder_callbacks_, count_++);
  return FilterHeadersStatus::Continue;
}

Http::FilterDataStatus HttpSampleDecoderFilter::encodeData(Buffer::Instance&, bool){
  return FilterDataStatus::Continue;
}

Http::FilterTrailersStatus HttpSampleDecoderFilter::encodeTrailers(ResponseTrailerMap&){
  return FilterTrailersStatus::Continue;
}

FilterHeadersStatus HttpSampleDecoderFilter::encode100ContinueHeaders(ResponseHeaderMap&){
  return FilterHeadersStatus::Continue;
}

FilterMetadataStatus HttpSampleDecoderFilter::encodeMetadata(MetadataMap&){
  return FilterMetadataStatus::Continue;
}

void HttpSampleDecoderFilter::setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) {
  decoder_callbacks_ = &callbacks;
}

void HttpSampleDecoderFilter::setEncoderFilterCallbacks(StreamEncoderFilterCallbacks& callbacks) {
  encoder_callbacks_ = &callbacks;
}

void HttpSampleDecoderFilter::onSuccess(const AsyncClient::Request&, ResponseMessagePtr&&){
  return;
}

void HttpSampleDecoderFilter::onFailure(const AsyncClient::Request&, AsyncClient::FailureReason){
  return;
}

} // namespace Http
} // namespace Envoy
