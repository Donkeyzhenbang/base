#!/usr/bin/env python3
"""高GPU占用的Ray分布式训练"""

import os
os.environ["RAY_TRAIN_ENABLE_V2_MIGRATION_WARNINGS"] = "0"
os.environ["NCCL_DEBUG"] = "WARN"
os.environ["PYTORCH_CUDA_ALLOC_CONF"] = "max_split_size_mb:128"

import torch
import torch.nn as nn
from torch.utils.data import DataLoader
from torchvision import datasets, transforms
from ray import train
from ray.train import ScalingConfig
from ray.train.torch import TorchTrainer

def super_simple_train():
    """最大化GPU占用的训练函数"""
    # 获取信息
    ctx = train.get_context()
    world_rank = ctx.get_world_rank()
    local_rank = ctx.get_local_rank()

    print(f"Worker {world_rank} 启动，使用 GPU {local_rank}")

    # 设置GPU
    torch.cuda.set_device(local_rank)
    device = torch.device(f"cuda:{local_rank}")
    print(f"Worker {world_rank} 设备: {torch.cuda.get_device_name(local_rank)}")
    
    # 打印初始GPU内存
    if torch.cuda.is_available():
        print(f"Worker {world_rank} 初始GPU内存: {torch.cuda.memory_allocated()/1024**3:.2f}GB")

    # 1. 创建非常大的模型（增加显存占用）
    model = nn.Sequential(
        # 增加层数和维度，显著增加计算量和显存
        nn.Flatten(),
        nn.Linear(28*28, 4096),  # 从128增加到4096
        nn.ReLU(),
        nn.Dropout(0.5),
        nn.Linear(4096, 4096),
        nn.ReLU(),
        nn.Dropout(0.5),
        nn.Linear(4096, 2048),
        nn.ReLU(),
        nn.Dropout(0.5),
        nn.Linear(2048, 1024),
        nn.ReLU(),
        nn.Dropout(0.5),
        nn.Linear(1024, 512),
        nn.ReLU(),
        nn.Linear(512, 10)
    ).to(device)
    
    # 打印模型大小
    total_params = sum(p.numel() for p in model.parameters())
    print(f"Worker {world_rank} 模型参数量: {total_params:,}")
    print(f"Worker {world_rank} 模型加载后GPU内存: {torch.cuda.memory_allocated()/1024**3:.2f}GB")

    # 转换为分布式模型
    model = train.torch.prepare_model(model)

    # 2. 数据准备（增加数据复杂度和batch size）
    transform = transforms.Compose([
        transforms.ToTensor(),
        transforms.Normalize((0.5,), (0.5,)),
        transforms.RandomAffine(degrees=10, translate=(0.1, 0.1)),  # 增加数据增强
    ])

    dataset = datasets.FashionMNIST(
        root="./data",
        train=True,
        download=True,
        transform=transform
    )

    # 大幅增加batch size以提升GPU使用率
    dataloader = DataLoader(dataset, batch_size=1024, shuffle=True, num_workers=2)  # 从32增加到1024
    dataloader = train.torch.prepare_data_loader(dataloader)

    # 3. 训练配置 - 使用混合精度训练提升GPU利用率
    criterion = nn.CrossEntropyLoss()
    optimizer = torch.optim.Adam(model.parameters(), lr=0.001)  # 使用Adam优化器
    scaler = torch.cuda.amp.GradScaler()  # 混合精度训练

    print(f"Worker {world_rank} 开始训练...")
    print(f"Worker {world_rank} 当前GPU内存: {torch.cuda.memory_allocated()/1024**3:.2f}GB")

    # 训练更多epoch
    for epoch in range(200):  # 增加到200个epoch
        total_loss = 0
        correct = 0
        total = 0

        for batch_idx, (inputs, targets) in enumerate(dataloader):
            # 使用混合精度训练（提升GPU使用率）
            with torch.cuda.amp.autocast():
                outputs = model(inputs)
                loss = criterion(outputs, targets)

            # 反向传播
            optimizer.zero_grad()
            scaler.scale(loss).backward()
            scaler.step(optimizer)
            scaler.update()

            # 统计
            total_loss += loss.item()
            _, predicted = outputs.max(1)
            total += targets.size(0)
            correct += predicted.eq(targets).sum().item()

            # 每20个batch打印一次，减少日志量
            if batch_idx % 20 == 0:
                acc = 100. * correct / total if total > 0 else 0
                # 打印GPU内存使用情况
                if torch.cuda.is_available():
                    memory_used = torch.cuda.memory_allocated() / 1024**3
                    memory_reserved = torch.cuda.memory_reserved() / 1024**3
                    print(f"Worker {world_rank}, Epoch {epoch}, Batch {batch_idx}, "
                          f"Loss: {loss.item():.4f}, Acc: {acc:.2f}%, "
                          f"GPU内存: {memory_used:.2f}GB/{memory_reserved:.2f}GB")

        # 报告指标
        avg_loss = total_loss / len(dataloader)
        epoch_acc = 100. * correct / total
        train.report({
            "epoch": epoch,
            "loss": avg_loss,
            "accuracy": epoch_acc,
            "worker": world_rank,
            "gpu_memory_used_gb": torch.cuda.memory_allocated() / 1024**3 if torch.cuda.is_available() else 0
        })

        # 每10个epoch打印一次详细信息
        if epoch % 10 == 0:
            print(f"Worker {world_rank}, Epoch {epoch} 完成: "
                  f"Loss: {avg_loss:.4f}, Acc: {epoch_acc:.2f}%")
            
            # 打印详细的GPU信息
            if torch.cuda.is_available():
                print(f"Worker {world_rank} GPU状态:")
                print(f"  已分配: {torch.cuda.memory_allocated()/1024**3:.2f}GB")
                print(f"  已缓存: {torch.cuda.memory_reserved()/1024**3:.2f}GB")
                print(f"  最大已分配: {torch.cuda.max_memory_allocated()/1024**3:.2f}GB")

    print(f"Worker {world_rank} 训练完成！")
    if torch.cuda.is_available():
        print(f"Worker {world_rank} 最终GPU内存: {torch.cuda.memory_allocated()/1024**3:.2f}GB")

