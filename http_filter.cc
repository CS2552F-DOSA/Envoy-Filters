#include <string>

#include "http_filter.h"
#include <chrono>
#include <cstdint>
#include <mutex>
#include <condition_variable> // std::condition_variable

#include "envoy/server/filter_config.h"
#include "common/http/message_impl.h"

namespace Envoy {
namespace Http {

static LowerCaseString Method{":method"};

DosaConfig::DosaConfig(const dosa::Dosa& proto_config, Upstream::ClusterManager& cm):
  cm_(cm), cluster_(proto_config.cluster()) {}

DosaEngine HttpSampleDecoderFilter::engine_ = DosaEngine();

HttpSampleDecoderFilter::HttpSampleDecoderFilter(
    DosaConfigConstSharedPtr config)
    : config_(config), filter_state_(FilterState::Null){}

HttpSampleDecoderFilter::~HttpSampleDecoderFilter() {}

void HttpSampleDecoderFilter::onDestroy() {}

FilterHeadersStatus HttpSampleDecoderFilter::decodeHeaders(RequestHeaderMap& headers, bool) {
  ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders: {}", *decoder_callbacks_, headers);
  if(headers.get(Method)->value() == "GET"){
    ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders: {}", *decoder_callbacks_, count_);
    RequestMessagePtr request(new RequestMessageImpl(
      createHeaderMap<RequestHeaderMapImpl>(headers)));
    // request->headers().insertMethod().value(Http::Headers::get().MethodValues.Post);
    // request->headers().insertPath().value(std::string("/ambassador/auth"));
    request->headers().setHost(config_->cluster_); // cluster name is Host: header value!
    // request->headers().insertContentType().value(std::string("application/json"));
    // request->headers().insertContentLength().value(request_body.size());
    // request->body() = Buffer::InstancePtr(new Buffer::OwnedImpl(request_body));
    test_request_ =
        config_->cm_.httpAsyncClientForCluster(config_->cluster_)
            .send(std::move(request), *this,
            AsyncClient::RequestOptions().setTimeout(std::chrono::milliseconds(5000)));
    filter_state_ = FilterState::GetDupSent;
    return FilterHeadersStatus::Continue;
  } else if()(

  )
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
  ENVOY_LOG(info, "The encode data called");
  std::unique_lock<std::mutex> lck(mtx_);
  while (filter_state_ != FilterState::GetDupRecv) cv_.wait(lck);
  lck.unlock();

  // TODO: Compare the timestamp and modify the package.
  ENVOY_LOG(info, "The encode data returned");
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

void HttpSampleDecoderFilter::onSuccess(const AsyncClient::Request&, ResponseMessagePtr&& response){
  ENVOY_LOG(info, "onSuccess was invoked");
  test_request_ = nullptr;
  std::unique_lock<std::mutex> lck(mtx_);
  if(filter_state_ == FilterState::GetDupWait){
    filter_state_ = FilterState::GetDupRecv;
    cv_.notify_all();
  }
  filter_state_ = FilterState::GetDupRecv;
  lck.unlock();

  // TODO: store the package
  test_response_body_ = response->bodyAsString();
  if (Http::Utility::getResponseStatus(response->headers()) != enumToInt(Http::Code::OK)) {
    ENVOY_LOG(info, "The dup request recvs a not 200");
  } else {
    ENVOY_LOG(info, "The dup request recvs a 200");
  }

  ENVOY_LOG(info, "onSuccess returned");
  return;
}

void HttpSampleDecoderFilter::onFailure(const AsyncClient::Request&, AsyncClient::FailureReason){
  ENVOY_LOG(info, "onFailure was invoked");
  test_request_ = nullptr;
  std::unique_lock<std::mutex> lck(mtx_);
  if(filter_state_ == FilterState::GetDupWait){
    cv_.notify_all();
  }
  filter_state_ = FilterState::GetDupRecv;
  return;
}

} // namespace Http
} // namespace Envoy
