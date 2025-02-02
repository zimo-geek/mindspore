mindspore.nn.Softmax
====================

.. py:class:: mindspore.nn.Softmax(axis=-1)

    Softmax激活函数。

    计算n维输入Tensor的Softmax函数。

    对输入Tensor在 `axis` 上的元素计算其指数函数值，然后归一化到[0, 1]范围，总和为1。

    Softmax定义为：

    .. math::
        \text{softmax}(x_{i}) =  \frac{\exp(x_i)}{\sum_{j=0}^{n-1}\exp(x_j)},

    其中， :math:`x_{i}` 是输入Tensor在 `axis` 上的第 :math:`i` 个元素。

    **参数：**

    **axis** (Union[int, tuple[int]]) - 指定Softmax运算的axis，-1表示最后一个维度。默认值：-1。

    **输入：**

    **x** (Tensor) - 用于计算Softmax函数的Tensor，数据类型为float16或float32。

    **输出：**

    Tensor，shape和数据类型与 `x` 相同，取值范围[0,1]。

    **异常：**

    - **TypeError** - `axis` 既不是int也不是tuple。
    - **TypeError** - `x` 的数据类型既不是float16也不是float32。
    - **ValueError** - `axis` 是长度小于1的tuple。
    - **ValueError** - `axis` 是一个tuple，其元素不都在 `[-x.ndim, x.ndim)` 范围内。

    **支持平台：**

    ``Ascend`` ``GPU`` ``CPU``

    **样例：**

    >>> x = Tensor(np.array([-1, -2, 0, 2, 1]), mindspore.float16)
    >>> softmax = nn.Softmax()
    >>> output = softmax(x)
    >>> print(output)
    [0.03168 0.01166 0.0861  0.636   0.2341 ]
    