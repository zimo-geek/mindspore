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
#include "minddata/dataset/audio/kernels/mu_law_encoding_op.h"

#include "minddata/dataset/audio/kernels/audio_utils.h"

namespace mindspore {
namespace dataset {

// constructor
MuLawEncodingOp::MuLawEncodingOp(int32_t quantization_channels) : quantization_channels_(quantization_channels) {}

// main function
Status MuLawEncodingOp::Compute(const std::shared_ptr<Tensor> &input, std::shared_ptr<Tensor> *output) {
  IO_CHECK(input, output);
  RETURN_IF_NOT_OK(ValidateLowRank("MuLawEncoding", input, kMinAudioDim, "<..., time>"));
  RETURN_IF_NOT_OK(ValidateTensorNumeric("MuLawEncoding", input));
  return MuLawEncoding(input, output, quantization_channels_);
}

Status MuLawEncodingOp::OutputType(const std::vector<DataType> &inputs, std::vector<DataType> &outputs) {
  RETURN_IF_NOT_OK(TensorOp::OutputType(inputs, outputs));
  RETURN_IF_NOT_OK(
    ValidateTensorType("MuLawEncoding", inputs[0].IsNumeric(), "[int, float, double]", inputs[0].ToString()));
  outputs[0] = DataType(DataType::DE_INT32);
  return Status::OK();
}
}  // namespace dataset
}  // namespace mindspore
