from PIL import Image
import serial
import time

PORT = "COM8"
BAUD = 115200

# Правильный путь к файлу на Windows (без лишнего слеша в начале)
IMAGE_PATH = "D:/загрузки с браузера/ant-mcu-master (6)/ant-mcu-master/5 Как абстрагироваться от железа/Задания/pics/get.jpg"

ser = None  # инициализируем переменную для порта

try:
    # Открываем изображение
    image = Image.open(IMAGE_PATH)

    # Преобразуем в RGB (на случай, если изображение в другом формате)
    image = image.convert("RGB")

    width, height = image.size
    print("Image size:", width, height)

    # Открываем последовательный порт
    ser = serial.Serial(PORT, BAUD, timeout=1)
    time.sleep(2)  # ждем инициализации устройства

    # Перебираем все пиксели и отправляем команды
    for y in range(height):
        for x in range(width):
            r, g, b = image.getpixel((x, y))
            color = (r << 16) | (g << 8) | b
            cmd = f"disp_px {x} {y} {color:06X}\n"
            ser.write(cmd.encode())

except FileNotFoundError:
    print(f"Ошибка: файл не найден по пути {IMAGE_PATH}")
except Exception as e:
    print(f"Произошла ошибка: {e}")
finally:
    time.sleep(0.1)
    if ser and ser.is_open:
        ser.close()
        print("Порт закрыт")