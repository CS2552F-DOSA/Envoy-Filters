#include <string>

#include "http_filter.h"

#include "envoy/server/filter_config.h"
#include "common/http/codes.h"
#include "common/http/header_map_impl.h"
#include "common/http/headers.h"
#include "common/http/message_impl.h"
#include "common/http/utility.h"

namespace Envoy {
namespace Http {

DosaConfig::DosaConfig(const dosa::Dosa& proto_config, Upstream::ClusterManager& cm):
  cm_(cm), cluster_(proto_config.cluster()) {}

bool DosaEngine::isKeyInCache(std::string key){
  return true;
  return cache_.find(key) != cache_.end();
}

DosaEngine HttpSampleDecoderFilter::engine_ = DosaEngine();

HttpSampleDecoderFilter::HttpSampleDecoderFilter(
    DosaConfigConstSharedPtr config)
    : config_(config){}

HttpSampleDecoderFilter::~HttpSampleDecoderFilter() {}

void HttpSampleDecoderFilter::onDestroy() {}

FilterHeadersStatus HttpSampleDecoderFilter::decodeHeaders(RequestHeaderMap& headers, bool) {
  ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders: {}", *decoder_callbacks_, headers);
  ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders: {}", *decoder_callbacks_, count_++);
  return FilterHeadersStatus::Continue;

  if(!copiedHeaders){
    copiedHeaders = &headers;
    decoder_callbacks_->continueDecoding();
  }

  if(decodeCacheCheck_){
    // The decodeData checked the cache
    if(decodeDoNotChange_){
      // We fork the data but do not change it
      return FilterHeadersStatus::Continue;
    } else {
      // // Change the header to test database
      // std::string host = headers->Host()->value().c_str();
      // ASSERT(!host.empty());
      // host += "_test";
      // headers->Host()->value(host);
      return FilterHeadersStatus::Continue;
    }
  } else {
    // We do not check the cache.
    return FilterHeadersStatus::StopIteration;
  }
}

FilterDataStatus HttpSampleDecoderFilter::decodeData(Buffer::Instance&, bool) {
  if(!decodeCacheCheck_){
    // TODO: Decode the data
    std::string key = "TODO:";

    // Note: The order is important
    // decodeDoNotChange_ = !engine_.isKeyInCache(key);
    decodeDoNotChange_ = true;
    decodeCacheCheck_ = true;

    decoder_callbacks_->continueDecoding();
  }

  if(decodeDoNotChange_){
    // We do not change the header of the request but we fork it
    if(copiedHeaders){
      // Shadow the request to the test server
      Http::RequestMessagePtr request(new Http::RequestMessageImpl(
        Http::createHeaderMap<Http::RequestHeaderMapImpl>(*copiedHeaders)));
      if(decoder_callbacks_->decodingBuffer()){
        request->body() = std::make_unique<Buffer::OwnedImpl>(*decoder_callbacks_->decodingBuffer());
      }

      auto parts = StringUtil::splitToken(request->headers().Host()->value().getStringView(), ":");
      ASSERT(!parts.empty() && parts.size() <= 2);
      request->headers().setHost(
          parts.size() == 2
              ? absl::StrJoin(parts, "_test:")
              : absl::StrCat(request->headers().Host()->value().getStringView(), "_test"));

      test_request_ = config_->cm_.httpAsyncClientForCluster(config_->cluster_)
          .send(std::move(request), *this, Http::AsyncClient::RequestOptions());
    } else {
      return FilterDataStatus::StopIterationAndBuffer;
    }
  }

  return FilterDataStatus::Continue;
}

FilterTrailersStatus HttpSampleDecoderFilter::decodeTrailers(RequestTrailerMap&){
  // if(!copiedTrailers){
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
  if(!decodeDoNotChange_)
    return FilterTrailersStatus::Continue;
  if(!hearhBackFromTest)
    return FilterTrailersStatus::StopIteration;
  // We have recevied from test and we might need to modify the package
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
