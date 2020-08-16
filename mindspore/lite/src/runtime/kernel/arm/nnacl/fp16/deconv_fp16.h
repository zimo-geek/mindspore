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
#ifndef MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_NNACL_FP16_DECONV_FP16_H_
#define MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_NNACL_FP16_DECONV_FP16_H_

#include <string.h>
#include <arm_neon.h>
#include "nnacl/conv_parameter.h"
#include "nnacl/matmul_parameter.h"
#include "nnacl/fp16/matmul_fp16.h"

#ifdef __cplusplus
extern "C" {
#endif
int DeConvPostFp16(const float16_t *src, float16_t *tmp, const float16_t *bias, float16_t *dst, int output_channel,
                   ConvParameter *conv_param);

void PostConvFuncFp16C8(const float16_t *c8_out_ptr, float16_t *out_ptr, const float16_t *bias_ptr,
                        size_t output_channel, size_t plane_size, size_t stride, bool is_relu, bool is_relu6);

void PostFuncBiasReluC8Fp16(float16_t *dst, const float16_t *src, const float16_t *bias, size_t oc8div, size_t oc8mod,
                            size_t plane_size, size_t stride, size_t relu_type);

#ifdef __cplusplus
}
#endif
#endif  // MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_NNACL_FP16_DECONV_FP16_H_
