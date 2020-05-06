#include <string>

#include "http_filter.h"
#include <chrono>
#include <cstdint>
#include <mutex>
#include <condition_variable> // std::condition_variable

#include "envoy/server/filter_config.h"
#include "common/http/message_impl.h"
#include "common/common/enum_to_int.h"

namespace Envoy {
namespace Http {

static LowerCaseString FidTimestamp{":fid_timestamp_unix_ns"};
static LowerCaseString FidTimestamp2{"fid_timestamp_unix_ns"};
static LowerCaseString Method{":method"};
static LowerCaseString URLPath{":path"};
static LowerCaseString CL{"cluster"};

// DosaConfig::DosaConfig(const dosa::Dosa& proto_config, Upstream::ClusterManager& cm):
//   cm_(cm), cluster_(proto_config.test_cluster()), cluster_(proto_config.prod_cluster()) {}

DosaConfig::DosaConfig(const dosa::Dosa& proto_config, Upstream::ClusterManager& cm):
  cm_(cm), cluster_(proto_config.cluster()), prod_cluster_(proto_config.prod_cluster()) {}

DosaEngine HttpSampleDecoderFilter::engine_ = DosaEngine();

HttpSampleDecoderFilter::HttpSampleDecoderFilter(
    DosaConfigConstSharedPtr config)
    : config_(config), filter_state_(FilterState::Null){}

HttpSampleDecoderFilter::~HttpSampleDecoderFilter() {}

void HttpSampleDecoderFilter::onDestroy() {}

FilterHeadersStatus HttpSampleDecoderFilter::decodeHeaders(RequestHeaderMap& headers, bool) {
  ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders: {}", *decoder_callbacks_, headers);

  if(headers.get(Method)->value() == "GET"
    && FilterState::Null == filter_state_){
    // We have a init get request, will send two ping requests
    filter_type_ = FilterType::Get;
    // filter_state_ = FilterState::GetDupSent;

    ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders: {}", *decoder_callbacks_, count_);
    RequestMessagePtr request1(new RequestMessageImpl(
      createHeaderMap<RequestHeaderMapImpl>(headers)));
    url_ = std::string(headers.get(URLPath)->value().getStringView());
    request1->headers().setPath(std::string("/1/ping") + url_);
    request1->headers().setHost(config_->cluster_);
    test_request_ =
        config_->cm_.httpAsyncClientForCluster(config_->cluster_)
            .send(std::move(request1), *this,
            AsyncClient::RequestOptions().setTimeout(std::chrono::milliseconds(5000)));

    RequestMessagePtr request2(new RequestMessageImpl(
      createHeaderMap<RequestHeaderMapImpl>(headers)));
    request2->headers().setPath(std::string("/ping") + url_);
    request2->headers().setHost(config_->prod_cluster_);
    // request2->headers().setHost(std::string(headers.get(Host)->value().getStringView())); // cluster name is Host: header value!
    test_request_ =
        config_->cm_.httpAsyncClientForCluster(config_->prod_cluster_)
            .send(std::move(request2), *this,
            AsyncClient::RequestOptions().setTimeout(std::chrono::milliseconds(5000)));

    return FilterHeadersStatus::StopIteration;
    
  } else if(headers.get(Method)->value() == "POST"
    && FilterState::Null == filter_state_){

    // filter_state_ = FilterState::PostSent;
    filter_type_ = FilterType::Post;

    RequestMessagePtr request(new RequestMessageImpl(
      createHeaderMap<RequestHeaderMapImpl>(headers)));
    url_ = std::string(headers.get(URLPath)->value().getStringView());
    request->headers().setMethod(Http::Headers::get().MethodValues.Get);
    request->headers().setPath(std::string("/ping") + url_);
    request->headers().setHost(config_->prod_cluster_);
    test_request_ =
        config_->cm_.httpAsyncClientForCluster(config_->prod_cluster_)
            .send(std::move(request), *this,
            AsyncClient::RequestOptions().setTimeout(std::chrono::milliseconds(5000)));

    // // use time stamp information and Send the write
    // headers.addCopy(FidTimestamp2, test_reponse_time_);
    // headers.setCopy(FidTimestamp2, test_reponse_time_);
    // // NOTE: Hard code
    // // std::string oldURL = std::string(headers.get(URLPath)->value().getStringView());
    // headers.setPath(std::string("/1") + oldURL);

    return FilterHeadersStatus::Continue;
  }

  return FilterHeadersStatus::StopIteration;
}

FilterDataStatus HttpSampleDecoderFilter::decodeData(Buffer::Instance& data, bool) {
  if(filter_type_ == FilterType::Post
    && FilterState::Null == filter_state_){
      request_body_ = data.toString();
      return FilterDataStatus::StopIterationNoBuffer;
    }
  return FilterDataStatus::Continue;
}

FilterTrailersStatus HttpSampleDecoderFilter::decodeTrailers(RequestTrailerMap&){
  return FilterTrailersStatus::Continue;
}

Http::FilterHeadersStatus HttpSampleDecoderFilter::encodeHeaders(ResponseHeaderMap& headers, bool){
  ENVOY_STREAM_LOG(info, "Dosa::encodeHeaders: {}", *encoder_callbacks_, headers);
  return FilterHeadersStatus::Continue;
}

Http::FilterDataStatus HttpSampleDecoderFilter::encodeData(Buffer::Instance&, bool){
  ENVOY_LOG(info, "The encode data called");
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
  if(filter_type_ == FilterType::Get
    && filter_state_ == FilterState::Null){

    if(test_reponse_time_ == ""){
      test_reponse_time_ = std::string(response->headers().get(FidTimestamp2)->value().getStringView());
      cluster_ = std::string(response->headers().get(CL)->value().getStringView());
    } else {
      // compare the time
      std::string new_reponse_time = std::string(response->headers().get(FidTimestamp2)->value().getStringView());
      if(std::stol(test_reponse_time_) < std::stol(new_reponse_time)){
        // cluster_ = std::string(request->cluster_->name());
        cluster_ = std::string(response->headers().get(CL)->value().getStringView());
      } else if(std::stol(test_reponse_time_) < std::stol(new_reponse_time)){
        cluster_ = config_->cluster_;
      }
      filter_state_ = FilterState::GetDupSent;

      RequestMessagePtr message(new RequestMessageImpl());
      if(cluster_ == config_->cluster_)
        message->headers().setPath(std::string("/1") + url_);
      else
        message->headers().setPath(url_);
      message->headers().setHost(cluster_);
      message->headers().setMethod(Headers::get().MethodValues.Get);
      test_request_ =
          config_->cm_.httpAsyncClientForCluster(cluster_)
              .send(std::move(message), *this,
              AsyncClient::RequestOptions().setTimeout(std::chrono::milliseconds(5000)));
    }
  } else if(filter_type_ == FilterType::Post
      && filter_state_ == FilterState::Null){

      ENVOY_STREAM_LOG(info, "Dosa::encodeHeaders: {}", *encoder_callbacks_, response->headers());
      
      test_reponse_time_ = std::string(response->headers().get(FidTimestamp2)->value().getStringView());
      filter_state_ = FilterState::PostSent;

      RequestMessagePtr message(new RequestMessageImpl());
      message->headers().setPath(std::string("/1") + url_);
      message->headers().setHost(config_->cluster_);
      message->headers().setMethod(Headers::get().MethodValues.Post);
      message->headers().addCopy(FidTimestamp2, test_reponse_time_);
      message->headers().setCopy(FidTimestamp2, test_reponse_time_);
      test_request_ =
          config_->cm_.httpAsyncClientForCluster(config_->cluster_)
              .send(std::move(message), *this,
              AsyncClient::RequestOptions().setTimeout(std::chrono::milliseconds(5000)));

  } else if(filter_state_ == FilterState::GetDupSent){
    std::string response_body(response->bodyAsString());
    // Http::HeaderMapPtr response_headers{new HeaderMapImpl(response->headers())};
    ResponseHeaderMapPtr headers(Http::createHeaderMap<Http::ResponseHeaderMapImpl>(response->headers()));
    decoder_callbacks_->encodeHeaders(std::move(headers), response_body.empty());
    if (!response_body.empty()) {
      Buffer::OwnedImpl buffer(response_body);
      decoder_callbacks_->encodeData(buffer, true);
    }
    return;
  } else if(filter_state_ == FilterState::PostSent){
    ENVOY_LOG(info, "databricks");

    std::string response_body(response->bodyAsString());
    // Http::HeaderMapPtr response_headers{new HeaderMapImpl(response->headers())};
    ResponseHeaderMapPtr headers(Http::createHeaderMap<Http::ResponseHeaderMapImpl>(response->headers()));
    decoder_callbacks_->encodeHeaders(std::move(headers), response_body.empty());
    if (!response_body.empty()) {
      Buffer::OwnedImpl buffer(response_body);
      decoder_callbacks_->encodeData(buffer, true);
    }
    return;

    // // use time stamp information and Send the write
    // headers.addCopy(FidTimestamp2, test_reponse_time_);
    // headers.setCopy(FidTimestamp2, test_reponse_time_);
    // // NOTE: Hard code
    // // std::string oldURL = std::string(headers.get(URLPath)->value().getStringView());
    // headers.setPath(std::string("/1") + oldURL);
  }

  ENVOY_LOG(info, "onSuccess returned");
  return;
}

void HttpSampleDecoderFilter::onFailure(const AsyncClient::Request&, AsyncClient::FailureReason){
  ENVOY_LOG(info, "onFailure was invoked");
  return;
}

} // namespace Http
} // namespace Envoy
