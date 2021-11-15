# Copyright 2021 Huawei Technologies Co., Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ============================================================================
"""st for scipy.sparse.linalg."""

import pytest
import numpy as onp
from scipy.sparse.linalg import cg as osp_cg

from mindspore import context
from mindspore.common import Tensor
from mindspore.scipy.sparse.linalg import cg as msp_cg

from .utils import create_sym_pos_matrix

onp.random.seed(0)


def _fetch_preconditioner(preconditioner, A):
    """
    Returns one of various preconditioning matrices depending on the identifier
    `preconditioner' and the input matrix A whose inverse it supposedly
    approximates.
    """
    if preconditioner == 'identity':
        M = onp.eye(A.shape[0], dtype=A.dtype)
    elif preconditioner == 'random':
        random_metrix = create_sym_pos_matrix(A.shape, A.dtype)
        M = onp.linalg.inv(random_metrix)
    elif preconditioner == 'exact':
        M = onp.linalg.inv(A)
    else:
        M = None
    return M


@pytest.mark.level0
@pytest.mark.platform_x86_gpu_training
@pytest.mark.platform_x86_cpu
@pytest.mark.env_onecard
@pytest.mark.parametrize('dtype_tol', [(onp.float32, 1e-5), (onp.float64, 1e-12)])
@pytest.mark.parametrize('shape', [(4, 4), (7, 7)])
@pytest.mark.parametrize('preconditioner', [None, 'identity', 'exact', 'random'])
@pytest.mark.parametrize('maxiter', [1, 3])
def test_cg_against_scipy(dtype_tol, shape, preconditioner, maxiter):
    """
    Feature: ALL TO ALL
    Description: test cases for cg
    Expectation: the result match scipy
    """
    dtype, tol = dtype_tol
    A = create_sym_pos_matrix(shape, dtype)
    b = onp.random.random(shape[:1]).astype(dtype)
    M = _fetch_preconditioner(preconditioner, A)
    osp_res = osp_cg(A, b, M=M, maxiter=maxiter, atol=tol, tol=tol)[0]

    A = Tensor(A)
    b = Tensor(b)
    M = Tensor(M) if M is not None else M

    # using PYNATIVE MODE
    context.set_context(mode=context.PYNATIVE_MODE)
    msp_res_dyn = msp_cg(A, b, M=M, maxiter=maxiter, atol=tol, tol=tol)[0]

    # using GRAPH MODE
    context.set_context(mode=context.GRAPH_MODE)
    msp_res_sta = msp_cg(A, b, M=M, maxiter=maxiter, atol=tol, tol=tol)[0]

    kw = {"atol": tol, "rtol": tol}
    onp.testing.assert_allclose(osp_res, msp_res_dyn.asnumpy(), **kw)
    onp.testing.assert_allclose(osp_res, msp_res_sta.asnumpy(), **kw)


@pytest.mark.level0
@pytest.mark.platform_x86_gpu_training
@pytest.mark.platform_x86_cpu
@pytest.mark.env_onecard
@pytest.mark.parametrize('dtype', [onp.float32, onp.float64])
@pytest.mark.parametrize('shape', [(2, 2)])
def test_cg_against_numpy(dtype, shape):
    """
    Feature: ALL TO ALL
    Description: test cases for cg
    Expectation: the result match numpy
    """
    A = create_sym_pos_matrix(shape, dtype)
    b = onp.random.random(shape[:1]).astype(dtype)
    expected = onp.linalg.solve(A, b)

    # using PYNATIVE MODE
    context.set_context(mode=context.PYNATIVE_MODE)
    actual_dyn, _ = msp_cg(Tensor(A), Tensor(b))

    # using GRAPH MODE
    context.set_context(mode=context.GRAPH_MODE)
    actual_sta, _ = msp_cg(Tensor(A), Tensor(b))

    kw = {"atol": 1e-5, "rtol": 1e-5}
    onp.testing.assert_allclose(expected, actual_dyn.asnumpy(), **kw)
    onp.testing.assert_allclose(expected, actual_sta.asnumpy(), **kw)