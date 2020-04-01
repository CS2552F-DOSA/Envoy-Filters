#include <string>

#include "http_filter.h"

#include "common/common/assert.h"
#include "common/common/enum_to_int.h"
#include "common/http/message_impl.h"
#include "common/http/utility.h"
#include "common/common/utility.h"

namespace Envoy {
namespace Http {

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

FilterHeadersStatus HttpSampleDecoderFilter::decodeHeaders(HeaderMap& headers, bool) {
  log().info("The count is {}", engine_.getCount());

  if(copiedHeaders){
    copiedHeaders = Http::HeaderMapPtr{new Http::HeaderMapImpl(*headers)};
    decoder_callbacks_->continueDecoding();
  }

  if(decodeCacheCheck_){
    // The decodeData checked the cache
    if(decodeDoNotChange_){
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

Http::FilterDataStatus HttpSampleDecoderFilter::decodeData(Buffer::Instance&, bool) {
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
    if(copiedHeaders && copiedTrailers){
      // TODO: shadow the request to the test server
      Http::MessagePtr request(new Http::RequestMessageImpl(copiedHeaders));
      request->body().reset(new Buffer::OwnedImpl(*callbacks_->decodingBuffer()));
    } else {
      return FilterDataStatus::StopIterationAndBuffer;
    }
  }

  return FilterDataStatus::Continue;
}

Http::FilterTrailersStatus HttpSampleDecoderFilter::decodeTrailers(HeaderMap& trailers){
  if(copiedTrailers){
    copiedTrailers = Http::HeaderMapPtr{new Http::HeaderMapImpl(*trailers)};
    decoder_callbacks_->continueDecoding();
  }
  return FilterTrailersStatus::Continue;
}

Http::FilterHeadersStatus HttpSampleDecoderFilter::encodeHeaders(HeaderMap&, bool){
  log().info("The count is now {}", engine_.getCount());
  return FilterHeadersStatus::Continue;
}

Http::FilterDataStatus HttpSampleDecoderFilter::encodeData(Buffer::Instance&, bool){
  return FilterDataStatus::Continue;
}

Http::FilterTrailersStatus HttpSampleDecoderFilter::encodeTrailers(HeaderMap&){
  return FilterTrailersStatus::Continue;
}

void HttpSampleDecoderFilter::setDecoderFilterCallbacks(StreamDecoderFilterCallbacks& callbacks) {
  decoder_callbacks_ = &callbacks;
}

void HttpSampleDecoderFilter::setEncoderFilterCallbacks(StreamEncoderFilterCallbacks& callbacks) {
  encoder_callbacks_ = &callbacks;
}

void HttpSampleDecoderFilter::onSuccess(Http::MessagePtr&&){
  return;
}

void HttpSampleDecoderFilter::onFailure(Http::AsyncClient::FailureReason){
  return;
}

} // namespace Http
} // namespace Envoy
