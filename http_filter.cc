#include <string>

#include "http_filter.h"

#include "common/common/assert.h"
#include "common/common/enum_to_int.h"
#include "common/http/message_impl.h"
#include "common/http/utility.h"
#include "common/common/utility.h"

namespace Envoy {
namespace Http {

DosaEngine::isKeyInCache(std::string key){
  return true;
  return cache_.find(key) != cache_.end();
}

DosaEngine HttpSampleDecoderFilter::engine_ = DosaEngine();

HttpSampleDecoderFilter::HttpSampleDecoderFilter(
    DosaConfigConstSharedPtr config)
    : config_(config){}

HttpSampleDecoderFilter::~HttpSampleDecoderFilter() {}

void HttpSampleDecoderFilter::onDestroy() {}

FilterHeadersStatus HttpSampleDecoderFilter::decodeHeaders(HeaderMap&, bool) {
  log().info("The count is {}", engine_.getCount());
  // if(false) {
  //   // auto parts = StringUtil::split(headers.Host()->value().getStringView(), ":");
  //   // ASSERT(!parts.empty() && parts.size() <= 2);
  //   // headers.setHost(parts.size() == 2
  //   //         ? absl::StrJoin(parts, "_test:")
  //   //         : absl::StrCat(headers.Host()->value().getStringView(), "_test"));
  // }
  // return FilterHeadersStatus::Continue;
  if(decodeCacheCheck_){
    // The decodeData checked the cache
    if(decodeDoNotChange_){
      return FilterHeadersStatus::Continue;
    } else {
      // TODO: change the header to test database
      return FilterHeadersStatus::Continue;
    }
  } else {
    // We do not check the cache.
    return FilterHeadersStatus::StopIteration;
  }
}

Http::FilterDataStatus HttpSampleDecoderFilter::decodeData(Buffer::Instance&, bool) {
  // TODO: Decode the data

  // Note: The order is important
  decodeDoNotChange_ = !engine_.isKeyInCache("TODO:");
  decodeCacheCheck_ = true;

  decoder_callbacks_.continueDecoding();
  
  return FilterDataStatus::Continue;
}

Http::FilterTrailersStatus HttpSampleDecoderFilter::decodeTrailers(HeaderMap&){
  return FilterTrailersStatus::Continue;
}

Http::FilterHeadersStatus HttpSampleDecoderFilter::encodeHeaders(HeaderMap&, bool){
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

} // namespace Http
} // namespace Envoy
