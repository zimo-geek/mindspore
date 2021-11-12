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

#ifndef MINDSPORE_CORE_UTILS_ANF_UTILS_H_
#define MINDSPORE_CORE_UTILS_ANF_UTILS_H_
#include <vector>
#include "ir/anf.h"
#include "ir/dtype.h"
#include "base/base.h"
#include "ir/primitive.h"

namespace mindspore {
class AnfUtils {
 public:
  static bool IsDimUnknown(const abstract::ShapePtr &shape);
  static bool IsShapeDynamic(const abstract::ShapePtr &shape);
  static bool IsShapeDynamic(const std::vector<size_t> &shape);
  static bool IsNodeOutputDynamicShape(const CNodePtr &node);
  static bool IsDimUnknown(const AnfNodePtr &node);
  // check whether the anf node is a real kernel that can run on device,parameter and constant is real kernel too
  static bool IsRealKernel(const AnfNodePtr &node);
  // check whether the anf node is a real kernel that is a cnode and can run on device
  static bool IsRealCNodeKernel(const AnfNodePtr &node);
};
}  // namespace mindspore
#endif  // MINDSPORE_CORE_UTILS_ANF_UTILS_H_