/**
 * Copyright 2021 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tools/converter/config_parser/config_file_parser.h"
#include "tools/common/parse_config_utils.h"
#include "include/errorcode.h"
#include "src/common/log_adapter.h"

namespace mindspore {
namespace lite {
int ConfigFileParser::ParseConfigFile(const std::string &config_file_path) {
  std::map<std::string, std::map<std::string, std::string>> maps;
  auto ret = mindspore::lite::ParseConfigFile(config_file_path, &maps);
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "image_path=input1:/mnt/calibration_input1_path";
    return ret;
  }
  ret = ParseDataPreProcessString(maps);
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "ParseDataPreProcessString failed.";
    return ret;
  }
  ret = ParseCommonQuantString(maps);
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "ParseCommonQuantString failed.";
    return ret;
  }
  ret = ParseMixedBitQuantString(maps);
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "ParseMixedBitQuantString failed.";
    return ret;
  }
  ret = ParseFullQuantString(maps);
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "ParseFullQuantString failed.";
    return ret;
  }
  return RET_OK;
}

int ConfigFileParser::SetMapData(const std::map<std::string, std::string> &input_map,
                                 const std::map<std::string, std::string &> &parse_map, const std::string &section) {
  for (const auto &map : input_map) {
    if (parse_map.find(map.first) == parse_map.end()) {
      MS_LOG(ERROR) << "INPUT ILLEGAL: `" << map.first << "` is not supported in "
                    << "[" << section << "]";
      return RET_INPUT_PARAM_INVALID;
    } else {
      parse_map.at(map.first) = map.second;
    }
  }
  return RET_OK;
}

int ConfigFileParser::ParseDataPreProcessString(const std::map<std::string, std::map<std::string, std::string>> &maps) {
  if (maps.find(DATA_PREPROCESS_PARAM) != maps.end()) {
    const auto &map = maps.at(DATA_PREPROCESS_PARAM);
    std::map<std::string, std::string &> parse_map{
      {"calibrate_path", data_pre_process_string_.calibrate_path},
      {"calibrate_size", data_pre_process_string_.calibrate_size},
      {"input_type", data_pre_process_string_.input_type},
      {"image_to_format", data_pre_process_string_.image_to_format},
      {"normalize_mean", data_pre_process_string_.normalize_mean},
      {"normalize_std", data_pre_process_string_.normalize_std},
      {"resize_width", data_pre_process_string_.resize_width},
      {"resize_height", data_pre_process_string_.resize_height},
      {"resize_method", data_pre_process_string_.resize_method},
      {"center_crop_width", data_pre_process_string_.center_crop_width},
      {"center_crop_height", data_pre_process_string_.center_crop_height},
    };
    return SetMapData(map, parse_map, DATA_PREPROCESS_PARAM);
  }
  return RET_OK;
}

int ConfigFileParser::ParseCommonQuantString(const std::map<std::string, std::map<std::string, std::string>> &maps) {
  if (maps.find(COMMON_QUANT_PARAM) != maps.end()) {
    const auto &map = maps.at(COMMON_QUANT_PARAM);
    std::map<std::string, std::string &> parse_map{
      {"quant_type", common_quant_string_.quant_type},
      {"bit_num", common_quant_string_.bit_num},
      {"min_quant_weight_size", common_quant_string_.min_quant_weight_size},
      {"min_quant_weight_channel", common_quant_string_.min_quant_weight_channel},
    };
    return SetMapData(map, parse_map, COMMON_QUANT_PARAM);
  }
  return RET_OK;
}

int ConfigFileParser::ParseMixedBitQuantString(const std::map<std::string, std::map<std::string, std::string>> &maps) {
  if (maps.find(MIXED_BIT_WEIGHT_QUANT_PARAM) != maps.end()) {
    const auto &map = maps.at(MIXED_BIT_WEIGHT_QUANT_PARAM);
    std::map<std::string, std::string &> parse_map{
      {"init_scale", mixed_bit_quant_string_.init_scale},
    };
    return SetMapData(map, parse_map, MIXED_BIT_WEIGHT_QUANT_PARAM);
  }
  return RET_OK;
}

int ConfigFileParser::ParseFullQuantString(const std::map<std::string, std::map<std::string, std::string>> &maps) {
  if (maps.find(FULL_QUANT_PARAM) != maps.end()) {
    const auto &map = maps.at(FULL_QUANT_PARAM);
    std::map<std::string, std::string &> parse_map{
      {"activation_quant_method", full_quant_string_.activation_quant_method},
      {"bias_correction", full_quant_string_.bias_correction},
    };
    return SetMapData(map, parse_map, FULL_QUANT_PARAM);
  }
  return RET_OK;
}
}  // namespace lite
}  // namespace mindspore