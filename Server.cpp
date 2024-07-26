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

void ServiceClient(int socketFD)
{
    // Получим длину имени пользователя, чтобы знать какой длины мы получим сообщение.
    int userNameLength;
    recv(socketFD, &userNameLength, sizeof(int), 0);
    // Размер сообщения равен 81 символу и имени пользователя.
    int messageSize = 90 + ntohl(userNameLength);
    // Подготавливаем буфер к чтению.
    char buf[messageSize];
    recv(socketFD, buf, messageSize, 0);

    // Блокируем файл для безопасной работы с файлом.
    OutFileMutex.lock();

    // Открываем файл и записываем
    std::ofstream OutFile("log.txt", std::ios::app);
    if (OutFile.is_open())
        OutFile << buf << std::endl;
    else
        perror("File lox.txt not open.\n");
    OutFile.close();

    OutFileMutex.unlock(); // Освобождаем файл

    close(socketFD); // Работа закончена, закрываем соединение.
}

int main(int argc, char *argv[])
{
    // Проверка аргументов командной строки - номер порта для прослушивания.
    if (argc < 2)
    {
        printf("The arguments do not specify a listening port.\n");
        exit(1);
    }
    unsigned short int port = atoi(argv[1]);
    if (port == 0)
    {
        printf("First argument not a port or equal to zero: %d.\n", port);
        exit(2);
    }

    int sock, listener;
    struct sockaddr_in addr;

    // Инициализируем сокет для работы.
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0)
    {
        perror("Socket");
        exit(errno);
    }

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

    while (1)
    {
        // Ожидаем подключения
        sock = accept(listener, NULL, NULL);
        if (sock < 0)
        {
            perror("Accept");
            exit(errno);
        }
        // Отправляем его обработку в отдельный поток
        std::thread thr(ServiceClient, sock);
        thr.detach();
    }

    return 0;
}