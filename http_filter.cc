#include <string>

#include "http_filter.h"
#include <chrono>
#include <cstdint>
#include <mutex>
#include <condition_variable> // std::condition_variable

#include "envoy/server/filter_config.h"
#include "common/http/message_impl.h"
#include "common/common/enum_to_int.h"
#include "common/buffer/buffer_impl.h"

namespace Envoy {
namespace Http {

static LowerCaseString FidTimestamp{":fid_timestamp_unix_ns"};
static LowerCaseString FidTimestamp2{"fid_timestamp_unix_ns"};
static LowerCaseString Method{":method"};
static LowerCaseString URLPath{":path"};
static LowerCaseString Host{":authority"};
static LowerCaseString CL{"cluster"};

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
    filter_state_ = FilterState::GetDupSent;

    ENVOY_STREAM_LOG(info, "Dosa::decodeHeaders: {}", *decoder_callbacks_, count_);
    RequestMessagePtr request1(new RequestMessageImpl(
      createHeaderMap<RequestHeaderMapImpl>(headers)));
    url_ = std::string(headers.get(URLPath)->value().getStringView());
    request1->headers().setPath(std::string("/1/ping") + url_);
    request1->headers().setHost(config_->cluster_); // cluster name is Host: header value!
    test_request_ =
        config_->cm_.httpAsyncClientForCluster(config_->cluster_)
            .send(std::move(request1), *this,
            AsyncClient::RequestOptions().setTimeout(std::chrono::milliseconds(5000)));

    RequestMessagePtr request2(new RequestMessageImpl(
      createHeaderMap<RequestHeaderMapImpl>(headers)));
    request2->headers().setPath(std::string("/ping") + url_);
    request2->headers().setHost(config_->prod_cluster_);
    test_request_ =
        config_->cm_.httpAsyncClientForCluster(config_->prod_cluster_)
            .send(std::move(request2), *this,
            AsyncClient::RequestOptions().setTimeout(std::chrono::milliseconds(5000)));
    return FilterHeadersStatus::StopIteration;
    // return FilterHeadersStatus::StopAllIterationAndBuffer;
    
  } else if(headers.get(Method)->value() == "GET"
    && FilterState::GetDupSent == filter_state_){
    // TODO: add the fast cache return
    headers.setHost(cluster_);
    ENVOY_LOG(info, "databricks");
    // Modify the url
    if(cluster_ == config_->cluster_){
      ENVOY_LOG(info, "databricks");
      // The winner is test storage
      filter_state_ = FilterState::GetCompass;
      // NOTE: Hard code
      // std::string oldURL = std::string(headers.get(URLPath)->value().getStringView());
      headers.setPath(std::string("/1") + url_);
    }
    return FilterHeadersStatus::Continue;
    
  } else if(headers.get(Method)->value() == "POST"
    && FilterState::Null == filter_state_){
      ENVOY_LOG(info, "databricks");

    filter_state_ = FilterState::PostSent;
    filter_type_ = FilterType::Post;

    RequestMessagePtr request(new RequestMessageImpl(
      createHeaderMap<RequestHeaderMapImpl>(headers)));
    url_ = std::string(headers.get(URLPath)->value().getStringView());
    request->headers().setPath(std::string("/ping") + url_);
    request->headers().setHost(config_->prod_cluster_);
    test_request_ =
        config_->cm_.httpAsyncClientForCluster(config_->prod_cluster_)
            .send(std::move(request), *this,
            AsyncClient::RequestOptions().setTimeout(std::chrono::milliseconds(5000)));

    return FilterHeadersStatus::StopAllIterationAndBuffer;
  } else if (headers.get(Method)->value() == "POST"
    && (FilterState::PostSent == filter_state_ || FilterState::PostCacheUpdate == filter_state_)){
ENVOY_LOG(info, "databricks");
    // Compare the recv timestamp with cache
    auto _tmp = engine_.get_timestamp_from_id(url_);
    if(!_tmp.first || _tmp.second < std::stol(test_reponse_time_)){
      // update the cache
      engine_.set_id_with_cache(url_, test_reponse_time_);

      filter_state_ = FilterState::PostCacheUpdate;

      RequestMessagePtr request(new RequestMessageImpl(
        createHeaderMap<RequestHeaderMapImpl>(headers)));
      request->headers().setPath(url_);
      request->headers().setHost(std::string(headers.get(Host)->value().getStringView()));
      test_request_ =
          config_->cm_.httpAsyncClientForCluster(std::string(headers.get(Host)->value().getStringView()))
              .send(std::move(request), *this,
              AsyncClient::RequestOptions().setTimeout(std::chrono::milliseconds(5000)));
      
      return FilterHeadersStatus::StopAllIterationAndBuffer;
    }
    // use time stamp information and Send the write
    headers.addCopy(FidTimestamp2, test_reponse_time_);
    headers.setCopy(FidTimestamp2, test_reponse_time_);
    // NOTE: Hard code
    // std::string oldURL = std::string(headers.get(URLPath)->value().getStringView());
    headers.setPath(std::string("/1") + url_);

    return FilterHeadersStatus::Continue;
  }
  ENVOY_LOG(info, "databricks");
  return FilterHeadersStatus::Continue;
}

