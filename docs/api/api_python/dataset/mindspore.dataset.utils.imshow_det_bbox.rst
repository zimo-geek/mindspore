﻿mindspore.dataset.utils.imshow_det_bbox
========================================

.. py:function:: mindspore.dataset.utils.imshow_det_bbox(image, bboxes, labels, segm=None, class_names=None, score_threshold=0, bbox_color=(0, 255, 0), text_color=(203, 192, 255), mask_color=(128, 0, 128), thickness=2, font_size=0.8, show=True, win_name='win', wait_time=2000, out_file=None)

    使用给定的边界框和类别置信度绘制图像。

    **参数：**

    - **image** (ndarray): 待绘制的图像，shape为(C, H, W)或(H, W, C)，通道顺序为RGB。
    - **bboxes** (ndarray): 边界框（包含类别置信度），shape为(N, 4)或(N, 5)，格式为（N,X,Y,W,H）。
    - **labels** (ndarray): 边界框的类别，shape为(N, 1)。
    - **segm** (ndarray): 图像分割掩码，shape为(M, H, W)，M表示类别总数（默认值None，不绘制掩码）。
    - **class_names** (list[str], dict): 类别索引到类别名的映射表（默认值None，仅显示类别索引）。
    - **score_threshold** (float): 绘制边界框的类别置信度阈值（默认值0，绘制所有边界框）。
    - **bbox_color** (tuple(int)): 指定绘制边界框时线条的颜色，顺序为BGR（默认值(0,255,0），表示'green'）。
    - **text_color** (tuple(int)):指定类别文本的显示颜色，顺序为BGR（默认值(203, 192, 255)，表示'pink'）。
    - **mask_color** (tuple(int)):指定掩码的显示颜色，顺序为BGR（默认值(128, 0, 128)，表示'purple'）。
    - **thickness** (int): 指定边界框和类别文本的线条粗细（默认值2）。
    - **font_size** (int, float): 指定类别文本字体大小（默认值0.8）。
    - **show** (bool): 是否显示图像（默认值为True）。
    - **win_name** (str): 指定窗口名称（默认值"win"）。
    - **wait_time** (int): 指定cv2.waitKey的时延，单位为ms，即图像显示的自动切换间隔（默认值2000，表示间隔为2000ms）。
    - **out_file** (str, optional): 输出图像的文件名，用于在绘制后将结果存储到本地（默认值None，不保存）。

    **返回：**

    ndarray，带边界框和类别置信度的图像。