def main():
    print("=" * 60)
    print("高GPU占用的Ray分布式训练")
    print("目标: 最大化GPU使用率和显存占用")
    print("=" * 60)

    # 检查GPU
    print(f"PyTorch版本: {torch.__version__}")
    print(f"CUDA可用: {torch.cuda.is_available()}")
    if torch.cuda.is_available():
        print(f"GPU数量: {torch.cuda.device_count()}")
        for i in range(torch.cuda.device_count()):
            gpu_name = torch.cuda.get_device_name(i)
            total_memory = torch.cuda.get_device_properties(i).total_memory / 1024**3
            print(f"  GPU {i}: {gpu_name}")
            print(f"    显存总量: {total_memory:.2f} GB")

    # 配置
    print("\n配置: 2个worker，每个使用1个GPU")
    print("训练设置: 200个epoch, batch_size=1024, 大模型")

    scaling_config = ScalingConfig(
        num_workers=2,
        use_gpu=True,
        resources_per_worker={"CPU": 4, "GPU": 1}  # 增加CPU核心数
    )

    # 创建训练器
    trainer = TorchTrainer(
        super_simple_train,
        scaling_config=scaling_config
    )

    # 开始训练
    print("\n开始训练...")
    print("-" * 60)
    print("提示: 在新终端中运行以下命令监控GPU:")
    print("  watch -n 1 nvidia-smi")
    print("  watch -n 5 ray status")
    print("-" * 60)

    try:
        result = trainer.fit()
        print("\n" + "=" * 60)
        print("训练成功完成！")
        print("=" * 60)
        
        # 打印最终统计信息
        if torch.cuda.is_available():
            for i in range(torch.cuda.device_count()):
                torch.cuda.synchronize(i)
                print(f"GPU {i} 最大显存使用: {torch.cuda.max_memory_allocated(i)/1024**3:.2f}GB")
        
        if result.metrics:
            print(f"最终指标: {result.metrics}")
            
    except Exception as e:
        print(f"\n训练失败: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
