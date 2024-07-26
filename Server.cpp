#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <fstream>
#include <mutex>

std::mutex OutFileMutex;

// Функция для обработки одного сообщения от одного пользователя в отдельном потоке.
void ServiceClient(int socketFD)
{
    // Получаем длину имени пользователя для вычисления длины его сообщения.
    int userNameLength;
    recv(socketFD, &userNameLength, sizeof(int), 0);

    // 90 — размер сообщения без имени пользователя.
    int messageSize = 90 + ntohl(userNameLength);


    // Подготавливаем буфер к чтению.
    char buf[messageSize];

    // Получаем сообщение
    recv(socketFD, buf, messageSize, 0);


    // Блокируем файл лога для других потоков.
    OutFileMutex.lock();
    // Открываем файл
    std::ofstream OutFile("log.txt", std::ios::app);
    if (OutFile.is_open()) // Если он успешно открыт, записываем в него сообщение.
        OutFile << buf << std::endl;
    else
        perror("File log.txt not open.\n");
    OutFile.close();
    // Освобождаем файл
    OutFileMutex.unlock();


    // Клиент обслужен, закрываем соединение.
    close(socketFD);
}

int main(int argc, char *argv[])
{
    // Программа должна запускаться с одним аргументом — номером порта для прослушивания.
    if (argc < 2)
    {
        printf("The arguments do not specify a listening port.\n");
        exit(1);
    }
    // Преобразуем порт из строки в число и проверяем что он задан числом и не равен техническому нулевому порту.
    unsigned short int port = atoi(argv[1]);
    if (port == 0)
    {
        printf("First argument not a port or equal to zero: %d.\n", port);
        exit(2);
    }


    // Подготавливаем сокет и прослушивание сокета.
    int listener;

    // Инициализируем сокет для работы.
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0)
    {
        perror("Socket");
        exit(errno);
    }

    // Адрес для прослушивания подключений.
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // Инициализируем порт
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        perror("Bind");
        exit(errno);
    }
    listen(listener, 1);

    // Цикл принятия подключений.
    int sock;
    while (1)
    {
        // Ожидаем подключение
        sock = accept(listener, NULL, NULL);
        if (sock < 0) // Проверяем, успешное ли оно.
        {
            perror("Accept");
            exit(errno);
        }
        // Создаем поток для обработки и отсоединяем от главного процесса.
        std::thread(ServiceClient, sock).detach();
    }

    return 0;
}