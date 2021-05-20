# Copyright 2020 Huawei Technologies Co., Ltd
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
"""YoloV4 train."""
import os
import time
import argparse
import datetime
import ast

from mindspore.context import ParallelMode
from mindspore.nn.optim.momentum import Momentum
from mindspore import Tensor
import mindspore.nn as nn
from mindspore import context
from mindspore.communication.management import init, get_rank, get_group_size
from mindspore.train.callback import ModelCheckpoint, RunContext
from mindspore.train.callback import _InternalCallbackParam, CheckpointConfig
import mindspore as ms
from mindspore import amp
from mindspore.train.loss_scale_manager import FixedLossScaleManager
from mindspore.common import set_seed
from mindspore.profiler.profiling import Profiler

from src.yolo import YOLOV4CspDarkNet53, YoloWithLossCell, TrainingWrapper
from src.logger import get_logger
from src.util import AverageMeter, get_param_groups
from src.lr_scheduler import get_lr
from src.yolo_dataset import create_yolo_dataset
from src.initializer import default_recurisive_init, load_yolov4_params
from src.config import ConfigYOLOV4CspDarkNet53
from src.util import keep_loss_fp32
from src.eval_utils import apply_eval, EvalCallBack

set_seed(1)

parser = argparse.ArgumentParser('mindspore coco training')

# device related
parser.add_argument('--device_target', type=str, default='Ascend',
                    help='device where the code will be implemented. (Default: Ascend)')

# dataset related
parser.add_argument('--data_dir', type=str, help='Train dataset directory.')
parser.add_argument('--per_batch_size', default=8, type=int, help='Batch size for Training. Default: 8.')

# network related
parser.add_argument('--pretrained_backbone', default='', type=str,
                    help='The ckpt file of CspDarkNet53. Default: "".')
parser.add_argument('--resume_yolov4', default='', type=str,
                    help='The ckpt file of YOLOv4, which used to fine tune. Default: ""')
parser.add_argument('--pretrained_checkpoint', default='', type=str,
                    help='The ckpt file of YoloV4CspDarkNet53. Default: "".')
parser.add_argument("--filter_weight", type=ast.literal_eval, default=False,
                    help="Filter the last weight parameters, default is False.")

# optimizer and lr related
parser.add_argument('--lr_scheduler', default='cosine_annealing', type=str,
                    help='Learning rate scheduler, options: exponential, cosine_annealing. Default: exponential')
parser.add_argument('--lr', default=0.012, type=float, help='Learning rate. Default: 0.001')
parser.add_argument('--lr_epochs', type=str, default='220,250',
                    help='Epoch of changing of lr changing, split with ",". Default: 220,250')
parser.add_argument('--lr_gamma', type=float, default=0.1,
                    help='Decrease lr by a factor of exponential lr_scheduler. Default: 0.1')
parser.add_argument('--eta_min', type=float, default=0., help='Eta_min in cosine_annealing scheduler. Default: 0')
parser.add_argument('--t_max', type=int, default=320, help='T-max in cosine_annealing scheduler. Default: 320')
parser.add_argument('--max_epoch', type=int, default=320, help='Max epoch num to train the model. Default: 320')
parser.add_argument('--warmup_epochs', default=20, type=float, help='Warmup epochs. Default: 0')
parser.add_argument('--weight_decay', type=float, default=0.0005, help='Weight decay factor. Default: 0.0005')
parser.add_argument('--momentum', type=float, default=0.9, help='Momentum. Default: 0.9')

# loss related
parser.add_argument('--loss_scale', type=int, default=64, help='Static loss scale. Default: 1024')
parser.add_argument('--label_smooth', type=int, default=0, help='Whether to use label smooth in CE. Default:0')
parser.add_argument('--label_smooth_factor', type=float, default=0.1,
                    help='Smooth strength of original one-hot. Default: 0.1')

# logging related
parser.add_argument('--log_interval', type=int, default=100, help='Logging interval steps. Default: 100')
parser.add_argument('--ckpt_path', type=str, default='outputs/', help='Checkpoint save location. Default: outputs/')
parser.add_argument('--ckpt_interval', type=int, default=None, help='Save checkpoint interval. Default: None')

parser.add_argument('--is_save_on_master', type=int, default=1,
                    help='Save ckpt on master or all rank, 1 for master, 0 for all ranks. Default: 1')

# distributed related
parser.add_argument('--is_distributed', type=int, default=1,
                    help='Distribute train or not, 1 for yes, 0 for no. Default: 1')
parser.add_argument('--rank', type=int, default=0, help='Local rank of distributed. Default: 0')
parser.add_argument('--group_size', type=int, default=1, help='World size of device. Default: 1')

# profiler init
parser.add_argument('--need_profiler', type=int, default=0,
                    help='Whether use profiler. 0 for no, 1 for yes. Default: 0')

