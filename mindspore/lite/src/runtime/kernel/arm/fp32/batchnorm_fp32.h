/**
 * Copyright 2020 Huawei Technologies Co., Ltd
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

#ifndef MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_BATCHNORM_FP32_H_
#define MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_BATCHNORM_FP32_H_

#include <vector>
#include "src/inner_kernel.h"
#include "include/context.h"
#include "nnacl/fp32/batchnorm_fp32.h"
#include "nnacl/batchnorm_parameter.h"

using mindspore::lite::InnerContext;

namespace mindspore::kernel {
class BatchnormCPUKernel : public InnerKernel {
 public:
  BatchnormCPUKernel(OpParameter *parameter, const std::vector<lite::Tensor *> &inputs,
                     const std::vector<lite::Tensor *> &outputs, const InnerContext *ctx)
      : InnerKernel(parameter, inputs, outputs, ctx) {}
  virtual ~BatchnormCPUKernel() { FreeMeanAndVariance(); }

  int Prepare() override;
  int ReSize() override;
  int Run() override;
  int SetupVirtualBatch(int virtual_batch_multiplier, int param) override;
  virtual int InitConstTensor();
  virtual int DoExecute(int task_id);
  virtual int set_momentum(float momentum);
  virtual float get_momentum();
  virtual int RestoreDefaultMomentum();

 protected:
  void FillParam();
  void FreeMeanAndVariance();
  void *mean_ = nullptr;
  void *variance_ = nullptr;
  float default_momentum_ = -1.0f;
};

int BatchNormRun(void *cdata, int task_id, float lhs_scale, float rhs_scale);
}  // namespace mindspore::kernel

#endif  // MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_BATCHNORM_FP32_H_
