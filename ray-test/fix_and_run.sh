#!/bin/bash
# fix_and_run.sh - 一步修复所有问题并运行训练

set -e  # 出错时退出

echo "========================================"
echo "Ray分布式训练 - 终极修复版"
echo "========================================"

# 1. 安装缺失的依赖
echo "[1/6] 安装必要的依赖..."
#pip install aiohttp aiosignal frozenlist attrs multidict yarl async-timeout -q
#pip install "ray[default]" --upgrade -q
#pip install torch torchvision --upgrade -q

# 2. 清理环境
echo "[2/6] 清理Ray进程..."
ray stop --force 2>/dev/null || true
pkill -f ray 2>/dev/null || true
sleep 2

# 3. 启动Ray（禁用dashboard，避免依赖问题）
echo "[3/6] 启动Ray集群（无dashboard）..."
ray start --head \
    --num-cpus=8 \
    --num-gpus=2 \
    --port=6379 \
    --include-dashboard=false \
    --disable-usage-stats \
    --object-store-memory=4000000000 \
    --temp-dir=/tmp/ray

# 等待Ray启动
sleep 3

# 4. 检查Ray状态
echo "[4/6] 检查Ray状态..."
if ray status; then
    echo "✓ Ray启动成功！"
else
    echo "✗ Ray启动失败，尝试备用方案..."
    # 备用方案：用最简单的方式启动
    ray start --head --num-gpus=2 --port=6379 --include-dashboard=false
    sleep 2
    ray status || echo "Ray状态检查失败，但继续尝试..."
fi

# 5. 创建绝对能运行的训练脚本
echo "[5/6] 创建绝对简化的训练脚本..."

cat > train_ultimate.py << 'EOF'
#!/usr/bin/env python3
"""绝对能运行的Ray分布式训练"""

import os
os.environ["RAY_TRAIN_ENABLE_V2_MIGRATION_WARNINGS"] = "0"
os.environ["NCCL_DEBUG"] = "WARN"

import torch
import torch.nn as nn
from torch.utils.data import DataLoader
from torchvision import datasets, transforms
from ray import train
from ray.train import ScalingConfig
from ray.train.torch import TorchTrainer

def super_simple_train():
    """最简单的训练函数"""
    # 获取信息
    ctx = train.get_context()
    world_rank = ctx.get_world_rank()
    local_rank = ctx.get_local_rank()
    
    print(f"Worker {world_rank} 启动，使用 GPU {local_rank}")
    
    # 设置GPU
    if torch.cuda.is_available():
        torch.cuda.set_device(local_rank)
        device = torch.device(f"cuda:{local_rank}")
        print(f"Worker {world_rank} 设备: {torch.cuda.get_device_name(local_rank)}")
    else:
        device = torch.device("cpu")
        print(f"Worker {world_rank} 使用CPU")
    
    # 1. 简单模型
    model = nn.Sequential(
        nn.Flatten(),
        nn.Linear(28*28, 128),
        nn.ReLU(),
        nn.Linear(128, 10)
    ).to(device)
    
    # 转换为分布式模型
    model = train.torch.prepare_model(model)
    
    # 2. 简单数据
    transform = transforms.Compose([
        transforms.ToTensor(),
        transforms.Normalize((0.5,), (0.5,))
    ])
    
    dataset = datasets.FashionMNIST(
        root="./data",
        train=True,
        download=True,
        transform=transform
    )
    
    dataloader = DataLoader(dataset, batch_size=32, shuffle=True)
    dataloader = train.torch.prepare_data_loader(dataloader)
    
    # 3. 训练
    criterion = nn.CrossEntropyLoss()
    optimizer = torch.optim.SGD(model.parameters(), lr=0.01)
    
    print(f"Worker {world_rank} 开始训练...")
    
    for epoch in range(2):  # 只训练2个epoch，快速验证
        total_loss = 0
        correct = 0
        total = 0
        
        for batch_idx, (inputs, targets) in enumerate(dataloader):
            # 前向传播
            outputs = model(inputs)
            loss = criterion(outputs, targets)
            
            # 反向传播
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
            
            # 统计
            total_loss += loss.item()
            _, predicted = outputs.max(1)
            total += targets.size(0)
            correct += predicted.eq(targets).sum().item()
            
            if batch_idx % 100 == 0:
                acc = 100. * correct / total if total > 0 else 0
                print(f"Worker {world_rank}, Epoch {epoch}, Batch {batch_idx}, "
                      f"Loss: {loss.item():.4f}, Acc: {acc:.2f}%")
        
        # 报告指标
        avg_loss = total_loss / len(dataloader)
        epoch_acc = 100. * correct / total
        train.report({
            "epoch": epoch,
            "loss": avg_loss,
            "accuracy": epoch_acc,
            "worker": world_rank
        })
        
        print(f"Worker {world_rank}, Epoch {epoch} 完成: "
              f"Loss: {avg_loss:.4f}, Acc: {epoch_acc:.2f}%")
    
    print(f"Worker {world_rank} 训练完成！")

def main():
    print("=" * 50)
    print("绝对简化的Ray分布式训练")
    print("=" * 50)
    
    # 检查GPU
    print(f"PyTorch版本: {torch.__version__}")
    print(f"CUDA可用: {torch.cuda.is_available()}")
    if torch.cuda.is_available():
        print(f"GPU数量: {torch.cuda.device_count()}")
        for i in range(torch.cuda.device_count()):
            print(f"  GPU {i}: {torch.cuda.get_device_name(i)}")
    
    # 配置
    print("\n配置: 2个worker，每个使用1个GPU")
    
    scaling_config = ScalingConfig(
        num_workers=2,
        use_gpu=True,
        resources_per_worker={"CPU": 2, "GPU": 1}
    )
    
    # 创建训练器
    trainer = TorchTrainer(
        super_simple_train,
        scaling_config=scaling_config
    )
    
    # 开始训练
    print("\n开始训练...")
    print("-" * 50)
    
    try:
        result = trainer.fit()
        print("\n" + "=" * 50)
        print("训练成功完成！")
        print("=" * 50)
        print(f"最终指标: {result.metrics}")
    except Exception as e:
        print(f"\n训练失败: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
EOF

# 6. 运行训练
echo "[6/6] 运行训练..."
echo "========================================"
#python3 train_ultimate.py
python3 large_scale_training.py
echo ""
echo "========================================"
echo "完成！"
echo "如果看到'训练成功完成！'，则一切正常"
echo "========================================"
