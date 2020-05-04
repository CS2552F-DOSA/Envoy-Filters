#include <string>

#include "http_filter.h"

#include "chrono"
#include "envoy/server/filter_config.h"
#include "common/http/header_map_impl.h"

#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "absl/strings/str_split.h"


namespace Envoy {
namespace Http {

static LowerCaseString Method{":method"};
static LowerCaseString URLPath{":path"};
static LowerCaseString FidTimestamp{":fid_timestamp_unix_ns"};
static LowerCaseString FidTimestamp2{"fid_timestamp_unix_ns"};
static LowerCaseString CL{"cluster"};

DosaConfig::DosaConfig(const dosa::Dosa& proto_config, Upstream::ClusterManager& cm):
  cm_(cm), cluster_(proto_config.cluster()) {}

DosaEngine HttpSampleDecoderFilter::engine_ = DosaEngine();

HttpSampleDecoderFilter::HttpSampleDecoderFilter(
    DosaConfigConstSharedPtr config)
    : config_(config){}

HttpSampleDecoderFilter::~HttpSampleDecoderFilter() {}

void HttpSampleDecoderFilter::onDestroy() {}

FilterHeadersStatus HttpSampleDecoderFilter::decodeHeaders(RequestHeaderMap& headers, bool) {
  // headers.EnvoyOriginalPath()->value()
  // ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders1: {}", *decoder_callbacks_, headers);

  if (headers.get(Method)->value() == "GET") {
    std::string url = std::string(headers.get(URLPath)->value().getStringView());
    id_ = url;
    if(id_[4] == 'i'){
      // This is a ping request from envoy filter
      int pos = id_.find("/", 3);
      id_ = "/1" + id_.substr(pos);

      ResponseHeaderMapPtr response_headers = createHeaderMap<ResponseHeaderMapImpl>(
        {{Http::Headers::get().Status, "200"}});
      
      response_headers->addCopy(FidTimestamp2, engine_.get_timestamp_from_id(id_).second);
      response_headers->setCopy(FidTimestamp2, engine_.get_timestamp_from_id(id_).second);

      response_headers->addCopy(CL, "cluster_1");
      response_headers->setCopy(CL, "cluster_1");

      decoder_callbacks_->encodeHeaders(std::move(response_headers), true);
      return FilterHeadersStatus::StopIteration;
    }
    return FilterHeadersStatus::Continue;
  } else if (headers.get(Method)->value() == "POST") {
    ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders1: {}", *decoder_callbacks_, headers);
    // ENVOY_LOG(info, "The decodeHeaders returned");
    std::string timestamp = std::string(headers.get(FidTimestamp2)->value().getStringView());
    // ENVOY_LOG(info, "The decodeHeaders returned");
    std::string url = std::string(headers.get(URLPath)->value().getStringView());
    // ENVOY_LOG(info, "The decodeHeaders returned");
    // store id, timestamp for the file.
    // ENVOY_LOG(info, "The decodeHeaders returned");
    this->engine_.set_id_with_timestamp(url, timestamp);
    // ENVOY_LOG(info, "The decodeHeaders returned");
    // ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders1: {}", *decoder_callbacks_, headers);
    return FilterHeadersStatus::Continue;
  }

  return FilterHeadersStatus::Continue;
}

FilterDataStatus HttpSampleDecoderFilter::decodeData(Buffer::Instance&, bool) {
  return FilterDataStatus::Continue;
}

FilterTrailersStatus HttpSampleDecoderFilter::decodeTrailers(RequestTrailerMap&){
  return FilterTrailersStatus::Continue;
}

Http::FilterHeadersStatus HttpSampleDecoderFilter::encodeHeaders(ResponseHeaderMap& headers, bool){
  if(id_ != ""){
      headers.addCopy(FidTimestamp2, engine_.get_timestamp_from_id(id_).second);
      headers.setCopy(FidTimestamp2, engine_.get_timestamp_from_id(id_).second);
  }
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
