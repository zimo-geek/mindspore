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

#include "backend/optimizer/pass/add_akg_kernel_attrs.h"
#include <memory>
#include <vector>
#include <string>
#include "base/core_ops.h"
#include "backend/session/anf_runtime_algorithm.h"

namespace mindspore {
namespace opt {
namespace {
void ClonePrimitive(const AnfNodePtr &node) {
  // Several CNode may share a primitive pointer, so we clone the primitive before setting attr.
  auto cnode = node->cast<CNodePtr>();
  if (cnode == nullptr) return;
  auto prim_node = NewValueNode(AnfAlgo::GetCNodePrimitive(cnode)->Clone());
  cnode->set_input(kAnfPrimitiveIndex, prim_node);
}
}  // namespace

void ProcessCast(const AnfNodePtr &node) {
  // The x and output are akg op input and output param.
  std::vector<std::string> input_names = {"x", kAttrDstType};
  std::vector<std::string> output_names = {"output"};
  ClonePrimitive(node);
  AnfAlgo::SetNodeAttr(kAttrInputNames, MakeValue(input_names), node);
  AnfAlgo::SetNodeAttr(kAttrOutputNames, MakeValue(output_names), node);
  TypeId output_type = AnfAlgo::GetOutputDeviceDataType(node, 0);
  AnfAlgo::SetNodeAttr(kAttrDstType, TypeIdToType(output_type), node);
}

void ProcessMatMul(const AnfNodePtr &node) {
  ClonePrimitive(node);
  TypeId output_type = AnfAlgo::GetOutputDeviceDataType(node, 0);
  AnfAlgo::SetNodeAttr(kAttrDstType, TypeIdToType(output_type), node);
  auto left_format = AnfAlgo::GetInputFormat(node, 0);
  auto right_format = AnfAlgo::GetInputFormat(node, 1);
  AnfAlgo::SetNodeAttr("left_format", MakeValue(left_format), node);
  AnfAlgo::SetNodeAttr("right_format", MakeValue(right_format), node);
}

const AnfNodePtr AddAkgKernelAttrs::Process(const FuncGraphPtr &func_graph, const AnfNodePtr &node,
                                            const EquivPtr &) const {
  MS_EXCEPTION_IF_NULL(func_graph);
  MS_EXCEPTION_IF_NULL(node);
  if (IsPrimitiveCNode(node, prim::kPrimCast)) {
    ProcessCast(node);
  } else if (IsPrimitiveCNode(node, prim::kPrimMatMul) || IsPrimitiveCNode(node, prim::kPrimBatchMatMul)) {
    ProcessMatMul(node);
  }
  return nullptr;
}

const BaseRef AddAkgKernelAttrs::DefinePattern() const {
  VarPtr X = std::make_shared<Var>();
  VarPtr Xs = std::make_shared<SeqVar>();
  return VectorRef({X, Xs});
}
}  // namespace opt
}  // namespace mindspore