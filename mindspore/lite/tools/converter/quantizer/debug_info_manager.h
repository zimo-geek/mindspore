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

#ifndef MINDSPORE_LITE_TOOLS_CONVERTER_DEBUGINFO_H
#define MINDSPORE_LITE_TOOLS_CONVERTER_DEBUGINFO_H

#include <string>
#include <vector>
#include <cstdio>
#include <map>
#include "nnacl/op_base.h"
#include "tools/common/statistic_utils.h"
#include "src/tensor.h"

namespace mindspore::lite {
enum InOutFlag { INPUT, OUTPUT };
enum DataTypeFlag { ORIGIN, DEQUANT };
enum TensorTypeFlag { ACTIVATION, WEIGHT };

struct PrimaryKey {
  std::string node_name;
  InOutFlag in_out_flag;
  size_t index;
  friend bool operator<(const struct PrimaryKey &p1, const struct PrimaryKey &p2) {
    return p1.node_name < p2.node_name || (p1.node_name == p2.node_name && p1.in_out_flag < p2.in_out_flag) ||
           (p1.node_name == p2.node_name && p1.in_out_flag == p2.in_out_flag && p1.index < p2.index);
  }
  friend std::ostream &operator<<(std::ostream &os, const PrimaryKey &p) {  // for struct output
    os << "[" << p.node_name << "," << p.in_out_flag << "," << p.index << "]";
    return os;
  }
};

struct TensorData {
  void *data;
  TypeId data_type;
  size_t size;
  size_t elements_num;
};

struct QuantDebugInfo {
  PrimaryKey primary_key;
  std::string node_type;
  std::string tensor_name;
  DataTypeFlag data_type_flag;
  TensorTypeFlag tensor_type_flag;
  float min{0};
  float quartile1{0};
  float median{0};
  float quartile3{0};
  float max{0};
  float mean{0};
  float var{0};
  float sparsity{0};
  float clip{0};
  float cos_similarity{1};
  TensorData tensor_data;
};

class DebugInfoManager {
 public:
  int AddOriginInfo(const mindspore::CallBackParam &call_back_param, OpParameter *op_parameter, bool is_input,
                    int tensor_index, mindspore::lite::Tensor *origin_tensor);

  int AddComparedInfo(const mindspore::CallBackParam &call_back_param, OpParameter *op_parameter, bool is_input,
                      int tensor_index, mindspore::lite::Tensor *compared_tensor);

  int CompareOriginWithDequant(const quant::SessionModel &origin, const quant::SessionModel &dequant,
                               const preprocess::DataPreProcessParam &data_preprocess,
                               const std::string &debug_info_save_path,
                               const std::map<std::string, OpParameter *> &op_parameters);

 private:
  void PrintInfo();

  int SaveInfo(const std::string &file_path);

  int SetOriginStaticInfo(QuantDebugInfo *quant_debug_info, const mindspore::lite::Tensor &tensor);

  int SetDequantStaticInfo(OpParameter *op_parameter, int tensor_index, QuantDebugInfo *quant_debug_info,
                           const mindspore::lite::Tensor &tensor);

  std::string ParseDataTypeFlagToString(DataTypeFlag data_type_flag);

  std::string ParseTensorTypeFlagToString(TensorTypeFlag tensor_type_flag);

  std::string ParseInOutTensorToString(InOutFlag in_out_flag);

  void PrintInfo(const QuantDebugInfo &info);

  void SaveInfo(std::ofstream &out_file, const QuantDebugInfo &info);

  std::map<std::string, mindspore::schema::Tensor *> ParseInputTensorFromModel(const Model &model);

  std::map<std::string, mindspore::schema::Tensor *> ParseOutputTensorFromModel(const Model &model);

  int GetDataFromTensorMap(const std::map<std::string, mindspore::schema::Tensor *> &maps,
                           const std::string &tensor_name, mindspore::lite::Tensor *dst_tensor);

  KernelCallBack GetBeforeCallBack(const std::map<std::string, mindspore::schema::Tensor *> &input_tensor_map,
                                   const std::map<std::string, OpParameter *> &op_parameters, bool is_origin);

  KernelCallBack GetAfterCallBack(const std::map<std::string, OpParameter *> &op_parameters, bool is_origin);

  void FreeBuffer();

  template <typename T>
  void GetStatByTensor(const std::vector<T> &tensor_data, QuantDebugInfo *infos) {
    MS_ASSERT(infos != nullptr);
    infos->min = mindposre::lite::Quantile(tensor_data, 0.0f);
    infos->quartile1 = mindposre::lite::Quantile(tensor_data, 0.25f);
    infos->median = mindposre::lite::Quantile(tensor_data, 0.5f);
    infos->quartile3 = mindposre::lite::Quantile(tensor_data, 0.75f);
    infos->max = mindposre::lite::Quantile(tensor_data, 1.0f);
    infos->mean = mindposre::lite::GetMeanValue(tensor_data);
    infos->var = mindposre::lite::GetVarValue(tensor_data);
    infos->sparsity = mindposre::lite::GetSparsity(tensor_data);
  }

 private:
  // the key is {node_name, tensor_index}
  std::map<PrimaryKey, QuantDebugInfo> origin_info_;
  // Use vector to preserve ordert, There may be more nodes, such as QuantCast, bias etc.
  std::vector<QuantDebugInfo> compared_info_;
};
}  // namespace mindspore::lite

#endif  // MINDSPORE_LITE_TOOLS_CONVERTER_DEBUGINFO_H