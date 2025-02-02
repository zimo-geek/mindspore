mindspore.ops.Squeeze
=====================

.. py:class:: mindspore.ops.Squeeze(axis=())

    返回删除指定 `axis` 中大小为1的维度的Tensor。

    如果指定了 `axis`，则删除指定 `axis` 中大小为1的维度。
    如果 `axis` 为None，则删除所有大小为1的维度。
    例如，如果输入的shape为(A×1×B×C×1×D)，则输出的Tensor的shape为(A×B×C×D)；如果指定维度，squeeze操作仅在指定维度中进行。
    如果输入的shape为(A×1×B)，squeeze(input_x, 0)不会改变输入的Tensor，但squeeze(input_x, 1)会使输入Tensor的shape变为(A×B)。
    请注意，在动态图模式下，输出Tensor将与输入Tensor共享数据，并且没有Tensor数据复制过程。

    .. note::
        维度索引从0开始，并且必须在 `[-input_x.ndim, input_x.ndim)` 范围内。

    **参数：**

    **axis** (Union[int, tuple(int)]) - 指定待删除shape的维度索引，它会删除该维度中所有大小为1的维度。如果指定了维度索引，其数据类型必须为int32或int64。默认值：()，空tuple。

    **输入：**

    **input_x** (Tensor) - 用于计算Squeeze的输入Tensor，shape为 :math:`(x_1, x_2, ..., x_R)` 。

    **输出：**

    Tensor，shape为 :math:`(x_1, x_2, ..., x_S)` 。

    **异常：**

    - **TypeError** - `axis` 既不是int也不是tuple。
    - **TypeError** - `axis` 是tuple，其元素并非全部是int。
    - **ValueError** - 指定 `axis` 的对应维度不等于1。

    **支持平台：**

    ``Ascend`` ``GPU`` ``CPU``

    **样例：**

    >>> input_x = Tensor(np.ones(shape=[3, 2, 1]), mindspore.float32)
    >>> squeeze = ops.Squeeze(2)
    >>> output = squeeze(input_x)
    >>> print(output)
    [[1. 1.]
     [1. 1.]
     [1. 1.]]
    