# reset default config
parser.add_argument('--training_shape', type=str, default="", help='Fix training shape. Default: ""')
parser.add_argument('--resize_rate', type=int, default=10,
                    help='Resize rate for multi-scale training. Default: None')

parser.add_argument("--run_eval", type=ast.literal_eval, default=False,
                    help="Run evaluation when training, default is False.")
parser.add_argument("--save_best_ckpt", type=ast.literal_eval, default=True,
                    help="Save best checkpoint when run_eval is True, default is True.")
parser.add_argument("--eval_start_epoch", type=int, default=200,
                    help="Evaluation start epoch when run_eval is True, default is 200.")
parser.add_argument("--eval_interval", type=int, default=1,
                    help="Evaluation interval when run_eval is True, default is 1.")
parser.add_argument('--ann_file', type=str, default='', help='path to annotation')

args, _ = parser.parse_known_args()

if args.lr_scheduler == 'cosine_annealing' and args.max_epoch > args.t_max:
    args.t_max = args.max_epoch

args.lr_epochs = list(map(int, args.lr_epochs.split(',')))
args.data_root = os.path.join(args.data_dir, 'train2017')
args.annFile = os.path.join(args.data_dir, 'annotations/instances_train2017.json')

args.data_val_root = os.path.join(args.data_dir, 'val2017')
args.ann_val_file = os.path.join(args.data_dir, 'annotations/instances_val2017.json')

config = ConfigYOLOV4CspDarkNet53()
args.nms_thresh = config.nms_thresh
args.ignore_threshold = config.eval_ignore_threshold

device_id = int(os.getenv('DEVICE_ID', '0'))
context.set_context(mode=context.GRAPH_MODE, enable_auto_mixed_precision=True,
                    device_target=args.device_target, save_graphs=False, device_id=device_id)

# init distributed
if args.is_distributed:
    if args.device_target == "Ascend":
        init()
    else:
        init("nccl")
    args.rank = get_rank()
    args.group_size = get_group_size()

# select for master rank save ckpt or all rank save, compatible for model parallel
args.rank_save_ckpt_flag = 0
if args.is_save_on_master:
    if args.rank == 0:
        args.rank_save_ckpt_flag = 1
else:
    args.rank_save_ckpt_flag = 1

# logger
args.outputs_dir = os.path.join(args.ckpt_path,
                                datetime.datetime.now().strftime('%Y-%m-%d_time_%H_%M_%S'))
args.logger = get_logger(args.outputs_dir, args.rank)
args.logger.save_args(args)


def convert_training_shape(args_training_shape):
    training_shape = [int(args_training_shape), int(args_training_shape)]
    return training_shape


class BuildTrainNetwork(nn.Cell):
    def __init__(self, network_, criterion):
        super(BuildTrainNetwork, self).__init__()
        self.network = network_
        self.criterion = criterion

    def construct(self, input_data, label):
        output = self.network(input_data)
        loss_ = self.criterion(output, label)
        return loss_


