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

#ifndef MINDSPORE_CCSRC_MINDDATA_DATASET_AUDIO_IR_VALIDATORS_H_
#define MINDSPORE_CCSRC_MINDDATA_DATASET_AUDIO_IR_VALIDATORS_H_

#include <string>
#include <vector>

#include "minddata/dataset/core/tensor.h"
#include "minddata/dataset/kernels/ir/tensor_operation.h"
#include "minddata/dataset/kernels/ir/validators.h"
#include "minddata/dataset/util/status.h"

namespace mindspore {
namespace dataset {

// Helper function to positive int scalar
Status ValidateIntScalarNonNegative(const std::string &op_name, const std::string &scalar_name, int32_t scalar);

// Helper function to non-nan float scalar
Status ValidateFloatScalarNotNan(const std::string &op_name, const std::string &scalar_name, float scalar);

template <typename T>
// Helper function to check scalar is not equal to zero
Status ValidateScalarNotZero(const std::string &op_name, const std::string &scalar_name, const T scalar) {
  if (scalar == 0) {
    std::string err_msg = op_name + ": " + scalar_name + " can't be zero, got: " + std::to_string(scalar);
    MS_LOG(ERROR) << err_msg;
    return Status(StatusCode::kMDSyntaxError, __LINE__, __FILE__, err_msg);
  }
  return Status::OK();
}

}  // namespace dataset
}  // namespace mindspore
#endif  // MINDSPORE_CCSRC_MINDDATA_DATASET_ADUIO_IR_VALIDATORS_H_