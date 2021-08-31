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

#include "tools/converter/acl/deparser/mul_fusion_deparser.h"
#include <memory>
#include "tools/converter/acl/deparser/primitive_deparser_register.h"

namespace mindspore {
namespace lite {
STATUS MulFusionDeparser::Deparser(const CNodePtr &cnode) {
  auto dst_prim = std::make_shared<ops::Mul>();
  if (MoveAttrMap(cnode, dst_prim) != RET_OK) {
    MS_LOG(ERROR) << "MulFusion deparser failed.";
    return RET_ERROR;
  }
  return RET_OK;
}

REGISTER_PRIMITIVE_DEPARSER(kNameMulFusion, MulFusionDeparser)
}  // namespace lite
}  // namespace mindspore