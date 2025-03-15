from PIL import Image
import os
from fpdf import FPDF


def images_to_pdf(image_folder, output_pdf):
    images = [Image.open(os.path.join(image_folder, f"Label_{i}.png")) for i in range(1, 17)]

    pdf = FPDF(orientation='L', unit='mm', format='A4')  # 横向PDF
    pdf.set_auto_page_break(auto=True, margin=10)
    page_width, page_height = 297, 210  # A4横向尺寸
    img_width, img_height = page_width - 20, page_height - 20  # 图片填充整个页面

    for i, img in enumerate(images):
        pdf.add_page()

        # 解决RGBA无法保存为JPEG的问题
        if img.mode == "RGBA":
            img = img.convert("RGB")

        img.thumbnail((img_width, img_height))  # 调整大小适应页面
        img_path = f"temp_{i}.jpg"
        img.save(img_path, "JPEG")
        pdf.image(img_path, x=10, y=10, w=img_width)
        os.remove(img_path)

    pdf.output(output_pdf)


# 使用示例
image_folder = "./"  # 你的图片文件夹路径
output_pdf = "output.pdf"
images_to_pdf(image_folder, output_pdf)