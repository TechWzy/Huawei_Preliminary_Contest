import pandas as pd
import matplotlib.pyplot as plt
import os

# 读取Excel文件
file_path = ("标签分布.xlsx")  # 请替换为你的Excel文件路径
df = pd.read_excel(file_path, header=None)

# 只保留从第三列开始的数据
df = df.iloc[:, 2:]

# 去除第一行（假设第一行为表头），从第二行开始
data = df.iloc[1:].reset_index(drop=True)

# 按行数的三倍数拆分数据
num_labels = 16
rows_per_part = num_labels  # 每个部分16行

# 计算每个部分的索引
delete_data = data.iloc[0:rows_per_part].values
store_data = data.iloc[rows_per_part:2*rows_per_part].values
read_data = data.iloc[2*rows_per_part:3*rows_per_part].values

# 获取时间点（假设从1开始，每个时间段1800）
time_intervals = [i * 1800 for i in range(data.shape[1])]

# 计算读取数据的缩放比例，使其最大值与删除、存入数据接近
scale_factor = (delete_data.max() + store_data.max()) / read_data.max()
read_data_scaled = read_data * scale_factor

# 创建保存图片的文件夹
output_folder = "折线图"
os.makedirs(output_folder, exist_ok=True)

# 绘制16个折线图并保存
for i in range(num_labels):
    plt.figure(figsize=(6, 4))
    plt.plot(time_intervals, delete_data[i], label='Delete', color='red')
    plt.plot(time_intervals, store_data[i], label='Store', color='blue')
    plt.plot(time_intervals, read_data_scaled[i], label='Read (scaled)', color='green')
    plt.title(f'Label {i+1}')
    plt.xlabel('Time')
    plt.ylabel('Count')
    plt.legend()
    plt.savefig(os.path.join(output_folder, f'Label_{i+1}.png'))
    plt.close()

print(f"所有折线图已保存至 {output_folder} 文件夹。")