if __name__ == "__main__":
    if args.need_profiler:
        profiler = Profiler(output_path=args.outputs_dir, is_detail=True, is_show_op_path=True)

    loss_meter = AverageMeter('loss')

    context.reset_auto_parallel_context()
    parallel_mode = ParallelMode.STAND_ALONE
    degree = 1
    if args.is_distributed:
        parallel_mode = ParallelMode.DATA_PARALLEL
        degree = get_group_size()
    context.set_auto_parallel_context(parallel_mode=parallel_mode, gradients_mean=True, device_num=degree)

    network = YOLOV4CspDarkNet53()
    if args.run_eval:
        network_eval = network
    # default is kaiming-normal
    args.checkpoint_filter_list = config.checkpoint_filter_list
    default_recurisive_init(network)
    load_yolov4_params(args, network)

    network = YoloWithLossCell(network)
    args.logger.info('finish get network')

    config.label_smooth = args.label_smooth
    config.label_smooth_factor = args.label_smooth_factor

    if args.training_shape:
        config.multi_scale = [convert_training_shape(args.training_shape)]
    if args.resize_rate:
        config.resize_rate = args.resize_rate

    ds, data_size = create_yolo_dataset(image_dir=args.data_root, anno_path=args.annFile, is_training=True,
                                        batch_size=args.per_batch_size, max_epoch=args.max_epoch,
                                        device_num=args.group_size, rank=args.rank, config=config)
    args.logger.info('Finish loading dataset')

    args.steps_per_epoch = int(data_size / args.per_batch_size / args.group_size)

    if not args.ckpt_interval:
        args.ckpt_interval = args.steps_per_epoch

    lr = get_lr(args)

    opt = Momentum(params=get_param_groups(network),
                   learning_rate=Tensor(lr),
                   momentum=args.momentum,
                   weight_decay=args.weight_decay,
                   loss_scale=args.loss_scale)
    is_gpu = context.get_context("device_target") == "GPU"
    if is_gpu:
        loss_scale_value = 1.0
        loss_scale = FixedLossScaleManager(loss_scale_value, drop_overflow_update=False)
        network = amp.build_train_network(network, optimizer=opt, loss_scale_manager=loss_scale,
                                          level="O2", keep_batchnorm_fp32=False)
        keep_loss_fp32(network)
    else:
        network = TrainingWrapper(network, opt)
        network.set_train()

    if config.rank_save_ckpt_flag or config.run_eval:
        cb_params = _InternalCallbackParam()
        cb_params.train_network = network
        cb_params.epoch_num = config.max_epoch * config.steps_per_epoch // config.ckpt_interval
        cb_params.cur_epoch_num = 1
        run_context = RunContext(cb_params)

    if config.rank_save_ckpt_flag:
        # checkpoint save
        ckpt_max_num = 10
        ckpt_config = CheckpointConfig(save_checkpoint_steps=config.ckpt_interval,
                                       keep_checkpoint_max=ckpt_max_num)
        save_ckpt_path = os.path.join(config.outputs_dir, 'ckpt_' + str(config.rank) + '/')
        ckpt_cb = ModelCheckpoint(config=ckpt_config,
                                  directory=save_ckpt_path,
                                  prefix='{}'.format(config.rank))
        ckpt_cb.begin(run_context)

    if args.run_eval:
        rank_id = int(os.environ.get('RANK_ID')) if os.environ.get('RANK_ID') else 0
        data_val_root = args.data_val_root
        ann_val_file = args.ann_val_file
        save_ckpt_path = os.path.join(args.outputs_dir, 'ckpt_' + str(args.rank) + '/')
        input_val_shape = Tensor(tuple(config.test_img_shape), ms.float32)
        # init detection engine
        eval_dataset, eval_data_size = create_yolo_dataset(data_val_root, ann_val_file, is_training=False,
                                                           batch_size=args.per_batch_size, max_epoch=1, device_num=1,
                                                           rank=0, shuffle=False, config=config)
        eval_param_dict = {"net": network_eval, "dataset": eval_dataset, "data_size": eval_data_size,
                           "anno_json": ann_val_file, "input_shape": input_val_shape, "args": args}
        eval_cb = EvalCallBack(apply_eval, eval_param_dict, interval=args.eval_interval,
                               eval_start_epoch=args.eval_start_epoch, save_best_ckpt=True,
                               ckpt_directory=save_ckpt_path, besk_ckpt_name="best_map.ckpt",
                               metrics_name="mAP")

    old_progress = -1
    t_end = time.time()
    data_loader = ds.create_dict_iterator(output_numpy=True, num_epochs=1)
    network.set_train(True)

    for i, data in enumerate(data_loader):
        images = data["image"]
        input_shape = images.shape[2:4]
        args.logger.info('iter[{}], shape{}'.format(i, input_shape[0]))

        images = Tensor.from_numpy(images)

        batch_y_true_0 = Tensor.from_numpy(data['bbox1'])
        batch_y_true_1 = Tensor.from_numpy(data['bbox2'])
        batch_y_true_2 = Tensor.from_numpy(data['bbox3'])
        batch_gt_box0 = Tensor.from_numpy(data['gt_box1'])
        batch_gt_box1 = Tensor.from_numpy(data['gt_box2'])
        batch_gt_box2 = Tensor.from_numpy(data['gt_box3'])

        input_shape = Tensor(tuple(input_shape[::-1]), ms.float32)
        loss = network(images, batch_y_true_0, batch_y_true_1, batch_y_true_2, batch_gt_box0, batch_gt_box1,
                       batch_gt_box2, input_shape)
        loss_meter.update(loss.asnumpy())

        # ckpt progress
        if args.rank_save_ckpt_flag:
            cb_params.cur_step_num = i + 1  # current step number
            cb_params.batch_num = i + 2
            ckpt_cb.step_end(run_context)

        if i % args.log_interval == 0:
            time_used = time.time() - t_end
            epoch = int(i / args.steps_per_epoch)
            fps = args.per_batch_size * (i - old_progress) * args.group_size / time_used
            if config.rank == 0:
                args.logger.info('epoch[{}], iter[{}], {}, {:.2f} imgs/sec, '
                                 'lr:{}'.format(epoch, i, loss_meter, fps, lr[i]))
            t_end = time.time()
            loss_meter.reset()
            old_progress = i

        if (i + 1) % config.steps_per_epoch == 0 and (config.run_eval or config.rank_save_ckpt_flag):
            if config.run_eval:
                eval_cb.epoch_end(run_context)
                network.set_train()
            cb_params.cur_epoch_num += 1

        if args.need_profiler:
            if i == 10:
                profiler.analyse()
                break

    args.logger.info('==========end training===============')
