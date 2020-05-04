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
static LowerCaseString CL{"cluster"};
static LowerCaseString FidTimestamp{":fid_timestamp_unix_ns"};
static LowerCaseString FidTimestamp2{"fid_timestamp_unix_ns"};

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
    std::string id = url;

    // Check if the url is a special request
    if(id[2] == 'i'){
      // This is a ping request from envoy filter
      int pos = id.find("/", 2);
      id = id.substr(pos);

      ResponseHeaderMapPtr response_headers = createHeaderMap<ResponseHeaderMapImpl>(
        {{Http::Headers::get().Status, "200"}});
      
      response_headers->addCopy(FidTimestamp2, (std::to_string(long(engine_.get_timestamp_from_id(id).second))));
      response_headers->setCopy(FidTimestamp2, (std::to_string(long(engine_.get_timestamp_from_id(id).second))));

      // NOTE: hard code
      response_headers->addCopy(CL, "cluster_0");
      response_headers->setCopy(CL, "cluster_0");

      decoder_callbacks_->encodeHeaders(std::move(response_headers), true);
      return FilterHeadersStatus::StopIteration;
    } else {
      id_ = id;
    }

    // ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders pre map mapsize: " + engine_.print_map() + " {}", *decoder_callbacks_, engine_.get_map_size());
    // ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders val " + std::to_string(long(engine_.get_timestamp_from_id(id).second)) +": {}", *decoder_callbacks_, 21);
    
    // ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders new headers: {}", *decoder_callbacks_, headers);

    // if (true) {
    //   // test
    //   // if (headers.get(URLPath)->value() == "/service") {
    //   // std::string url = std::string(headers.get(URLPath)->value().getStringView());
      
    //   // ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders path: {}", *decoder_callbacks_, 20);
    //   // if (url == "/service") {
    //   //   ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders path: {}", *decoder_callbacks_, 25);
    //   // }

    //   // } else {
    //   //   ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders path: {}", *decoder_callbacks_, 30);

    //   // }
    //   // ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders1: {}", *decoder_callbacks_, headers.get(URLPath)->value());

    //   // test begin
    //   // get url.
    //   // std::string url = std::string(headers.EnvoyOriginalUrl()->value().getStringView());
    //   std::string url = std::string(headers.get(URLPath)->value().getStringView());

    //   // // Currently use url as id.
    //   std::string id = url;

    //   // ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders url: {}", *decoder_callbacks_, 26);

    //   // // get timestamp for the id.
    //   const auto p1 = std::chrono::system_clock::now();
    //   int64_t file_time_stamp = std::chrono::duration_cast<std::chrono::nanoseconds>(p1.time_since_epoch()).count();
    //   // std::cout << "file timestamp: " << filetime_stamp << '\n';
    //   // ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders timestamp: {}", *decoder_callbacks_, file_time_stamp);

    //   // // store id, timestamp for the file.
    //   this->engine_.set_id_with_timestamp(id, file_time_stamp);

    //   ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders map: {}", *decoder_callbacks_, long(engine_.get_timestamp_from_id(id).second));
    //   ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders mapsize: " + engine_.print_map() + " {}", *decoder_callbacks_, engine_.get_map_size());

    //   // test end
    // }
    
    return FilterHeadersStatus::Continue;

  } else if (headers.get(Method)->value() == "POST") {
    // ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders2_post: {}", *decoder_callbacks_, headers);
    
    // headers.setHost(config_->cluster_);

    // get url.
    std::string url = std::string(headers.get(URLPath)->value().getStringView());

    // Currently use url as id.
    std::string id = url;

    // get timestamp for the id.
    const auto p1 = std::chrono::system_clock::now();
    int64_t file_time_stamp = std::chrono::duration_cast<std::chrono::nanoseconds>(p1.time_since_epoch()).count();
    // std::cout << "file timestamp: " << filetime_stamp << '\n';
    // ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders3_post: {}", *decoder_callbacks_, file_time_stamp);

    // store id, timestamp for the file.
    this->engine_.set_id_with_timestamp(id, file_time_stamp);

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
      headers.addCopy(FidTimestamp2, (std::to_string(long(engine_.get_timestamp_from_id(id_).second))));
      headers.setCopy(FidTimestamp2, (std::to_string(long(engine_.get_timestamp_from_id(id_).second))));
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