FilterDataStatus HttpSampleDecoderFilter::decodeData(Buffer::Instance& data, bool) {
  if(FilterState::PostSent == filter_state_ 
    && filter_type_ == FilterType::Post){
    ENVOY_LOG(info, "databricks");
    // TODO: compass
    std::string whole = data.toString();
    auto _tmp = engine_.get_cache_from_id(url_);
    std::string delta = _tmp.second + whole;
    data.drain(data.length());
    data.add(delta);
  }
  return FilterDataStatus::Continue;
}

FilterTrailersStatus HttpSampleDecoderFilter::decodeTrailers(RequestTrailerMap&){
  ENVOY_LOG(info, "databricks");
  return FilterTrailersStatus::Continue;
}

Http::FilterHeadersStatus HttpSampleDecoderFilter::encodeHeaders(ResponseHeaderMap&, bool){
  return FilterHeadersStatus::Continue;
}

Http::FilterDataStatus HttpSampleDecoderFilter::encodeData(Buffer::Instance& data, bool){
  ENVOY_LOG(info, "The encode data called");
  if(filter_state_ == FilterState::GetCompass){

    // Fetch the id from request, get cache
    auto _tmp = engine_.get_cache_from_id(url_);
    std::string delta = data.toString();
    // TODO: compass the files
    std::string whole = delta + _tmp.second;
    // data = new Buffer::OwnedImpl(delta);
    data.drain(data.length());
    data.add(whole);
  } else if(filter_state_ == FilterState::GetDupSent){

    engine_.set_id_with_cache(url_, data.toString());
  }

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
    && filter_state_ == FilterState::GetDupSent){
      if(test_reponse_time_ == ""){
        ENVOY_LOG(info, "databricks");
        test_reponse_time_ = std::string(response->headers().get(FidTimestamp2)->value().getStringView());
        cluster_ = std::string(response->headers().get(CL)->value().getStringView());
      } else {
        // compare the time
        std::string new_reponse_time = std::string(response->headers().get(FidTimestamp2)->value().getStringView());
        if(std::stol(test_reponse_time_) < std::stol(new_reponse_time)){
          ENVOY_LOG(info, "databricks");
          // cluster_ = std::string(request->cluster_->name());
          test_reponse_time_ = new_reponse_time;
          cluster_ = std::string(response->headers().get(CL)->value().getStringView());
        } else if(std::stol(test_reponse_time_) == std::stol(new_reponse_time)){
          ENVOY_LOG(info, "databricks");
          cluster_ = "cluster_1";
        }
        decoder_callbacks_->continueDecoding();
      }
  } else if(filter_type_ == FilterType::Post
    && filter_state_ == FilterState::PostSent){

    test_reponse_time_ = std::string(response->headers().get(FidTimestamp2)->value().getStringView());
    decoder_callbacks_->continueDecoding();
  } else if(filter_type_ == FilterType::Post
    && filter_state_ == FilterState::PostCacheUpdate){

    test_reponse_time_ = std::string(response->headers().get(FidTimestamp2)->value().getStringView());
    engine_.set_id_with_cache(url_, response->bodyAsString());
    decoder_callbacks_->continueDecoding();
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
